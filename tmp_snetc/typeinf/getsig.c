/*******************************************************************************
 *
 * $Id: getsig.c 3378 2012-03-11 11:36:57Z vnn $
 *
 * Sig Getter - generates NETDEF_NTYPESIG object from NETDEF_SIGN node
 *
 *******************************************************************************/

#include "getsig.h"
#include "getvrec.h"
#include "getnrec.h"
#include "typing.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "ctinfo.h"
#include "memory.h"
#include "free.h"

/* The info struct */
struct INFO {
  TYPntypesig *ntypesig;
  bool initializer;
};

/* macros for accessing the fields */
#define INFO_NTYPESIG(i) (i->ntypesig)
#define INFO_INITIALIZER(i) (i->initializer)

/* Makes an info struct. */
static info *infoMake(void)
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_NTYPESIG(result) = TYPnewNtypesig();
  INFO_INITIALIZER(result) = TRUE;

  DBUG_RETURN(result);
}

/* Releases an info struct. */
static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *TGStypesigns(node *arg_node, info *arg_info){
	DBUG_ENTER("TSGtypsigns");

	bool b = INFO_INITIALIZER(arg_info);
	TYPESIGNS_TYPESIG(arg_node) = TRAVdo(TYPESIGNS_TYPESIG(arg_node), arg_info);
	INFO_INITIALIZER(arg_info) &= b;
	if (TYPESIGNS_NEXT(arg_node) != NULL)
		TYPESIGNS_NEXT(arg_node) = TRAVdo(TYPESIGNS_NEXT(arg_node), arg_info);
	INFO_INITIALIZER(arg_info) &= b;

	DBUG_RETURN(arg_node);
}

/* creates a typemap into info.ntypesig. */
node *TGStypemap(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TSGtypemap");

  TYPvrectype *intype = NULL;
  if (TYPEMAP_INTYPE(arg_node) != NULL) {
    intype = TGVdoGetVRec(TYPEMAP_INTYPE(arg_node));
  }

  TYPvrectype *outtype = NULL;
  if (TYPEMAP_OUTTYPE(arg_node) != NULL) {
    outtype = TGVdoGetVRec(TYPEMAP_OUTTYPE(arg_node));
  }

  bool result = TYPjoinNtypesig(INFO_NTYPESIG(arg_info), intype, outtype,
      TYPEMAP_OUTTYPETOINFER(arg_node));
  if (!result) {
    CTIabortNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		 arg_node, "An input type cannot be shared between a "
		 "standard type map and an input-only (output-to-infer) type map");
  }

  TYPEMAP_INITIALIZER(arg_node) = (TYPEMAP_INTYPE(arg_node) == NULL);
  INFO_INITIALIZER(arg_info) = TYPEMAP_INITIALIZER(arg_node);
  DBUG_RETURN(arg_node);
}

/* traverses BoxSign and creates an ntypesig into info */
node *TGSboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGSboxsign");

  TYPvrectype *intype = TGVdoGetVRec(BOXSIGN_INTYPE(arg_node));
  TYPvrectype *outtype = TGVdoGetVRec(BOXSIGN_OUTTYPES(arg_node));
  TYPjoinNtypesig(INFO_NTYPESIG(arg_info), intype, outtype, FALSE);
  BOXSIGN_INITIALIZER(arg_node) = (BOXSIGN_INTYPE(arg_node) == NULL);
  INFO_INITIALIZER(arg_info) = BOXSIGN_INITIALIZER(arg_node);

  DBUG_RETURN(arg_node);
}

/* filt: collects as if it's one ntypemap */
node *TGSfilt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TGSfilt");

  if (FILT_GUARDACTIONS(arg_node) == NULL) { /* special [ ] syntax */
    TYPnrectype *input = TYPnewNrectype();
    TYPvrectype *output = TYPnewVrectype();
    TYPfeedVrectype(output, TYPnewNrectype());
    TYPfeedNtypesig(INFO_NTYPESIG(arg_info), input, output, FALSE);
  }
  else {
    TYPnrectype *input = TGNdoGetNRec(FILT_PATTERN(arg_node));
    TYPvrectype *output = TGVdoGetVRec(FILT_GUARDACTIONS(arg_node));
    TYPfeedNtypesig(INFO_NTYPESIG(arg_info), input, output, FALSE);
  }

  DBUG_RETURN(arg_node);
}

/* main for this file */
TYPntypesig *TGSdoGetSig(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("TGSdoGetSig");

  inf = infoMake();

  TRAVpush(TR_tgs);

  if (syntax_tree != NULL) {
    syntax_tree = TRAVdo(syntax_tree, inf);
  }

  TRAVpop();

  TYPntypesig *out = INFO_NTYPESIG(inf);

  infoFree(inf);

  DBUG_RETURN(out);
}
