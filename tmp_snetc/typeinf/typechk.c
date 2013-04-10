/******************************************************************************
 *
 * $Id: typechk.c 3378 2012-03-11 11:36:57Z vnn $
 *
 * Type Check
 *
 * Max Troy (Haoxuan Cai), Imperial College London,
 * 2009.3-4
 *
 ******************************************************************************/

#include "typechk.h"
#include "typing.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "ctinfo.h"
#include "memory.h"
#include "copy.h"
#include "free.h"
#include "str.h"
#include "set.h"
#include "getsig.h"
#include "getvrec.h"

/* The info struct */
struct INFO {
  bool started;
  TYPnrectype *input;
  TYPvrectype *outputs;
  TYPntypesig *lastsig;
  int branchms;
  bool branchgo;
  bool isdet;
  bool hasinit;
};

/* macros for accessing the fields */
#define INFO_STARTED(i) (i->started)
#define INFO_INPUT(i)   (i->input)
#define INFO_OUTPUTS(i) (i->outputs)
#define INFO_LASTSIG(i) (i->lastsig)
#define INFO_BRANCHMS(i) (i->branchms)
#define INFO_BRANCHGO(i) (i->branchgo)
#define INFO_ISDET(i)    (i->isdet)
#define INFO_HASINIT(i) (i->hasinit)

/* Makes an info struct. */
static info *infoMake(void)
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_STARTED(result) = FALSE;
  INFO_INPUT(result) = NULL;
  INFO_OUTPUTS(result) = NULL;
  INFO_LASTSIG(result) = NULL;
  INFO_BRANCHMS(result) = -1;
  INFO_BRANCHGO(result) = FALSE;
  INFO_ISDET(result) = FALSE;
  INFO_HASINIT(result) = FALSE;

  DBUG_RETURN(result);
}

/* Releases an info struct. */
static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

#define PRINT_AND_OUTPUT(vrt) { \
  if (TSDOPRINT) { \
    char *char_star_vrt_string = TYPprintVrectype(vrt); \
    TSPRINT("  produces %s.", char_star_vrt_string); \
    MEMfree(char_star_vrt_string); \
  } \
  INFO_OUTPUTS(arg_info) = TYPcombineVrectype(INFO_OUTPUTS(arg_info), vrt); \
}

/* traverses NetDef as the main entrance of typechk phase */
static node *TCnetdefEntrance(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCnetdefEntrance");

  /* if sig has any output-to-infer (inputs-only) typemaps, this function
   * will complete them. mark here so we can print the completed sig later */
  bool wasIncomplete = TYPisIncompleteNtypesig(NETDEF_NTYPESIG(arg_node));

  NETDEF_INITIALIZER(arg_node) = TYPisInitializerNtypesig(NETDEF_NTYPESIG(arg_node));
  if (NETDEF_INITIALIZER(arg_node) && NETDEF_TOPLEVEL(arg_node)) {
	  CTIabortNode(CTI_ERRNO_TOPLEVEL_NET_INITIALIZER_ERROR,
	    				 arg_node, "ERROR: Top level network %s can not be an initialiser (i.e. no input type)", NETDEF_NAME(arg_node));
  }


  /* all possible inputs */
  TYPvrectype *myInputs = TYPntypesigIns(NETDEF_NTYPESIG(arg_node));

  /* some printing */
  char *chrsloc = NULL;
  if (TSDOPRINT) {
    char *chrsins = TYPprintVrectype(myInputs);
    TSPRINT("Type-checking net %s at %s:%d.%d with these inputs: %s.",
        NETDEF_NAME(arg_node), NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node), chrsins);
  }

  /* check initters. activated just once from here. */
  INFO_INPUT(arg_info) = NULL;
  INFO_LASTSIG(arg_info) = NETDEF_NTYPESIG(arg_node);
  if (TSDOPRINT) {
    TSPRINT("Sending init signal");
  }
  NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  CTIabortOnError();

  TYPvrectype *myOutputs = INFO_OUTPUTS(arg_info);
  INFO_OUTPUTS(arg_info) = NULL;
  /* wrong: iff there has been any initters, myOutputs != NULL
   * right: initter generated outputs may be sink'd, making myOutputs NULL,
   * so we should look at INFO_HASINIT flag which will be flagged by an
   * initter. */
  if (!INFO_HASINIT(arg_info)) {
    if (TYPhasInitMap(NETDEF_NTYPESIG(arg_node))) {
      CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		   arg_node, "Net %s declares an init map but there are "
		   "no init boxes in the topology.", NETDEF_NAME(arg_node));
    }
  }
  else { /* has init outputs */
    if (TYPhasInitMap(NETDEF_NTYPESIG(arg_node))) {
      if (!TYPcheckOutputs(NETDEF_NTYPESIG(arg_node), NULL, myOutputs)) {
        TYPvrectype *declared = TYPgetInitResults(NETDEF_NTYPESIG(arg_node));
        char *chrsiouts = TYPprintVrectype(myOutputs);
        char *chrsdouts = TYPprintVrectype(declared);
        CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		     arg_node, "Inferred init box outputs in net %s "
		     "not compatible with the declared outputs.\nInferred: %s\n"
		     "Declared: %s", NETDEF_NAME(arg_node), chrsiouts, chrsdouts);
        TYPfreeVrectype(declared);
        MEMfree(chrsiouts);
        MEMfree(chrsdouts);
      }
    }
    else { /* no init typemap in the sig */
      char *chrsiouts = TYPprintVrectype(myOutputs);
      CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		   arg_node, "Net %s has no init maps capturing the init box"
		   " outputs.\nInferred init outputs: %s", NETDEF_NAME(arg_node),
		   chrsiouts);
      MEMfree(chrsiouts);
    }
    TYPfreeVrectype(myOutputs);
  }
  CTIabortOnError();

  /* checking inputs one by one */
  TYPnrectype *myInput;
  while ((myInput = TYPpopVrectype(myInputs)) != NULL) {
    if (TSDOPRINT) {
      char *chrsin = TYPprintNrectype(myInput);
      TSPRINT("Sending %s from start of net %s at %s",
        chrsin, NETDEF_NAME(arg_node), chrsloc);
      MEMfree(chrsin);
    }
    /* set all pass for proper pass-through evaluation */
    TYPsetAllPass(myInput);
    INFO_INPUT(arg_info) = myInput;
    INFO_LASTSIG(arg_info) = NETDEF_NTYPESIG(arg_node);
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
    CTIabortOnError();

    myOutputs = INFO_OUTPUTS(arg_info);
    INFO_OUTPUTS(arg_info) = NULL;
    if (!TYPcheckOutputs(NETDEF_NTYPESIG(arg_node), myInput, myOutputs)) {
      char *chrsin = TYPprintNrectype(myInput);
      char *chrsouts = TYPprintVrectype(myOutputs);
      char *chrssig = TYPprintNtypesig(NETDEF_NTYPESIG(arg_node));
      CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		   arg_node, "Inferred outputs in net %s "
		   "not compatible with the declared outputs. Net sig:\n"
		   "%s\nInput in question: %s\nInferred outputs: %s",
          NETDEF_NAME(arg_node), chrssig, chrsin, chrsouts);
      MEMfree(chrsin);
      MEMfree(chrsouts);
      MEMfree(chrssig);
    }
    TYPfreeNrectype(myInput);
    TYPfreeVrectype(myOutputs);
    CTIabortOnError();
  }

  TYPfreeVrectype(myInputs);

  if (wasIncomplete) {
    if (TSDOPRINT) {
      char *chrssig = TYPprintNtypesig(NETDEF_NTYPESIG(arg_node));
      TSPRINT("Sig of net %s at %s has been completed as:\n%s.",
        NETDEF_NAME(arg_node), chrsloc, chrssig);
      MEMfree(chrssig);
    }
    FREEdoFreeTree(NETDEF_SIGN(arg_node));
    NETDEF_SIGN(arg_node) = TYPcreateTypeSigns(NETDEF_NTYPESIG(arg_node));
  }

  if (TSDOPRINT) {
    TSPRINT("Type check for net %s at %s passed.", NETDEF_NAME(arg_node),
        chrsloc);
    MEMfree(chrsloc);
  }

  DBUG_RETURN(arg_node);
}



/* traverses NetDef as called by NetRefs */
static node *TCnetdefCalledByNetRefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCnetdefCalledByNetRefs");



  if (NETDEF_EXTERNAL(arg_node)){
	  // || NETDEF_SIGNED(arg_node)) { /* allow to travel in signed network to verify if it is a initter */
    /* this is a boundary network, do not traverse in, use NTypeSig */

    if (TSDOPRINT) {
      TSPRINT(" into boundary net %s at %s:%d.%d", NETDEF_NAME(arg_node),
          NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    if (INFO_INPUT(arg_info) == NULL) { /* to do init */

      TYPvrectype *myOutputs = TYPgetInitResults(NETDEF_NTYPESIG(arg_node));
      if (myOutputs != NULL) {
        PRINT_AND_OUTPUT(myOutputs);
      }

    }
    else { /* INFO_INPUT != NULL */

      TYPvrectype *myOutputs =
        TYPgetOutputs(NETDEF_NTYPESIG(arg_node), INFO_INPUT(arg_info));
      if (myOutputs == NULL) {
        char *chrssig = TYPprintNtypesig(NETDEF_NTYPESIG(arg_node));
        char *chrsin = TYPprintNrectype(INFO_INPUT(arg_info));
        CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		     arg_node,
		     "Sig of net %s:\n%s\ndoes not accept input %s",
		     NETDEF_NAME(arg_node), chrssig, chrsin);
        MEMfree(chrssig);
        MEMfree(chrsin);
      }
      else {
        PRINT_AND_OUTPUT(myOutputs);
      }

    }

    /* end if boundary network */
  }
  else {
    /* this is a simple wrapper netdef. traverse in for connect expr */

    INFO_LASTSIG(arg_info) = NETDEF_NTYPESIG(arg_node);
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

/* dispatcher of the two functions above */
node *TCnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCnetdef");

  if (NETDEF_SIGN(arg_node) != NULL && NETDEF_NTYPESIG(arg_node) == NULL) {
    /* regenerate NTypeSig in case SUBPH_sip (sig inf prim) has populated
     * NETDEF_SIGN nodes but the compilation gets terminated, NULLifying
     * NETDEF_NTYPESIG attribs */
    NETDEF_NTYPESIG(arg_node) = TGSdoGetSig(NETDEF_SIGN(arg_node));
  }

  if (INFO_STARTED(arg_info)) {
    /* typecheck started, this is called from a NetRefs node */
    arg_node = TCnetdefCalledByNetRefs(arg_node, arg_info);
  }
  else {
    /* typecheck not started, do start */
    INFO_STARTED(arg_info) = TRUE;
    arg_node = TCnetdefEntrance(arg_node, arg_info);
  }

  DBUG_RETURN(arg_node);
}

/* traversed in boxref by netdef */
node *TCboxref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCboxref");

  node *box = BOXREF_BOX(arg_node);
  TYPntypesig *sig = INFO_LASTSIG(arg_info); /* must be a wrapper net outside,
      don't use Box.NTypeSig as it may be NULL caused by resumed compilation */
  if (TSDOPRINT) {
    TSPRINT(" into box %s at %s:%d.%d", BOXDEF_NAME(box),
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
  }

  if (INFO_INPUT(arg_info) == NULL) { /* to do init */
    TYPvrectype *initouts = TYPgetInitResults(sig);
    if (initouts == NULL) {
      if (TSDOPRINT) {
        TSPRINT("  which is not an init box, passes through.");
      }
    }
    else {
      INFO_HASINIT(arg_info) = TRUE;
      PRINT_AND_OUTPUT(initouts);
    }
  }
  else { /* (INFO_INPUT(arg_info) != NULL) */
    TYPvrectype *outs = TYPgetOutputs(sig, INFO_INPUT(arg_info));
    if (outs == NULL) {
      char *in = TYPprintNrectype(INFO_INPUT(arg_info));

      CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		   arg_node, "Box %s does not accept input %s",
		   BOXDEF_NAME(box), in);
      MEMfree(in);
    }
    else {
      PRINT_AND_OUTPUT(outs);
    }
  }

  DBUG_RETURN(arg_node);
}

/* traversed in filt by netdef */
node *TCfilt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCfilt");

  TYPntypesig *sig = INFO_LASTSIG(arg_info); /* must be a wrapper net outside */
  if (TSDOPRINT) {
    TSPRINT(" into filter at %s:%d.%d", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
  }

  if (INFO_INPUT(arg_info) == NULL) { /* do init */
    if (TSDOPRINT) {
      TSPRINT("  which does not handle init signal, passes through.");
    }
  }
  else {
    TYPvrectype *outs = TYPgetOutputs(sig, INFO_INPUT(arg_info));
    if (outs == NULL) {
      char *in = TYPprintNrectype(INFO_INPUT(arg_info));
      CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		   arg_node, "Filter does not accept input %s", in);
      MEMfree(in);
    }
    else {
      PRINT_AND_OUTPUT(outs);
    }
  }

  DBUG_RETURN(arg_node);
}

/* traversed in sync by netdef */
node *TCsync(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCsync");

  TYPntypesig *sig = INFO_LASTSIG(arg_info); /* must be a wrapper net outside */
  if (TSDOPRINT) {
    TSPRINT(" into synchrocell at %s:%d.%d",
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
  }

  if (INFO_INPUT(arg_info) == NULL) { /* do init */
    if (TSDOPRINT) {
      TSPRINT("  which does not handle init signal, passes through.");
    }
  }
  else {
    TYPvrectype *outs = TYPgetOutputs(sig, INFO_INPUT(arg_info));
    if (outs == NULL) {
      char *in = TYPprintNrectype(INFO_INPUT(arg_info));
      CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		   arg_node, "Synchrocell does not accept input %s", in);
      MEMfree(in);
    }
    else {
      PRINT_AND_OUTPUT(outs);
    }
  }

  DBUG_RETURN(arg_node);
}

/* traversed in NetRefs by netdef */
node *TCnetrefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCnetrefs");

  /* NETREFS_NET is an attribute and won't be traversed by travsons */
  NETREFS_NET(arg_node) = TRAVdo(NETREFS_NET(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/* traversed in Serial by netdef */
node *TCserial(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCserial");

  if (TSDOPRINT) {
    TSPRINT(" into '..' at %s:%d.%d", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
  }

  bool doInit = INFO_INPUT(arg_info) == NULL;

  /* do left */
  SERIAL_LEFT(arg_node) = TRAVdo(SERIAL_LEFT(arg_node), arg_info);
  CTIabortOnError();

  TYPvrectype *mid = INFO_OUTPUTS(arg_info);
  INFO_OUTPUTS(arg_info) = NULL;

  /* do right for each mid output,
   * but if there was an init signal, do init signal first,
   * and combine all outputs together */

  TYPvrectype *out = NULL;

  /* do right init */
  if (doInit) {
    INFO_INPUT(arg_info) = NULL;
    if (TSDOPRINT) {
      TSPRINT("Passing init signal across '..' at %s:%d.%d", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    SERIAL_RIGHT(arg_node) = TRAVdo(SERIAL_RIGHT(arg_node), arg_info);
    CTIabortOnError();
    out = TYPcombineVrectype(out, INFO_OUTPUTS(arg_info));
    INFO_OUTPUTS(arg_info) = NULL;
  }

  /* send mid outputs to right */
  TYPnrectype *mid1;
  while ((mid1 = TYPpopVrectype(mid)) != NULL) {
    if (TSDOPRINT) {
      char *chrsmid1 = TYPprintNrectype(mid1);
      TSPRINT("Passing %s across '..' at %s:%d.%d", chrsmid1, NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
      MEMfree(chrsmid1);
    }
    INFO_INPUT(arg_info) = mid1;
    SERIAL_RIGHT(arg_node) = TRAVdo(SERIAL_RIGHT(arg_node), arg_info);
    CTIabortOnError();
    out = TYPcombineVrectype(out, INFO_OUTPUTS(arg_info));
    INFO_OUTPUTS(arg_info) = NULL;
    TYPfreeNrectype(mid1);
  }

  /* combined output */
  INFO_OUTPUTS(arg_info) = out;

  DBUG_RETURN(arg_node);
}

/* traversed in Choice by netdef */
node *TCchoice(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCchoice");

  if (TSDOPRINT) {
    INFO_ISDET(arg_info) = CHOICE_ISDETERM(arg_node);
    TSPRINT(INFO_ISDET(arg_info) ?
        " into '||' at %s:%d.%d, where" : " into '|' at %s:%d.%d, where",
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
  }

  if (INFO_INPUT(arg_info) == NULL) { /* do init, every branch goes */
    CHOICE_BRANCHLIST(arg_node) = TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);
  }
  else { /* not init; best matching branches go */
    INFO_BRANCHMS(arg_info) = -1;
    INFO_BRANCHGO(arg_info) = FALSE; /* do find best match first */
    CHOICE_BRANCHLIST(arg_node) = TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);
    if (INFO_BRANCHMS(arg_info) == -1) {
      char *in = TYPprintNrectype(INFO_INPUT(arg_info));
      CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		   arg_node, "No branches of this parallel composition"
		   " accepts input %s", in);
      MEMfree(in);
    }
    else {
      INFO_BRANCHGO(arg_info) = TRUE; /* now go */
      CHOICE_BRANCHLIST(arg_node) =
          TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);
    }
  }

  DBUG_RETURN(arg_node);
}

/* traversed in BranchList by Choice */
node *TCbranchlist(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCbranchlist");

  bool isdet = INFO_ISDET(arg_info);

  TYPnrectype *input = INFO_INPUT(arg_info);

  if (input == NULL) {
    /* do init */

    if (TSDOPRINT) {
      TSPRINT(isdet ?
          "Routing init signal to '||' branch at %s:%d.%d" :
          "Routing init signal to '|' branch at %s:%d.%d",
          NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    BRANCHLIST_BRANCH(arg_node) = TRAVdo(BRANCHLIST_BRANCH(arg_node), arg_info);
    CTIabortOnError();
    if (BRANCHLIST_NEXT(arg_node) != NULL) {
      INFO_INPUT(arg_info) = NULL; /* someone might have set it non-null. */
      BRANCHLIST_NEXT(arg_node) = TRAVdo(BRANCHLIST_NEXT(arg_node), arg_info);
    }

  }
  else {
    /* not init, do best match */

    DBUG_ASSERT(BRANCHLIST_TYPED(arg_node),
        "Encountered an untyped parallel branch. Syntax tree may be tampered");

    int bms = INFO_BRANCHMS(arg_info);
    TYPvrectype *attracts = TGVdoGetVRec(BRANCHLIST_ATTRACTS(arg_node));
    int ms = TYPbestMatchScore(input, attracts);
    char *in = NULL;
    if (TSDOPRINT) {
      in = TYPprintNrectype(input);
    }

    if (INFO_BRANCHGO(arg_info)) {
      /* real action */
      if (ms == bms) {
        if (TSDOPRINT) {
          TSPRINT(isdet ? "Routing %s to '||' branch at %s:%d.%d" :
              "Routing %s to '|' branch at %s:%d.%d", in,
              NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
        }
        BRANCHLIST_BRANCH(arg_node) =
            TRAVdo(BRANCHLIST_BRANCH(arg_node), arg_info);
        CTIabortOnError();
        /* restore info for next branch */
        INFO_INPUT(arg_info) = input;
        INFO_BRANCHMS(arg_info) = bms;
        INFO_BRANCHGO(arg_info) = TRUE;
      }
    }
    else {
      /* test only */
      if (TSDOPRINT) {
        char *at = TYPprintVrectype(attracts);
        TSPRINT("  Branch at %s:%d.%d attracts %s,", NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node), at);
        MEMfree(at);
        if (ms == -1) {
          TSPRINT("   not matching input %s.", in);
        }
        else if (ms > bms) {
          TSPRINT((bms == -1 ?
              "   matching input %s." : "   matching input %s better."), in);
        }
        else if (ms == bms) {
          TSPRINT("   matching input %s equally well.", in);
        }
        else {
          TSPRINT("   not matching input %s as well.", in);
        }
      }
      if (ms > bms) {
        INFO_BRANCHMS(arg_info) = ms;
      }
    }
    TYPfreeVrectype(attracts);

    if (TSDOPRINT) {
      MEMfree(in);
    }

    /* next branch please */
    if (BRANCHLIST_NEXT(arg_node) != NULL) {
      TYPvrectype *out = INFO_OUTPUTS(arg_info);
      INFO_OUTPUTS(arg_info) = NULL;
      INFO_ISDET(arg_info) = isdet; /* prevents overwriting */
      BRANCHLIST_NEXT(arg_node) = TRAVdo(BRANCHLIST_NEXT(arg_node), arg_info);
      INFO_OUTPUTS(arg_info) = TYPcombineVrectype(out, INFO_OUTPUTS(arg_info));
    }

  }

  DBUG_RETURN(arg_node);
}

/* traversed in Star by netref */
node *TCstar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCstar");

  TYPvrectype *seen = TYPnewVrectype(); /* seen outputs */
  TYPvrectype *mid = NULL; /* intermediate outputs */
  TYPvrectype *out = NULL; /* final outputs */
  TYPvrectype *uterms = TGVdoGetVRecUC(STAR_TERM(arg_node), TRUE, FALSE);
                                /* termination patterns with no conditions */
  TYPvrectype *cterms = TGVdoGetVRecUC(STAR_TERM(arg_node), FALSE, TRUE);
                                   /* termination patterns with conditions */

  TYPnrectype *input = INFO_INPUT(arg_info);
  bool reprint = FALSE;
  char *origIn = NULL;

  if (input == NULL) { /* do init */
    /* note: in this implementation, the "init" event does not just mean before
     * any record enters the boundary network; in the case of stars, the init
     * event also covers the case when a new replica is created and initted,
     * because a record is due to enter it. But in the real runtime, there may
     * have been some direct-through records before a record actually triggers
     * the creation of a replica and then the initialisation of the init boxes.
     * However typewise the order of the record types are not important and in
     * here we're grouping all init events together for type-check. */
    if (TSDOPRINT) {
      origIn = STRcpy("init signal");
      TSPRINT(STAR_ISDETERM(arg_node) ?
          " into '**' at %s:%d.%d" : " into '*' at %s:%d.%d",
          NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    STAR_LEFT(arg_node) = TRAVdo(STAR_LEFT(arg_node), arg_info);
    CTIabortOnError();
    mid = INFO_OUTPUTS(arg_info);
    INFO_OUTPUTS(arg_info) = NULL;
    reprint = TRUE;
  }
  else {
    if (TSDOPRINT) {
      origIn = TYPprintNrectype(input);
      TSPRINT(STAR_ISDETERM(arg_node) ?
          "  enters %s in '**' at %s:%d.%d." : "  enters %s in '*' at %s:%d.%d.",
          origIn, NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    mid = TYPnewVrectype();
    TYPfeedVrectype(mid, TYPcopyNrectype(input));
  }

  /* first input or intermediate results */
  while ((input = TYPpopVrectype(mid)) != NULL) {
    char *in = NULL;
    if (TSDOPRINT) {
      in = TYPprintNrectype(input);
      if (reprint) { /* been traversing elsewhere */
        TSPRINT(STAR_ISDETERM(arg_node) ?
            "At '**' at %s:%d.%d," : "At '*' at %s:%d.%d,",
            NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
        reprint = FALSE;
      }
    }
    if (TYPisCapturedBy(uterms, input)) {
      if (TSDOPRINT) {
        TSPRINT("  %s matches an unconditional termination pattern "
            "and is output.", in);
      }
      TYPvrectype *container = TYPnewVrectype();
      TYPfeedVrectype(container, input);
      out = TYPcombineVrectype(out, container);
      input = NULL;
    }
    else if (TYPisCapturedBy(cterms, input)) {
      if (TSDOPRINT) {
        TSPRINT("  %s matches a conditional termination pattern "
            "and is both output and entered.", in);
      }
      TYPvrectype *container = TYPnewVrectype();
      TYPfeedVrectype(container, TYPcopyNrectype(input));
      out = TYPcombineVrectype(out, container);
    }
    else {
      if (TSDOPRINT) {
        TSPRINT("  %s does not match any termination pattern.", in);
      }
    }
    if (input != NULL) { /* asked by the if-block above to enter iteration */
      if (TYPcontainedIn(seen, input)) {
        if (TSDOPRINT) {
          TSPRINT("  %s is seen before, not retrying.", in);
        }
        TYPfreeNrectype(input);
      }
      else {
        TYPfeedVrectype(seen, input);
        if (TSDOPRINT) {
          TSPRINT("Sending %s into one iteration", in);
        }
        INFO_INPUT(arg_info) = input;
        STAR_LEFT(arg_node) = TRAVdo(STAR_LEFT(arg_node), arg_info);
        CTIabortOnError();
        mid = TYPcombineVrectype(mid, INFO_OUTPUTS(arg_info));
        INFO_OUTPUTS(arg_info) = NULL;
        reprint = TRUE;
      }
    }
    if (TSDOPRINT) {
      MEMfree(in);
    }
  }

  if (TSDOPRINT) {
    TSPRINT(STAR_ISDETERM(arg_node) ?
        "In conclusion, %s through '**' at %s:%d.%d" :
        "In conclusion, %s through '*' at %s:%d.%d", origIn,
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
  }

  PRINT_AND_OUTPUT(out);

  if (TSDOPRINT) {
     MEMfree(origIn);
  }

  TYPfreeVrectype(seen);
  TYPfreeVrectype(mid);
  TYPfreeVrectype(uterms);
  TYPfreeVrectype(cterms);

  DBUG_RETURN(arg_node);
}





/* traversed in Split by netdef */
node *TCsplit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCsplit");

  if (TSDOPRINT) {
    TSPRINT(SPLIT_ISDETERM(arg_node) ?
        " into '!!' at %s:%d.%d" : " into '!' at %s:%d.%d",
        NODE_FILE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
  }

  if (INFO_INPUT(arg_info) != NULL) {
    /* go into Range node and check if all required tags are present */
    SPLIT_RANGE(arg_node) = TRAVdo(SPLIT_RANGE(arg_node), arg_info);
    CTIabortOnError();
  }

  SPLIT_LEFT(arg_node) = TRAVdo(SPLIT_LEFT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/* traversed in Range by Split */
node *TCrange(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCrange");

  TYPnrectype *input = INFO_INPUT(arg_info);
  if (RANGE_STAGSTART(arg_node) != NULL &&
      !TYPnrectypeHas(input, RANGE_STAGSTART(arg_node))) {
    char *in = TYPprintNrectype(input);
    CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		 arg_node, "Input %s does not carry tag <%s> required for "
		 "parallel replication", in,
		 STAGS_NAME(STAGREF_STAG(RANGE_STAGSTART(arg_node))));
    MEMfree(in);
  }
  if (RANGE_BTAGSTART(arg_node) != NULL &&
      !TYPnrectypeHas(input, RANGE_BTAGSTART(arg_node))) {
    char *in = TYPprintNrectype(input);
    CTIerrorNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		 arg_node, "Input %s does not carry binding tag <#%s> "
		 "required for parallel replication", in,
		 BTAGS_NAME(BTAGREF_BTAG(RANGE_BTAGSTART(arg_node))));
    MEMfree(in);
  }

  DBUG_RETURN(arg_node);
}

/* main for this file */
node *TCdoTypeCheck(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("TCdoTypeCheck");

  DBUG_ASSERT((syntax_tree != NULL),
      "TCdoTypeCheck called with empty syntaxtree");
  DBUG_ASSERT(NODE_TYPE(syntax_tree) == N_netdef &&
      !NETDEF_EXTERNAL(syntax_tree) && NETDEF_SIGNED(syntax_tree),
      "TCdoTypeCheck expects internal signed netdef node");

  inf = infoMake();

  TRAVpush(TR_tc);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
