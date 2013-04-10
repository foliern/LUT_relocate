/*******************************************************************************
 *
 * $Id: routeinf.c 3378 2012-03-11 11:36:57Z vnn $
 *
 * Route Inference
 *
 * Max Troy (Haoxuan Cai), Imperial College London,
 * 2009.3-4
 *
 *******************************************************************************/

#include "routeinf.h"
#include "getsig.h"
#include "getvrec.h"
#include "typing.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "ctinfo.h"
#include "memory.h"
#include "copy.h"
#include "free.h"
#include "set.h"
#include "typechk.h"

/* The info struct */
struct INFO {
  bool started;
  set *doneNets;
  TYPvrectype *accepts; /* routing restriction; NULL/empty = no restriction */
  TYPntypesig *sig;
  bool sigowned;
  bool redo;
  bool exact; /* true = restriction should be exact; false = subtype allowed */
  bool isdet; /* for |/|| and !/!! to print the right operand */
  node *tag;
  bool initializer; // for checking top level network
};

/* macros for accessing the fields */
#define INFO_STARTED(i) (i->started)
#define INFO_DONE(i)    (i->doneNets)
#define INFO_ACCEPTS(i) (i->accepts)
#define INFO_SIG(i)     (i->sig)
#define INFO_SIGOWNED(i) (i->sigowned)
#define INFO_REDO(i)    (i->redo)
#define INFO_EXACT(i)   (i->exact)
#define INFO_ISDET(i)   (i->isdet)
#define INFO_TAG(i)     (i->tag)
#define INFO_INITIALIZER(i) (i->initializer)

/* Makes an info struct. */
static info *infoMake(void)
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_STARTED(result) = FALSE;
  INFO_DONE(result) = SETnewSet(NULL);
  INFO_ACCEPTS(result) = NULL;
  INFO_SIG(result) = NULL;
  INFO_SIGOWNED(result) = TRUE;
  INFO_REDO(result) = FALSE;
  INFO_EXACT(result) = FALSE;
  INFO_ISDET(result) = FALSE;
  INFO_TAG(result) = NULL;
  INFO_INITIALIZER(result) = FALSE;

  DBUG_RETURN(result);
}

/* Releases an info struct. */
static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  TYPfreeVrectype(INFO_ACCEPTS(inf));
  SETfreeSet(INFO_DONE(inf));
  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}


/* netrefs: pass to netrefs_net, an attribute that won't be auto traversed */
node *TRInetrefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRInetrefs");

  NETREFS_NET(arg_node) = TRAVdo(NETREFS_NET(arg_node), arg_info);
  node *ptr = NETREFS_NET(arg_node);

  INFO_INITIALIZER(arg_info) = NETDEF_INITIALIZER(ptr);
  DBUG_RETURN(arg_node);
}

/* netbody: ignores Defs node */
node *TRInetbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRInetbody");

  NETBODY_CONNECT(arg_node) = TRAVdo(NETBODY_CONNECT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *TRIboxref(node *arg_node, info *arg_info){
	DBUG_ENTER("TRItypemap");

	node *box_sign = BOXDEF_SIGN(BOXREF_BOX(arg_node));
	INFO_INITIALIZER(arg_info) = BOXSIGN_INITIALIZER(box_sign);

	DBUG_RETURN(arg_node);
}


node *TRItypesigref(node *arg_node, info *arg_info){
	DBUG_ENTER("TRItypesigref");
	if (TYPESIGREF_TYPESIG(arg_node) != NULL)
		TRAVdo (TYPESIGREF_TYPESIG(arg_node), arg_info);
	DBUG_RETURN(arg_node);
}

node *TRItypemap(node *arg_node, info *arg_info){
	DBUG_ENTER("TRItypemap");
	TYPEMAP_INITIALIZER(arg_node) = (TYPEMAP_INTYPE(arg_node) == NULL);
	INFO_INITIALIZER(arg_info) = (TYPEMAP_INTYPE(arg_node) == NULL);
	DBUG_RETURN(arg_node);
}


/* netdefs: dispatch route inf and type check for signed nets */
node *TRInetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRInetdef");
  if (NETDEF_SIGN(arg_node) != NULL) {
	  TSPRINT("check inferred sig\n");
	  TRAVdo(NETDEF_SIGN(arg_node), arg_info);
  }

  if (!INFO_STARTED(arg_info)) {
    if (NETDEF_TOPLEVEL(arg_node)) { /* skip all netdefs until toplevel found */
      if (TSDOPRINT) {
        TSPRINT("Route inference starts with top-level net %s at %s:%d.%d.",
            NETDEF_NAME(arg_node), NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
      }
      INFO_STARTED(arg_info) = TRUE;
    }
  }

  if (INFO_STARTED(arg_info)) {

    /* regen ntypesig from sign */
    if (NETDEF_SIGN(arg_node) != NULL && NETDEF_NTYPESIG(arg_node) == NULL) {
      NETDEF_NTYPESIG(arg_node) = TGSdoGetSig(NETDEF_SIGN(arg_node));
    }

    /* if external net, does nothing */

    if (NETDEF_EXTERNAL(arg_node)) {
    }

    /* for internal signed net, route & type check internally */

    else if (NETDEF_SIGNED(arg_node)) {
      if (!SEThasElem(INFO_DONE(arg_info), arg_node)) {
        /* only continue if not done yet */
        INFO_DONE(arg_info) = SETaddElem(INFO_DONE(arg_info), arg_node);
        /* apply new restriction */
        TYPvrectype *oldAccepts = INFO_ACCEPTS(arg_info);
        bool oldExact = INFO_EXACT(arg_info);
        INFO_ACCEPTS(arg_info) = TYPntypesigIns(NETDEF_NTYPESIG(arg_node));
        INFO_EXACT(arg_info) = FALSE;
        if (!INFO_SIGOWNED(arg_info)) {
          TYPfreeNtypesig(INFO_SIG(arg_info));
        }
        INFO_SIG(arg_info) = NETDEF_NTYPESIG(arg_node);
        INFO_SIGOWNED(arg_info) = TRUE;
        NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
        /* restore restriction */
        TYPfreeVrectype(INFO_ACCEPTS(arg_info));
        INFO_ACCEPTS(arg_info) = oldAccepts;
        INFO_EXACT(arg_info) = oldExact;
        /* type check me */
        arg_node = TCdoTypeCheck(arg_node);
      }
    }

    /* for internal unsigned net, traverse body */

    else if (NETDEF_TOPLEVEL(arg_node) || /* for toplevel unsigned there's a
                                             block at the end to treat it */
        NETDEF_SIGN(arg_node) == NULL ||
        INFO_REDO(arg_info)) {   /* see Serial comments for use of "Redo" */

      if (!INFO_SIGOWNED(arg_info)) {
        TYPfreeNtypesig(INFO_SIG(arg_info));
      }
      INFO_SIG(arg_info) = NETDEF_NTYPESIG(arg_node);
      INFO_SIGOWNED(arg_info) = TRUE;
      NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
      TYPntypesig *mysig = INFO_SIG(arg_info);

      if (mysig != NETDEF_NTYPESIG(arg_node)) { /* something changed */
        NETDEF_NTYPESIG(arg_node) = TYPfreeNtypesig(NETDEF_NTYPESIG(arg_node));
        if (NETDEF_SIGN(arg_node) != NULL) {
          NETDEF_SIGN(arg_node) = FREEdoFreeTree(NETDEF_SIGN(arg_node));
        }
        NETDEF_NTYPESIG(arg_node) = INFO_SIGOWNED(arg_info) ?
            TYPcopyNtypesig(INFO_SIG(arg_info)) : INFO_SIG(arg_info);
        NETDEF_SIGN(arg_node) = TYPcreateTypeSigns(mysig);

        if (TSDOPRINT) {
          char *chrssig = TYPprintNtypesig(mysig);
          TSPRINT("Net %s at %s:%d.%d is inferred this sig:\n%s.",
              NETDEF_NAME(arg_node), NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node), chrssig);
          MEMfree(chrssig);
        }

        if (TYPisEmptyNtypesig(mysig)) {
          if (NETDEF_TOPLEVEL(arg_node)) {
            CTIabortNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
			 arg_node, "Top-level net is inferred an empty sig "
			 "meaning type error");
          }
          else {
            CTIwarnNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
			arg_node, "Net %s is inferred an empty sig, type error "
			"will be reported later if this net is used or has init boxes",
			NETDEF_NAME(arg_node));
          }
        }
      }

      if (NETDEF_TOPLEVEL(arg_node)) {
        /* toplevel but unsigned? issue warning and perform type check on
         * inferred typemap */
        CTIwarnNode(CTI_ERRNO_SIGNATURE_ERROR,
		    arg_node, "Top-level net does not have a signature. "
		    "Using inferred sig for type-check.");
        NETDEF_SIGNED(arg_node) = TRUE;
        INFO_DONE(arg_info) = SETaddElem(INFO_DONE(arg_info), arg_node);
        /* type check me */
        TCdoTypeCheck(arg_node);

      }

    } /* end if internal unsigned net */

    /* output sig to caller */
    INFO_SIG(arg_info) = NETDEF_NTYPESIG(arg_node);
    INFO_SIGOWNED(arg_info) = TRUE; /* tells caller: I own my sig */
  }


  DBUG_RETURN(arg_node);
}


/* in Serial by NetRefs */
node *TRIserial(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRIserial");

  TYPvrectype *oldAccepts = INFO_ACCEPTS(arg_info);
  bool oldExact = INFO_EXACT(arg_info);

  /* trav left with unchanged restriction */
  SERIAL_LEFT(arg_node) = TRAVdo(SERIAL_LEFT(arg_node), arg_info);

  if INFO_INITIALIZER(arg_info)
		  CTIabortNode(CTI_ERRNO_OPERATION_INITIALIZER_ERROR,
		  			 arg_node, "ERROR: Initialiser boxes/subnets are not allowed as an expression of the serial operation");

  TYPntypesig *lnts = INFO_SIG(arg_info);
  INFO_SIG(arg_info) = NULL;
  DBUG_ASSERT(lnts != NULL, "Can't get serial left operand signature, "
      "AST may be tampered");
  bool doFreeLnts = !INFO_SIGOWNED(arg_info);

  /* trav right with no restrictions */
  bool oldRedo = INFO_REDO(arg_info);
  INFO_REDO(arg_info) = FALSE; /* if a sig is found then just use the sig,
      instead of going deeper to re-infer everything,
      because the right operand will not be affected by route reducing */
  INFO_ACCEPTS(arg_info) = NULL;
  SERIAL_RIGHT(arg_node) = TRAVdo(SERIAL_RIGHT(arg_node), arg_info);

  if INFO_INITIALIZER(arg_info)
  		  CTIabortNode(CTI_ERRNO_OPERATION_INITIALIZER_ERROR,
  		  			 arg_node, "ERROR: Initialiser boxes/subnets are not allowed as an expression of the serial operation");


  INFO_REDO(arg_info) = oldRedo;
  TYPntypesig *rnts = INFO_SIG(arg_info);
  INFO_SIG(arg_info) = NULL;
  DBUG_ASSERT(rnts != NULL, "Can't get serial right operand signature, "
      "AST may be tampered");
  bool doFreeRnts = !INFO_SIGOWNED(arg_info);


  if (TSDOPRINT) {
    char *chrssig = TYPprintNtypesig(lnts);
    TSPRINT("At '..' at %s:%d.%d,", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    TSPRINT("Left sig: \n%s", chrssig);
    MEMfree(chrssig);
    chrssig = TYPprintNtypesig(rnts);
    TSPRINT("Right sig: \n%s", chrssig);
    MEMfree(chrssig);
  }

  /* finally, inference */
  INFO_SIG(arg_info) = TYPinferDotDot(arg_node, lnts, rnts);
  INFO_SIGOWNED(arg_info) = FALSE;
  INFO_ACCEPTS(arg_info) = oldAccepts;
  INFO_EXACT(arg_info) = oldExact;

  if (doFreeLnts) {
    TYPfreeNtypesig(lnts);
  }
  if (doFreeRnts) {
    TYPfreeNtypesig(rnts);
  }

  DBUG_RETURN(arg_node);
}

node *TRIchoice(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRIchoice");

  INFO_ISDET(arg_info) = CHOICE_ISDETERM(arg_node);
  CHOICE_BRANCHLIST(arg_node) = TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);
  INFO_INITIALIZER(arg_info) = FALSE;

  DBUG_RETURN(arg_node);
}

/* in BranchList by NetRefs->Choice */
node *TRIbranchlist(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRIbranchlist");

  TYPvrectype *oldAccepts = INFO_ACCEPTS(arg_info);
  bool oldExact = INFO_EXACT(arg_info);
  bool oldRedo = INFO_REDO(arg_info);

  bool goDeep = FALSE;
  TYPvrectype *attracts = NULL;
  if (BRANCHLIST_TYPED(arg_node)) { /* if was typed then merge restrictions */
    attracts = TGVdoGetVRec(BRANCHLIST_ATTRACTS(arg_node));
    if (oldAccepts != NULL) {
      goDeep = TYPadaptRoutingInfo(attracts, oldAccepts, oldExact);
      /* if has changes then go deep */
    }
    /* if oldAccepts == NULL we don't even need to redo anything but
     * just get back a sig */
  }

  /* go deep inference or just get the sig */
  if (goDeep) {
    INFO_REDO(arg_info) = TRUE;
    INFO_ACCEPTS(arg_info) = attracts;
    INFO_EXACT(arg_info) = TRUE;
  }
  else {
    INFO_REDO(arg_info) = FALSE;
  }
  BRANCHLIST_BRANCH(arg_node) = TRAVdo(BRANCHLIST_BRANCH(arg_node), arg_info);
  INFO_REDO(arg_info) = oldRedo;
  INFO_ACCEPTS(arg_info) = oldAccepts;
  INFO_EXACT(arg_info) = oldExact;

  /* the sig */
  TYPntypesig *sig = INFO_SIG(arg_info);
  INFO_SIG(arg_info) = NULL;
  DBUG_ASSERT(sig != NULL,
      "Can't get parallel branch sig, AST may be tampered");
  if (INFO_SIGOWNED(arg_info)) {
    sig = TYPcopyNtypesig(sig);
  }

  /* re-apply the restrictions */
  if (attracts == NULL) { /* wasn't typed but now typed, redo restrictions */
    attracts = TYPntypesigIns(sig);
    if (oldAccepts != NULL) {
      TYPadaptRoutingInfo(attracts, oldAccepts, oldExact);
    }
  }
  TYPntypesig *sig2 = TYPinferBarBranch(sig, attracts);
  TYPfreeNtypesig(sig);
  sig = sig2;
  TYPfreeVrectype(attracts);
  attracts = TYPntypesigIns(sig);

  /* writes routing info */
  if (BRANCHLIST_ATTRACTS(arg_node) != NULL) {
    FREEdoFreeTree(BRANCHLIST_ATTRACTS(arg_node));
  }
  BRANCHLIST_ATTRACTS(arg_node) = TYPcreateTypesNode(attracts);
  BRANCHLIST_TYPED(arg_node) = TRUE;

  if (TSDOPRINT) {
    char *chrsattr = TYPprintVrectype(attracts);
    TSPRINT(INFO_ISDET(arg_info) ?
        "'||' branch at %s:%d.%d attracts %s." :
        "'|' branch at %s:%d.%d attracts %s.",
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node), chrsattr);
    MEMfree(chrsattr);
  }

  TYPfreeVrectype(attracts);

  /* do next */
  if (BRANCHLIST_NEXT(arg_node) != NULL) {
    BRANCHLIST_NEXT(arg_node) = TRAVdo(BRANCHLIST_NEXT(arg_node), arg_info);
    DBUG_ASSERT(INFO_SIG(arg_info) != NULL,
        "Can't get parallel branch sig, AST may be tampered");
    bool succeed = TYPntypesigMergeInto(sig, INFO_SIG(arg_info));
    DBUG_ASSERT(succeed, "Algorithm Error, couldn't mix input-only with "
        "standard typemaps in parallel branchlist");
    if (!INFO_SIGOWNED(arg_info)) {
      TYPfreeNtypesig(INFO_SIG(arg_info));
    }
  }

  /* return my sig */
  INFO_SIG(arg_info) = sig;
  INFO_SIGOWNED(arg_info) = FALSE;

  DBUG_RETURN(arg_node);
}

/* in Star by NetRefs */
node *TRIstar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRIstar");

  TYPvrectype *oldAccepts = INFO_ACCEPTS(arg_info);
  bool oldExact = INFO_EXACT(arg_info);

  /* get u/c termination patterns */
  TYPvrectype *uterm = TGVdoGetVRecUC(STAR_TERM(arg_node), TRUE, FALSE);
  TYPvrectype *cterm = TGVdoGetVRecUC(STAR_TERM(arg_node), FALSE, TRUE);

  /* trav left with unchanged restriction to get the basic sig */
  STAR_LEFT(arg_node) = TRAVdo(STAR_LEFT(arg_node), arg_info);
  if INFO_INITIALIZER(arg_info)
  		  CTIabortNode(CTI_ERRNO_OPERATION_INITIALIZER_ERROR,
  		  			 arg_node, "ERROR: Initialiser boxes/subnets are not allowed as an expression of the star operation");


  TYPntypesig *sig = INFO_SIG(arg_info);
  INFO_SIG(arg_info) = NULL;
  DBUG_ASSERT(sig != NULL, "Can't get star left operand signature, "
      "AST may be tampered");
  if (INFO_SIGOWNED(arg_info)) {
    sig = TYPcopyNtypesig(sig);
  }

  if (TSDOPRINT) {
    char *chrssig = TYPprintNtypesig(sig);
    TSPRINT(STAR_ISDETERM(arg_node) ? "At '**' at %s:%d.%d," : "At '*' at %s:%d.%d,", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    TSPRINT("  Operand sig: \n%s,", chrssig);
    MEMfree(chrssig);
    char *chrsuterm = TYPprintVrectype(uterm);
    TSPRINT("  Unconditional termination: %s,", chrsuterm);
    MEMfree(chrsuterm);
    char *chrscterm = TYPprintVrectype(cterm);
    TSPRINT("  Conditional termination: %s.", chrscterm);
    MEMfree(chrscterm);
  }

  /* actual inference */
  INFO_SIG(arg_info) = TYPinferStar(arg_node, sig, uterm, cterm);
  INFO_SIGOWNED(arg_info) = FALSE;
  INFO_ACCEPTS(arg_info) = oldAccepts;
  INFO_EXACT(arg_info) = oldExact;

  /* clean up */
  TYPfreeNtypesig(sig);
  TYPfreeVrectype(uterm);
  TYPfreeVrectype(cterm);
  DBUG_RETURN(arg_node);
}


/* in Feedback by NetRefs */
node *TRIfeedback(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRIfeedback");

  TYPvrectype *oldAccepts = INFO_ACCEPTS(arg_info);
  bool oldExact = INFO_EXACT(arg_info);

  /*XXX
   * dlp
   *
   * This is the workaround in the current compiler for type-inference
   * of the feedback operator...
   */
  DBUG_ASSERT(oldAccepts != NULL, "Feedback operator must be placed into a "
      "type-annotated network for now!");


  /* get u/c feedback patterns */
  TYPvrectype *uback = TGVdoGetVRecUC(FEEDBACK_BACK(arg_node), TRUE, FALSE);
  TYPvrectype *cback = TGVdoGetVRecUC(FEEDBACK_BACK(arg_node), FALSE, TRUE);

  /* trav left with unchanged restriction to get the basic sig */
  FEEDBACK_LEFT(arg_node) = TRAVdo(FEEDBACK_LEFT(arg_node), arg_info);
  if INFO_INITIALIZER(arg_info)
  		  CTIabortNode(CTI_ERRNO_OPERATION_INITIALIZER_ERROR,
  		  			 arg_node, "ERROR: Initialiser boxes/subnets are not allowed as an expression of the feedback operation");

  TYPntypesig *sig = INFO_SIG(arg_info);
  INFO_SIG(arg_info) = NULL;
  DBUG_ASSERT(sig != NULL, "Can't get feedback left operand signature, "
      "AST may be tampered");
  if (INFO_SIGOWNED(arg_info)) {
    sig = TYPcopyNtypesig(sig);
  }

  if (TSDOPRINT) {
    char *chrssig = TYPprintNtypesig(sig);
    TSPRINT("At '\\' at %s:%d.%d,", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    TSPRINT("  Operand sig: \n%s,", chrssig);
    MEMfree(chrssig);
    char *chrsuback = TYPprintVrectype(uback);
    TSPRINT("  Unconditional feedback: %s,", chrsuback);
    MEMfree(chrsuback);
    char *chrscback = TYPprintVrectype(cback);
    TSPRINT("  Conditional feedback: %s.", chrscback);
    MEMfree(chrscback);
  }

  /* actual inference */
  //FIXME if desired INFO_SIG(arg_info) = TYPinferFeedback(arg_node, sig, uback, cback);
  INFO_SIGOWNED(arg_info) = FALSE;
  INFO_ACCEPTS(arg_info) = oldAccepts;
  INFO_EXACT(arg_info) = oldExact;

  /* clean up */
  TYPfreeNtypesig(sig);
  TYPfreeVrectype(uback);
  TYPfreeVrectype(cback);
  DBUG_RETURN(arg_node);
}


node *TRIsplit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRIsplit");

  /* trav left with unchanged restriction to get the sig */
  SPLIT_LEFT(arg_node) = TRAVdo(SPLIT_LEFT(arg_node), arg_info);
  if INFO_INITIALIZER(arg_info)
  		  CTIabortNode(CTI_ERRNO_OPERATION_INITIALIZER_ERROR,
  		  			 arg_node, "ERROR: Initialiser boxes/subnets are not allowed as an expression of the split operation");

  TYPntypesig *sig = INFO_SIG(arg_info);
  INFO_SIG(arg_info) = NULL;
  DBUG_ASSERT(sig != NULL, "Can't get split left operand signature, "
      "AST may be tampered");
  bool doFreeSig = !INFO_SIGOWNED(arg_info);

  if (TSDOPRINT) {
    char *chrssig = TYPprintNtypesig(sig);
    TSPRINT(SPLIT_ISDETERM(arg_node) ? "At '!!' at %s:%d.%d," : "At '!' at %s:%d.%d,", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    TSPRINT("  Operand sig: \n%s,", chrssig);
    MEMfree(chrssig);
  }

  /* get tag */
  SPLIT_RANGE(arg_node) = TRAVdo(SPLIT_RANGE(arg_node), arg_info);

  /* inference */
  INFO_SIG(arg_info) = TYPinferEx(sig, INFO_TAG(arg_info));;
  INFO_SIGOWNED(arg_info) = FALSE;

  if (doFreeSig) {
    TYPfreeNtypesig(sig);
  }

  DBUG_RETURN(arg_node);
}

node *TRIrange(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TRIrange");

  if (RANGE_STAGSTART(arg_node) != NULL) {
    INFO_TAG(arg_info) = RANGE_STAGSTART(arg_node);
    if (TSDOPRINT) {
      TSPRINT("  Indexing tag: <%s>.",
          STAGS_NAME(STAGREF_STAG(INFO_TAG(arg_info))));
    }
  }
  else if (RANGE_BTAGSTART(arg_node) != NULL) {
    INFO_TAG(arg_info) = RANGE_BTAGSTART(arg_node);
    if (TSDOPRINT) {
      TSPRINT("  Indexing tag: <#%s>.",
          BTAGS_NAME(BTAGREF_BTAG(INFO_TAG(arg_info))));
    }
  }
  else {
    CTIabortNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		 arg_node, "Indexing tag missing");
  }

  DBUG_RETURN(arg_node);
}

/* main for this file */
node *TRIdoRouteInf(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("TRIdoRouteInf");

  DBUG_ASSERT((syntax_tree != NULL), "TRIdoRouteInf called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_tri);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
