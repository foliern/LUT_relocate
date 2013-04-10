/*******************************************************************************
 *
 * $Id: siginfprim.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Primitive Sig Inference -- generates NET/BOXDEF_NTYPESIG for
 * boxes, box nets, filter nets, sync nets, external nets, signed nets.
 *
 *******************************************************************************/

#include "siginfprim.h"
#include "getsig.h"
#include "getvrec.h"
#include "getnrec.h"
#include "typing.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "copy.h"
#include "free.h"
#include "str.h"
#include "ctinfo.h"

/* The info struct */
struct INFO {
  node *netdef;
  TYPntypesig *ntypesig;
};

/* macros for accessing the fields */
#define INFO_NETDEF(i)   (i->netdef)
#define INFO_NTYPESIG(i) (i->ntypesig)

/* Makes an info struct. */
static info *infoMake(void)
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_NETDEF(result) = NULL;
  INFO_NTYPESIG(result) = NULL;

  DBUG_RETURN(result);
}

/* Releases an info struct. */
static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

/* traverses BoxDef and creates its ntypesig */
node *TSIPboxdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TSIPboxdef");

  BOXDEF_NTYPESIG(arg_node) = TGSdoGetSig(BOXDEF_SIGN(arg_node));

  if (TSDOPRINT) {
    char *display = TYPprintNtypesig(BOXDEF_NTYPESIG(arg_node));
    TSPRINT("Box %s at %s:%d.%d is inferred this sig:\n%s", BOXDEF_NAME(arg_node),
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node), display);
    MEMfree(display);
  }

  DBUG_RETURN(arg_node);
}

/* stores NetDef into info and traverses NetBody, then generates NetDef.Sign
 * if NetDef.NTypeSig is present */
node *TSIPnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TSIPnetdef");

  INFO_NETDEF(arg_info) = arg_node;

  if (NETDEF_EXTERNAL(arg_node)) {
    NETDEF_NTYPESIG(arg_node) = TGSdoGetSig(NETDEF_SIGN(arg_node));
    if (TYPisEmptyNtypesig(NETDEF_NTYPESIG(arg_node))) {
      CTIerrorNode(CTI_ERRNO_SIGNATURE_ERROR,
		   arg_node,
		   "External net %s has an empty signature", NETDEF_NAME(arg_node));
    }
    else if (TYPisIncompleteNtypesig(NETDEF_NTYPESIG(arg_node))) {
      CTIerrorNode(CTI_ERRNO_SIGNATURE_ERROR,
		   arg_node,
		   "External net %s has an incomplete signature", NETDEF_NAME(arg_node));
    }
    else {
      NETDEF_SIGNED(arg_node) = TRUE;
    }
  }
  else {
    if (NETDEF_BODY(arg_node) != NULL) {
      NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
    }
    if (NETDEF_SIGNED(arg_node)) {
      TYPfreeNtypesig(NETDEF_NTYPESIG(arg_node));
      NETDEF_NTYPESIG(arg_node) = TGSdoGetSig(NETDEF_SIGN(arg_node));
    }
  }

  /* reconstructs Sign node from NTypeSig if present */
  if (NETDEF_NTYPESIG(arg_node) != NULL) {
    if (NETDEF_SIGN(arg_node) != NULL) {
      FREEdoFreeTree(NETDEF_SIGN(arg_node));
    }
    NETDEF_SIGN(arg_node) = TYPcreateTypeSigns(NETDEF_NTYPESIG(arg_node));
  }
  DBUG_RETURN(arg_node);
}

/* traverses netbody (defs + connect) and puts the inferred type signature
 * into the current netdef (in info) */
node *TSIPnetbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TSIPnetbody");

  node *mynetdef = INFO_NETDEF(arg_info);

  if (NETBODY_DEFS(arg_node) != NULL) {
    NETBODY_DEFS(arg_node) = TRAVdo(NETBODY_DEFS(arg_node), arg_info);
  }

  if (NETBODY_CONNECT(arg_node) != NULL) {
    NETBODY_CONNECT(arg_node) = TRAVdo(NETBODY_CONNECT(arg_node), arg_info);
  }

  NETDEF_NTYPESIG(mynetdef) = INFO_NTYPESIG(arg_info);
  INFO_NTYPESIG(arg_info) = NULL;

  DBUG_RETURN(arg_node);
}

/* traverses a boxref and copies the signature of the referenced boxdef */
node *TSIPboxref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TSIPboxref");

  node *box = BOXREF_BOX(arg_node);
  TYPntypesig *sig = BOXDEF_NTYPESIG(box);
  INFO_NTYPESIG(arg_info) = TYPcopyNtypesig(sig);

  DBUG_RETURN(arg_node);
}

/* sync: collects a vrectype and infers signature */
node *TSIPsync(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TSIPsync");

  TYPnrectype *maintype = TGNdoGetNRec(SYNC_MAINPATTERN(arg_node));
  TYPvrectype *auxtypes = TGVdoGetVRec(SYNC_AUXPATTERNS(arg_node));
  INFO_NTYPESIG(arg_info) = TYPinferSync(maintype, auxtypes);

  if (TSDOPRINT) {
    char *display = TYPprintNtypesig(INFO_NTYPESIG(arg_info));
    TSPRINT("Synchrocell at %s:%d.%d is inferred this sig:\n%s",
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node), display);
    MEMfree(display);
  }

  DBUG_RETURN(arg_node);
}

/* filt: delegate to TGS */
node *TSIPfilt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TSIPfilt");

  INFO_NTYPESIG(arg_info) = TGSdoGetSig(arg_node);

  if (TSDOPRINT) {
    char *display = TYPprintNtypesig(INFO_NTYPESIG(arg_info));
    TSPRINT("Filter at %s:%d.%d is inferred this sig:\n%s",
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node), display);
    MEMfree(display);
  }

  DBUG_RETURN(arg_node);
}

/* main for this file */
node *TSIPdoSigInf(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("TSIPdoSigInf");

  DBUG_ASSERT((syntax_tree != NULL), "TSIPdoSigInf called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_tsip);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

