/*******************************************************************************
 *
 * $Id: getvrec.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Sig Getter - generates NETDEF_NTYPESIG object from NETDEF_SIGN node
 *
 *******************************************************************************/

#include "getvrec.h"
#include "getnrec.h"
#include "typing.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "free.h"

/* The info struct */
struct INFO {
  TYPvrectype *vrectype;
  bool collectUTs;
  bool collectCTs;
};

/* macros for accessing the fields */
#define INFO_VRECTYPE(i) (i->vrectype)
#define INFO_COLLECTUTS(i) (i->collectUTs)
#define INFO_COLLECTCTS(i) (i->collectCTs)

/* Makes an info struct. */
static info *infoMake(bool collectUTs, bool collectCTs)
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_VRECTYPE(result) = TYPnewVrectype();
  INFO_COLLECTUTS(result) = collectUTs;
  INFO_COLLECTCTS(result) = collectCTs;

  DBUG_RETURN(result);
}

/* Releases an info struct. */
static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

/* types: collect nrectypes into info.vrectype */
node *TGVtypes(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGVtypes");

  TYPnrectype *n = TGNdoGetNRec(TYPES_TYPE(arg_node));
  TYPfeedVrectype(INFO_VRECTYPE(arg_info), n);

  if (TYPES_NEXT(arg_node) != NULL) {
    TYPES_NEXT(arg_node) = TRAVdo(TYPES_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

/* traverses BoxTypes and populates the vrectype in the info */
node *TGVboxtypes(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGVboxtypes");

  TYPnrectype *n = TGNdoGetNRec(BOXTYPES_ENTRIES(arg_node));
  TYPfeedVrectype(INFO_VRECTYPE(arg_info), n);

  if (BOXTYPES_NEXT(arg_node) != NULL) {
    BOXTYPES_NEXT(arg_node) = TRAVdo(BOXTYPES_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

/* guardpatterns for sync: collect nrectypes into info.vrectype */
node *TGVguardpatterns(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGVguardpatterns");

  if ((GUARDPATTERNS_CONDITION(arg_node) == NULL && INFO_COLLECTUTS(arg_info))||
      (GUARDPATTERNS_CONDITION(arg_node) != NULL && INFO_COLLECTCTS(arg_info))){
    TYPnrectype *n = TGNdoGetNRec(GUARDPATTERNS_ENTRIES(arg_node));
    TYPfeedVrectype(INFO_VRECTYPE(arg_info), n);
  }

  if (GUARDPATTERNS_NEXT(arg_node) != NULL) {
    GUARDPATTERNS_NEXT(arg_node) =
      TRAVdo(GUARDPATTERNS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

/* recouts: collects nrectypes for the filter vrectype */
node *TGVrecouts(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGVrecouts");

  TYPnrectype *n = TGNdoGetNRec(RECOUTS_FIELDS(arg_node));
  TYPfeedVrectype(INFO_VRECTYPE(arg_info), n);

  if (RECOUTS_NEXT(arg_node) != NULL) {
    RECOUTS_NEXT(arg_node) = TRAVdo(RECOUTS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

/* mains for this file */
TYPvrectype *TGVdoGetVRec(node *syntax_tree)
{
  DBUG_ENTER("TGVdoGetVRec");

  TYPvrectype *out = TGVdoGetVRecUC(syntax_tree, TRUE, TRUE);

  DBUG_RETURN(out);
}

TYPvrectype *TGVdoGetVRecUC(node *syntax_tree, bool collectUTs, bool collectCTs)
{
  info *inf;

  DBUG_ENTER("TGVdoGetVRec");

  inf = infoMake(collectUTs, collectCTs);

  TRAVpush(TR_tgv);

  if (syntax_tree != NULL) {
    syntax_tree = TRAVdo(syntax_tree, inf);
  }

  TRAVpop();

  TYPvrectype *out = INFO_VRECTYPE(inf);

  infoFree(inf);

  DBUG_RETURN(out);
}
