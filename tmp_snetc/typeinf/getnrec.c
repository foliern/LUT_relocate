/*******************************************************************************
 *
 * $Id: getnrec.c 2381 2009-05-16 05:44:52Z hxc $
 *
 * Sig Getter - generates NETDEF_NTYPESIG object from NETDEF_SIGN node
 *
 *******************************************************************************/

#include "getnrec.h"
#include "typing.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "free.h"

/* The info struct */
struct INFO {
  TYPnrectype *nrectype;
  lblqual qual;
};

/* macros for accessing the fields */
#define INFO_NRECTYPE(i) (i->nrectype)
#define INFO_QUALIFIER(i) (i->qual)

/* Makes an info struct. */
static info *infoMake(void)
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_NRECTYPE(result) = TYPnewNrectype();
  INFO_QUALIFIER(result) = LQUA_none;
  
  DBUG_RETURN(result);
}

/* Releases an info struct. */
static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

/* populates the nrectype in the info */
node *TGNrecentries(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGNrecentries");
  
  INFO_QUALIFIER(arg_info) = RECENTRIES_QUALIFIER(arg_node);
  if (RECENTRIES_FIELD(arg_node) != NULL) {
    RECENTRIES_FIELD(arg_node) = TRAVdo(RECENTRIES_FIELD(arg_node), arg_info);
  }
  if (RECENTRIES_STAG(arg_node) != NULL) {
    RECENTRIES_STAG(arg_node) = TRAVdo(RECENTRIES_STAG(arg_node), arg_info);
  }
  if (RECENTRIES_BTAG(arg_node) != NULL) {
    RECENTRIES_BTAG(arg_node) = TRAVdo(RECENTRIES_BTAG(arg_node), arg_info);
  }
  if (RECENTRIES_NEXT(arg_node) != NULL) {
    RECENTRIES_NEXT(arg_node) = TRAVdo(RECENTRIES_NEXT(arg_node), arg_info);
  }
  INFO_QUALIFIER(arg_info) = LQUA_none;
  
  DBUG_RETURN(arg_node);
}

node *TGNfieldref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGNfieldref");
  
  TYPfeedNrectype(INFO_NRECTYPE(arg_info), arg_node, INFO_QUALIFIER(arg_info));
  
  DBUG_RETURN(arg_node);
}

node *TGNstagref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGNstagref");
  
  TYPfeedNrectype(INFO_NRECTYPE(arg_info), arg_node, INFO_QUALIFIER(arg_info));
  
  DBUG_RETURN(arg_node);
}

node *TGNbtagref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGNbtagref");
  
  TYPfeedNrectype(INFO_NRECTYPE(arg_info), arg_node, INFO_QUALIFIER(arg_info));
  
  DBUG_RETURN(arg_node);
}

/* outputfields: collects nrectype entries for the filter nrectype */
node *TGNoutputfields(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGNoutputfields");
  
  if (OUTPUTFIELDS_LEFTFIELD(arg_node) != NULL) {
    TYPfeedNrectype(INFO_NRECTYPE(arg_info),
      OUTPUTFIELDS_LEFTFIELD(arg_node),
      (OUTPUTFIELDS_RIGHTFIELD(arg_node) == NULL ||
       OUTPUTFIELDS_RIGHTFIELD(arg_node) == OUTPUTFIELDS_LEFTFIELD(arg_node)
       ? LQUA_pass : LQUA_none));
  }
  else if (OUTPUTFIELDS_STAG(arg_node) != NULL) {
    TYPfeedNrectype(INFO_NRECTYPE(arg_info),
      OUTPUTFIELDS_STAG(arg_node),
      (OUTPUTFIELDS_TAGEXPR(arg_node) == NULL ? LQUA_pass : LQUA_none));
  }
  else {
    TYPfeedNrectype(INFO_NRECTYPE(arg_info),
      OUTPUTFIELDS_BTAG(arg_node), LQUA_none);
  }
  
  if (OUTPUTFIELDS_NEXT(arg_node) != NULL) {
    OUTPUTFIELDS_NEXT(arg_node) = TRAVdo(OUTPUTFIELDS_NEXT(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}

/* main for this file */
TYPnrectype *TGNdoGetNRec(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("TGNdoGetNRec");
  
  inf = infoMake();

  TRAVpush(TR_tgn);

  if (syntax_tree != NULL)
  {
    syntax_tree = TRAVdo(syntax_tree, inf);
  }

  TRAVpop();
  
  TYPnrectype *out = INFO_NRECTYPE(inf);
  
  infoFree(inf);

  DBUG_RETURN(out);
}
