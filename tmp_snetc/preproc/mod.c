/*******************************************************************************
 *
 * $Id: mod.c 2507 2009-08-05 12:02:11Z jju $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   20.02.2007
 * -----
 *
 *******************************************************************************/

#include "mod.h"
#include <dlfcn.h>
#include <string.h>
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "str.h"
#include "free.h"
#include "scanparse.h"
#include "lookup_table.h"

static lut_t *fieldLUT = NULL; /* Field names to node mapping */
static lut_t *stagLUT  = NULL; /* Simple tag names to node mapping */
static lut_t *btagLUT  = NULL; /* Binding tag names to node mapping */

static node *getSign(char *netName)
{
  node *tree    = NULL;
  node *sign    = NULL;
  char *oldPath = global.pathname;
  node *defs    = NULL;
  node *net     = NULL;

  global.pathname = STRcat(netName, ".snet");
  tree = SPdoScanParse(NULL);

  MEMfree(global.pathname);
  global.pathname = oldPath;

  if(tree != NULL) {
    defs = MODULE_DEFS(tree);
    if(defs != NULL) {
      net = DEFS_DEF(defs);
      if(net != NULL) {
        sign = NETDEF_SIGN(net);
        NETDEF_SIGN(net) = NULL;
        FREEdoFreeNode(tree);
      }
    }
  }
  return sign;
}

static node *getSignature(char *netName)
{
  void *lib        = NULL;
  char *ptr        = NULL;
  char *libDir     = NULL;
  char *envLibDirs = getenv("LIBRARY_PATH");
  char *libDirs    = STRcatn(3, ". ", envLibDirs, global.clib_dirs);
  FILE *file       = NULL;
  char *libName    = NULL;
  char *fileName   = STRcat(netName, ".snet");
  void (*func)(FILE *) = 0;
  char *err = NULL;

  CTInote("Opening external net signature file %s", fileName);
  if((file = fopen(fileName, "r")) == NULL) {
    CTIerror(CTI_ERRNO_FILE_ACCESS_ERROR,
	     "External net signature file %s open failed\n", fileName);
  }
  else {
    libDir = strtok_r(libDirs, " ", &ptr);
    while(libDir != NULL) {
      libName = STRcatn(4, libDir, "/lib", netName, ".so");
      lib = dlopen(libName, RTLD_LAZY);
      libName = MEMfree(libName);
      if(lib != NULL) {
        func = (void (*)(FILE *))dlsym(lib, "getSignature");
        err = dlerror();
        if(err != NULL) {
          CTIerror(CTI_ERRNO_SIGNATURE_ERROR,
		   "Signature load failed: %s\n", err);
        }
        else {
          func(file);
        }
        break;
      }
      libDir = strtok_r(NULL, " ", &ptr);
    }
  }

  if(file != NULL) {
    fclose(file);
  }
  if(lib != NULL) {
    dlclose(lib);
  }
  MEMfree(libDirs);
  MEMfree(fileName);

  return getSign(netName);
}

node *PREPMODbtags(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPMODbtags");

  LUTinsertIntoLutS(btagLUT, BTAGS_NAME(arg_node), arg_node);

  if(BTAGS_NEXT(arg_node) != NULL) {
    TRAVdo(BTAGS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PREPMODfields(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPMODfields");

  LUTinsertIntoLutS(fieldLUT, FIELDS_NAME(arg_node), arg_node);

  if(FIELDS_NEXT(arg_node) != NULL) {
    TRAVdo(FIELDS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PREPMODmodule(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPMODmodule");

  fieldLUT = LUTgenerateLut();
  stagLUT  = LUTgenerateLut();
  btagLUT  = LUTgenerateLut();

  if(MODULE_FIELDS(arg_node) != NULL) {
    TRAVdo(MODULE_FIELDS(arg_node), arg_info);
  }
  if(MODULE_STAGS(arg_node) != NULL) {
    TRAVdo(MODULE_STAGS(arg_node), arg_info);
  }
  if(MODULE_BTAGS(arg_node) != NULL) {
    TRAVdo(MODULE_BTAGS(arg_node), arg_info);
  }

  if(MODULE_DEFS(arg_node) != NULL) {
    TRAVdo(MODULE_DEFS(arg_node), arg_info);
  }

  LUTremoveLut(fieldLUT);
  LUTremoveLut(stagLUT);
  LUTremoveLut(btagLUT);
  fieldLUT = NULL;
  stagLUT  = NULL;
  btagLUT  = NULL;

  DBUG_RETURN(arg_node);
}

node *PREPMODnetdef(node *arg_node, info *arg_info)
{
  node *sign = NULL;

  DBUG_ENTER("PREPMODnetdef");

  if(NETDEF_EXTERNAL(arg_node)) {
    sign = getSignature(NETDEF_NAME(arg_node));
    if(sign == NULL) {
      CTIerror(CTI_ERRNO_SIGNATURE_ERROR,
	       "Cannot resolve signature\n");
    }
    else {
      NETDEF_SIGN(arg_node) = sign;
      NETDEF_SIGNED(arg_node) = TRUE;
      TRAVdo(sign, arg_info);
    }
  }

  if(NETDEF_BODY(arg_node) != NULL) {
    TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PREPMODrecentries(node *arg_node, info *arg_info)
{
  bool freeNode = FALSE;
  node *top     = arg_node;
  node *prev    = NULL;
  node *next    = NULL;
  node *ref     = NULL;
  void **p      = NULL;

  DBUG_ENTER("PREPMODrecentries");

  while(arg_node != NULL) {
    freeNode = FALSE;

    if(RECENTRIES_FIELD(arg_node) != NULL) {
      ref = RECENTRIES_FIELD(arg_node);
      p = LUTsearchInLutS(fieldLUT, FIELDREF_NAME(ref));
      if(p == NULL) {
        CTIwarn(CTI_ERRNO_DEAD_CODE,
		"Unreferenced external net's field %s (will be removed)\n", FIELDREF_NAME(ref));
        freeNode = TRUE;
      }
      else {
        FIELDREF_FIELD(ref) = *p;
      }
    }
    else if(RECENTRIES_STAG(arg_node) != NULL) {
      ref = RECENTRIES_STAG(arg_node);
      p = LUTsearchInLutS(stagLUT, STAGREF_NAME(ref));
      if(p == NULL) {
        CTIwarn(CTI_ERRNO_DEAD_CODE,
		"Unreferenced external net's simple tag %s (will be removed)\n", STAGREF_NAME(ref));
        freeNode = TRUE;
      }
      else {
        STAGREF_STAG(ref) = *p;
      }
    }
    else if(RECENTRIES_BTAG(arg_node) != NULL) {
      ref = RECENTRIES_BTAG(arg_node);
      p = LUTsearchInLutS(btagLUT, BTAGREF_NAME(ref));
      if(p == NULL) {
        CTIwarn(CTI_ERRNO_DEAD_CODE,
		"Unreferenced external net's binding tag %s (will be removed)\n", BTAGREF_NAME(ref));
        freeNode = TRUE;
      }
      else {
        BTAGREF_BTAG(ref) = *p;
      }
    }
    next = RECENTRIES_NEXT(arg_node);
    if(freeNode) {
      RECENTRIES_NEXT(arg_node) = NULL;
      if(top == arg_node) {
        top  = next;
        prev = NULL;
      }
      else {
        RECENTRIES_NEXT(prev) = next;
      }
      FREEdoFreeNode(arg_node);
    }
    else {
      prev = arg_node;
    }
    arg_node = next;
  }

  DBUG_RETURN(top);
}

node *PREPMODstags(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPMODstags");

  LUTinsertIntoLutS(stagLUT, STAGS_NAME(arg_node), arg_node);

  if(STAGS_NEXT(arg_node) != NULL) {
    TRAVdo(STAGS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PREPdoLoad(node *syntax_tree)
{
  DBUG_ENTER("PREPdoLoad");
  
  DBUG_ASSERT((syntax_tree != NULL), "PREPdoLoad called with empty syntaxtree");

  TRAVpush(TR_prepmod);

  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}
