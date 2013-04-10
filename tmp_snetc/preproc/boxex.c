/*******************************************************************************
 *
 * $Id: boxex.c 2507 2009-08-05 12:02:11Z jju $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   15.02.2007
 * -----
 *
 *******************************************************************************/

#include "boxex.h"
#include "boxexfile.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "str.h"
#include "free.h"

/*
 * INFO structure
 */
struct INFO {
  char *language;
};

#define INFO_LANGUAGE(n) (n->language)

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_LANGUAGE(result) = NULL;

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *PREPBOXEXboxbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPBOXEXboxbody");

  if(INFO_LANGUAGE(arg_info) == NULL) {
    if(BOXBODY_LANGNAME(arg_node) != NULL) {
      INFO_LANGUAGE(arg_info) = STRcpy(BOXBODY_LANGNAME(arg_node));
      if(!openFile(global.filebase, "box", BOXBODY_LANGNAME(arg_node))) {
        CTIwarn(CTI_ERRNO_FILE_ACCESS_ERROR,
		"Cannot open box file box.%s for writing\n", 
                BOXBODY_LANGNAME(arg_node));
      }
    }
  }
  if(INFO_LANGUAGE(arg_info) != NULL) {
    if(STReq(INFO_LANGUAGE(arg_info), BOXBODY_LANGNAME(arg_node))) {
      writeFile(BOXBODY_CODE(arg_node));
      MEMfree(BOXBODY_LANGNAME(arg_node));
      MEMfree(BOXBODY_CODE(arg_node));
      BOXBODY_LANGNAME(arg_node) = NULL;
      BOXBODY_CODE(arg_node) = NULL;
    }
  }

  DBUG_RETURN(arg_node);
}

node *PREPBOXEXmodule(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPBOXEXmodule");

  if(MODULE_DEFS(arg_node) != NULL) {
    do {
      if(INFO_LANGUAGE(arg_info) != NULL) {
        MEMfree(INFO_LANGUAGE(arg_info));
        INFO_LANGUAGE(arg_info) = NULL;
      }
      MODULE_DEFS(arg_node) = TRAVdo(MODULE_DEFS(arg_node), arg_info);
      closeFile();
    }while(INFO_LANGUAGE(arg_info) != NULL);
  }

  DBUG_RETURN(arg_node);
}

node *PREPdoExtract(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("PREPdoExtract");
  
  DBUG_ASSERT((syntax_tree != NULL), "PREPdoExtract called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_prepboxex);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
