/*****************************************************************************
 *
 * $Id: typing.h 3378 2012-03-11 11:36:57Z vnn $
 *
 * Type Inference -- Typing
 * 
 * Max Troy (Haoxuan Cai), Imperial College London,
 * 2007.10.31
 *
 *****************************************************************************/

#ifndef _SNETC_TYPING_H_
#define _SNETC_TYPING_H_

#include "types.h"
#include "globals.h"

/* nonvariant record type, variant record type and normalised type sig */
typedef struct TYP_NRECTYPE TYPnrectype;
typedef struct TYP_VRECTYPE TYPvrectype;
typedef struct TYP_NTYPESIG TYPntypesig;


/************* For N(onvariant) Rec(ord) Type ******************/

/* Creates an empty nrectype object */
TYPnrectype *TYPnewNrectype(void);

/* Collects a FieldRef, STagRef, or BTagRef into an nrectype object. */
void TYPfeedNrectype(TYPnrectype *nrt, node *entryref, lblqual qualifier);

/* Checks if a FieldRef, STagRef or BTagRef is contained in an nrectype */
bool TYPnrectypeHas(TYPnrectype *nrt, node *entryref);

/* Clones an nrectype object */
TYPnrectype *TYPcopyNrectype(TYPnrectype *nrt);

/* Frees an nrectype object */
TYPnrectype *TYPfreeNrectype(TYPnrectype *nrt);

/* Checks subtype relationship for nonvariant record types */
bool TYPisSubtypeOfN(TYPnrectype *super, TYPnrectype *sub);

/*************** For V(ariant) Rec(ord) Type *******************/

/* Creates an empty vrectype object */
TYPvrectype *TYPnewVrectype(void);

/* Adds an nrectype object to a vrectype object. The nrectype object will
 * then be owned by the vrectype object. If a duplicate exists in the
 * vrectype object, the provided nrectype object will be freed immediately. */
void TYPfeedVrectype(TYPvrectype *vrt, TYPnrectype *nrt);

/* Checks if an nrectype is contained in a vrectype */
bool TYPcontainedIn(TYPvrectype *vrt, TYPnrectype *nrt);

/* Clones a vrectype object */
TYPvrectype *TYPcopyVrectype(TYPvrectype *vrt);

/* Frees a vrectype object as well as all contained nrectype objects */
TYPvrectype *TYPfreeVrectype(TYPvrectype *vrt);

/* Pops an nrectype object from a vrectype object. The returned nrectype
 * will no longer be owned by the vrectype. NULL if vrt is NULL or empty. */
TYPnrectype *TYPpopVrectype(TYPvrectype *vrt);

/* Checks if a vrectype is empty */
bool TYPisEmptyVrectype(TYPvrectype *vrt);

/* Combines two vrectypes into one. Returns the combined type. Arguments will
 * be owned. Returns NULL if both arguments are NULL. */
TYPvrectype *TYPcombineVrectype(TYPvrectype *vrta, TYPvrectype *vrtb);

/* Checks if an nrectype is captured by a vrectype (is subtype of one of
 * the vrectype variants) */
bool TYPisCapturedBy(TYPvrectype *super, TYPnrectype *nrt);

/* Checks subtype relationship for variant record types */
bool TYPisSubtypeOfV(TYPvrectype *super, TYPvrectype *sub);

/* Gets the match score */
int TYPbestMatchScore(TYPnrectype *nrt, TYPvrectype *vrt);

/*************** For N(ormalised) Type Sig(nature) *************/

/* Creates an empty ntypesig object */
TYPntypesig *TYPnewNtypesig(void);

/* Adds a normalised type map (nrectype lhs, vrectype rhs) into an ntypesig.
 * The objects will be copied into the ntypesig object, and maybe modified.
 * Returns FALSE if the inputsOnly param conflicts with existing typemaps. */
bool TYPfeedNtypesig(TYPntypesig *nts, TYPnrectype *lhs, TYPvrectype *rhs,
    bool inputsOnly);

/* Adds a type map (vrectype lhs and rhs) into an ntypesig. The lhs will be
 * exploded into different normalised type maps and the rhs will be copied
 * for all so produced type maps. Afterwards the vrectype objects will be
 * freed. Returns FALSE if there has been any conflict about inputOnly. */
bool TYPjoinNtypesig(TYPntypesig *nts, TYPvrectype *lhs, TYPvrectype *rhs,
    bool inputsOnly);

/* Gets the inputs from an ntypesig. */
TYPvrectype *TYPntypesigIns(TYPntypesig *nts);

/* Merges ntsb into ntsa. Returns TRUE if successful and FALSE if there was
 * any input-only conflict. */
bool TYPntypesigMergeInto(TYPntypesig *ntsa, TYPntypesig *ntsb);

/* Checks if an ntypesig is empty (type error) */
bool TYPisEmptyNtypesig(TYPntypesig *nts);

/*check if all types of nts are initializer */
bool TYPisInitializerNtypesig(TYPntypesig *nts);

/* Checks if an ntypesig has input-only typemaps */
bool TYPisIncompleteNtypesig(TYPntypesig *nts);

/* Checks if an ntypesig has an init typemap (NULL-input) */
bool TYPhasInitMap(TYPntypesig *nts);

/* Gets the init results from an ntypesig that has an init typemap */
TYPvrectype *TYPgetInitResults(TYPntypesig *nts);

/* Simulates inputting the ntypesig into a network having the provided ntypesig
 * and returns a vrectype collecting all output possibilities. NULL is returned
 * if the ntypesig does not accept the input. */
TYPvrectype *TYPgetOutputs(TYPntypesig *nts, TYPnrectype *in);

void TYPsetAllPass(TYPnrectype *nrt);

/* Checks the provided output types against the declaration in the ntypesig
 * for correctness. For an input-only typemap in the ntypesig, the check always
 * succeeds, and the provided outputs are copied into the typemap. */
bool TYPcheckOutputs(TYPntypesig *sig, TYPnrectype *in, TYPvrectype *outs);

/* Clones an ntypesig object */
TYPntypesig *TYPcopyNtypesig(TYPntypesig *nts);

/* Frees an ntypesig object */
TYPntypesig *TYPfreeNtypesig(TYPntypesig *nts);

/************************* Type Inference **********************/

/* For type inferences of boxes and filters,
 * just walk the AST and collect into an ntypesig. */

/* dotdot helper: tells if a re-route is required for the left operand.
 * Returns NULL if not required, and a vrectype of accepted inputs (Ins of
 * original sig minus those causing troubles) if so. */
TYPvrectype *TYPddhCheckRerouteNecessity(TYPntypesig *ls, TYPntypesig *rs);

/* Infers the type signature for a synchrocell. Arguments will be freed. */
TYPntypesig *TYPinferSync(TYPnrectype *main, TYPvrectype *aux);

/* Infers the type signature for a serial composition. */
TYPntypesig *TYPinferDotDot(node *arg_node, TYPntypesig *ls, TYPntypesig *rs);

/* Adapts the routing information to the routing restriction. Returns if any
 * changes have made. */
bool TYPadaptRoutingInfo(TYPvrectype *ri, TYPvrectype *restr, bool exact);

/* Infers the type signature for a parallel branch. For default routing info
 * (no filtering) just use nts and do not call this function. */
TYPntypesig *TYPinferBarBranch(TYPntypesig *nts, TYPvrectype *ri);

#define TYPSTAR_ITERLIMIT 50

/* star helper: tells if a re-route is required for the operand.
 * Returns NULL if not required, and a vrectype of accepted inputs (Ins of
 * original sig minus those causing troubles) if so. */
TYPvrectype *TYPshCheckRerouteNecessity(TYPntypesig *sig, TYPvrectype *utv,
    TYPvrectype *ctv);

/* Infers the type signature for a serial replication. Arguments utv and
 * ctv are the termination patterns without and with conditions, collected 
 * in vrectype objects. */
TYPntypesig *TYPinferStar(node *arg_node, TYPntypesig *sig, TYPvrectype *utv,
                          TYPvrectype *ctv);

/* Infers the type signature for a parallel replication. Argument tag is
 * an STagRef or BTagRef node. */
TYPntypesig *TYPinferEx(TYPntypesig *nts, node *tag);

/************************** Finishing **************************/

/* Creates a Types node from a vrectype object */
node *TYPcreateTypesNode(TYPvrectype *obj);

/* Creates a TypeSigns node from an ntypesig object */
node *TYPcreateTypeSigns(TYPntypesig *nts);

/* Constructs a string of an nrectype */
char *TYPprintNrectype(TYPnrectype *nrt);

/* Constructs a string of a vrectype */
char *TYPprintVrectype(TYPvrectype *vrt);

/* Constructs a string of an ntypesig pretty-printed */
char *TYPprintNtypesig(TYPntypesig *nts);

/************************** Debug print *************************/

/* prints debug info for type system tasks. */
#define TSPRINT(...) CTInote(__VA_ARGS__)

/* because printing involves making lots of strings, if the user chooses not to
 * print anything, it's better not make those strings at all. the following 
 * macro can be used as the conditional in an if statement */
#define TSDOPRINT global.verbose_level >= 3

/************************* Rerouting Switch *****************************/

/* DOREROUTE: defined = do reroute */

#endif /*TYPING_H_*/
