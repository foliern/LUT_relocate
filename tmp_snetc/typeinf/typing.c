/*******************************************************************************
 *
 * $Id: typing.c 3378 2012-03-11 11:36:57Z vnn $
 *
 * Type Inference -- Typing Library
 *
 * Max Troy (Haoxuan Cai), Imperial College London
 *
 *******************************************************************************/

#include "typing.h"
#include "dbug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "set.h"
#include "node_basic.h"
#include "str.h"
#include "string.h"
#include "lookup_table.h"

/**************** structs ******************/

struct TYP_NRECTYPE {
  set *btags;
  set *fields;
  set *passes;
  set *discards;
};

struct TYP_VRECTYPE {
  set *nrts;
};

typedef struct {
  TYPnrectype *lhs;
  TYPvrectype *rhs;
  bool inputsOnly; /* "Types -> ..."; rhs will be empty */
} TYPntypemap;

struct TYP_NTYPESIG {
  set *maps;
};

/******************** nrectype *************************/

/* recentry comparer (for set operations)
 * (param types are one of [Fields, STags, BTags]) */
static int reCompare(void *entryrefA, void *entryrefB)
{
  DBUG_ENTER("reCompare");

  DBUG_RETURN(entryrefA - entryrefB);
}

/* Creates an empty nrectype object */
TYPnrectype *TYPnewNrectype(void)
{
  TYPnrectype *new;
  DBUG_ENTER("TYPnewNrectype");

  new = MEMmalloc(sizeof(TYPnrectype));
  new->btags = SETnewSet(&reCompare);
  new->fields = SETnewSet(&reCompare);
  new->passes = SETnewSet(&reCompare);
  new->discards = SETnewSet(&reCompare);

  DBUG_RETURN(new);
}

/* Clones an nrectype object */
TYPnrectype *TYPcopyNrectype(TYPnrectype *nrt)
{
  DBUG_ENTER("TYPcopyNrectype");

  TYPnrectype *new = NULL;
  if (nrt != NULL) {
    new = MEMmalloc(sizeof(TYPnrectype));
    new->btags = SETnewSetFrom(nrt->btags);
    new->fields = SETnewSetFrom(nrt->fields);
    new->passes = SETnewSetFrom(nrt->passes);
    new->discards = SETnewSetFrom(nrt->discards);
  }

  DBUG_RETURN(new);
}

/* Gets a copy of the nrectype object modulo passes and discards */
static TYPnrectype *cleanCopyNrectype(TYPnrectype *nrt)
{
  DBUG_ENTER("cleanCopyNrectype");

  TYPnrectype *new = NULL;
  if (nrt != NULL) {
    new = MEMmalloc(sizeof(TYPnrectype));
    new->btags = SETnewSetFrom(nrt->btags);
    new->fields = SETnewSetFrom(nrt->fields);
    new->passes = SETnewSet(&reCompare);
    new->discards = SETnewSet(&reCompare);
  }

  DBUG_RETURN(new);
}

/* Frees an nrectype object */
TYPnrectype *TYPfreeNrectype(TYPnrectype *nrt)
{
  DBUG_ENTER("TYPfreeNrectype");

  if (nrt != NULL) {
    SETfreeSet(nrt->btags);
    SETfreeSet(nrt->fields);
    SETfreeSet(nrt->passes);
    SETfreeSet(nrt->discards);
    MEMfree(nrt);
  }

  DBUG_RETURN(NULL);
}

/* Void-pointer version of the function above to comply with set function */
static void *freeNrectype(void *nrt) { return TYPfreeNrectype(nrt); }

/* Collects a FieldRef, STagRef or BTagRef into an nrectype */
void TYPfeedNrectype(TYPnrectype *nrt, node *entryref, lblqual qualifier)
{
  DBUG_ENTER("TYPfeedNrectype");

  DBUG_ASSERT(nrt != NULL, "TYPfeedNrectype called without an nrectype object");
  DBUG_ASSERT(entryref != NULL && (NODE_TYPE(entryref) == N_fieldref
      || NODE_TYPE(entryref) == N_stagref || NODE_TYPE(entryref) == N_btagref),
    "TYPfeedNrectype called with an invalid node");

  node *ref = NULL;
  switch (NODE_TYPE(entryref)) {
  case N_fieldref:
    ref = FIELDREF_FIELD(entryref);
    /* no break: use N_stagref's main part */
  case N_stagref:
    if (ref == NULL) { /* takes care of flowing from case N_fieldref */
      ref = STAGREF_STAG(entryref);
    }
    switch (qualifier) {
    case LQUA_pass:
      nrt->passes = SETaddElem(nrt->passes, ref);
      /* no break: go on to add to field */
    case LQUA_none:
      nrt->fields = SETaddElem(nrt->fields, ref);
      break;
    default: /* discard */
      nrt->discards = SETaddElem(nrt->discards, ref);
      break;
    }
    break;
  default: /* btag */
    if (qualifier == LQUA_none) {
      nrt->btags = SETaddElem(nrt->btags, BTAGREF_BTAG(entryref));
    }
    else {
      DBUG_ASSERT(FALSE, "BTag with a qualifier found");
    }
    break;
  }

  DBUG_VOID_RETURN;
}

/* Checks if a FieldRef, STagRef or BTagRef is contained in an nrectype */
bool TYPnrectypeHas(TYPnrectype *nrt, node *entryref)
{
  DBUG_ENTER("TYPnrectypeHas");

  DBUG_ASSERT(entryref != NULL && (NODE_TYPE(entryref) == N_fieldref
      || NODE_TYPE(entryref) == N_stagref || NODE_TYPE(entryref) == N_btagref),
    "TYPnrectypeHas called with an invalid node");

  bool out = FALSE;

  if (nrt != NULL) {
    switch (NODE_TYPE(entryref)) {
    case N_fieldref:
      out = SEThasElem(nrt->fields, FIELDREF_FIELD(entryref));
      break;
    case N_stagref:
      out = SEThasElem(nrt->fields, STAGREF_STAG(entryref));
      break;
    default: /* btag */
      out = SEThasElem(nrt->btags, BTAGREF_BTAG(entryref));
      break;
    }
  }

  DBUG_RETURN(out);
}

/* nrectype comparer */
static int nrtComparer(TYPnrectype *a, TYPnrectype *b, bool doPasses)
{
  DBUG_ENTER("nrtComparer");

  if (a == b) { /* shortcut */
    DBUG_RETURN(0);
  }
  else if (a == NULL || b == NULL) {
    DBUG_RETURN( (long)a - (long)b );
  }

  int cr = SETcompare(a->btags, b->btags);
  if (cr == 0) {
    cr = SETcompare(a->fields, b->fields);
    if (cr == 0) {
      cr = SETcompare(a->discards, b->discards);
      if (cr == 0 && doPasses) {
        cr = SETcompare(a->passes, b->passes);
      }
    }
  }

  DBUG_RETURN(cr);
}

/* nrectype comparer: core version, ignores passes */
static int nrtCompare(void *rta, void *rtb)
{
  return nrtComparer(rta, rtb, FALSE);
}

/* nrectype comparer: full version
static int nrtCompareFull(void *rta, void *rtb)
{
  return nrtComparer(rta, rtb, TRUE);
}
 */
/********************* NRecType Relationships ******************/

/* Checks if two nonvariant record types have the same BT set */
static inline bool nrtBTEquals(TYPnrectype *a, TYPnrectype *b)
{
  DBUG_ENTER("nrtBTEquals");

  bool out = SETequals(a->btags, b->btags);

  DBUG_RETURN(out);
}

/* Checks subtype relationship for nonvariant record types */
bool TYPisSubtypeOfN(TYPnrectype *super, TYPnrectype *sub)
{
  DBUG_ENTER("TYPisSubtypeOfN");

  bool out = nrtBTEquals(sub, super)
    && SETisSubsetOf(sub->fields, super->fields)
    && SETisSubsetOf(sub->passes, super->passes);

  DBUG_RETURN(out);
}

/********************** vrectype ***********************/

/* Creates an empty vrectype object */
TYPvrectype *TYPnewVrectype(void)
{
  TYPvrectype *new;
  DBUG_ENTER("TYPnewVrectype");

  new = MEMmalloc(sizeof(TYPvrectype));
  new->nrts = SETnewSet(&nrtCompare);

  DBUG_RETURN(new);
}

/* Checks if an nrectype is contained in a vrectype */
bool TYPcontainedIn(TYPvrectype *vrt, TYPnrectype *nrt)
{
  DBUG_ENTER("TYPcontainedIn");

  bool out = FALSE;
  if (vrt != NULL && nrt != NULL) {
    out = SEThasElem(vrt->nrts, nrt);
  }

  DBUG_RETURN(out);
}

/* Adds an nrectype object to a vrectype object */
void TYPfeedVrectype(TYPvrectype *vrt, TYPnrectype *nrt)
{
  DBUG_ENTER("TYPfeedVrectype");

  DBUG_ASSERT(vrt != NULL && nrt != NULL,
    "TYPfeedVrectype called with NULL argument(s)");

  TYPnrectype *existing = SETfindElem(vrt->nrts, nrt);
  if (existing == NULL) {
    vrt->nrts = SETaddElem(vrt->nrts, nrt);
  }
  else {
    /* an existing nrectype in the vrectype has the same core part,
     * do merge passes */
    existing->passes = SETintersectWith(existing->passes, nrt->passes);
    TYPfreeNrectype(nrt); // simulate "owned"
  }

  DBUG_VOID_RETURN;
}

/* Clones a vrectype object */
TYPvrectype *TYPcopyVrectype(TYPvrectype *vrt)
{
  DBUG_ENTER("TYPcopyVrectype");

  TYPvrectype *new = NULL;
  if (vrt != NULL) {
    new = TYPnewVrectype();
    int i, size = SETsize(vrt->nrts);
    for (i = 0; i < size; i++) {
      new->nrts = SETaddElem(new->nrts, TYPcopyNrectype(SETelem(vrt->nrts, i)));
    }
  }

  DBUG_RETURN(new);
}

/* Frees a vrectype object as well as all contained nrectype objects */
TYPvrectype *TYPfreeVrectype(TYPvrectype *vrt)
{
  DBUG_ENTER("TYPfreeVrectype");

  if (vrt != NULL) {
    SETfreeSetWith(vrt->nrts, &freeNrectype);
    MEMfree(vrt);
  }

  DBUG_RETURN(NULL);
}

/* Combines two vrectypes into one. Returns the combined type. Arguments will
 * be owned */
TYPvrectype *TYPcombineVrectype(TYPvrectype *vrta, TYPvrectype *vrtb)
{
  DBUG_ENTER("TYPcombineVrectype");

  if (vrta == NULL) {
    if (vrtb != NULL) {
      vrta = vrtb;
      vrtb = NULL;
    }
  }
  TYPnrectype *n;
  while ((n = TYPpopVrectype(vrtb)) != NULL) {
    TYPfeedVrectype(vrta, n);
  }
  TYPfreeVrectype(vrtb);

  DBUG_RETURN(vrta);
}

/* Pops an nrectype object from a vrectype object. The returned nrectype
 * will no longer be owned by the vrectype. NULL if vrt is NULL or empty. */
TYPnrectype *TYPpopVrectype(TYPvrectype *vrt)
{
  DBUG_ENTER("TYPpopVrectype");

  TYPnrectype *n = NULL;
  if (vrt != NULL && SETsize(vrt->nrts) != 0) {
    n = SETremoveElemAt(vrt->nrts, 0);
  }

  DBUG_RETURN(n);
}

/* Checks if a vrectype is empty */
bool TYPisEmptyVrectype(TYPvrectype *vrt)
{
  DBUG_ENTER("TYPisEmptyVrectype");

  bool out = vrt == NULL || SETsize(vrt->nrts) == 0;

  DBUG_RETURN(out);
}

/********************** VRecType Relationships *********************/

/* Checks if an nrectype is captured by a vrectype (is subtype of one of
 * the vrectype variants) */
bool TYPisCapturedBy(TYPvrectype *super, TYPnrectype *nrt)
{
  DBUG_ENTER("TYPisCapturedBy");

  bool out = FALSE;
  if (super != NULL) {
    int i, size = SETsize(super->nrts);
    for (i = 0; i < size && !out; i++) {
      out = TYPisSubtypeOfN(SETelem(super->nrts, i), nrt);
    }
  }
  DBUG_RETURN(out);
}

/* Checks subtype relationship for variant record types */
bool TYPisSubtypeOfV(TYPvrectype *super, TYPvrectype *sub)
{
  DBUG_ENTER("TYPisSubtypeOfV");

  bool out = TRUE;
  if (sub != NULL) {
    int i, size = SETsize(sub->nrts);
    for (i = 0; i < size && out; i++) {
      out = out && TYPisCapturedBy(super, SETelem(sub->nrts, i));
    }
  }
  DBUG_RETURN(TRUE);
}

/* Gets the match score */
int TYPbestMatchScore(TYPnrectype *nrt, TYPvrectype *vrt)
{
  DBUG_ENTER("TYPbestMatchScore");

  int out = -1, i = SETsize(vrt->nrts) - 1;
  for (; i >= 0; i--) {
    TYPnrectype *var = SETelem(vrt->nrts, i);
    if (TYPisSubtypeOfN(var, nrt)) {
      if (out < SETsize(var->fields)) {
        out = SETsize(var->fields);
      }
    }
  }

  DBUG_RETURN(out);
}

/********************** ntypemap/sig **********************/

/* Creates an ntypemap */
static TYPntypemap *TYPnewNtypemap(TYPnrectype *lhs, TYPvrectype *rhs, bool io)
{
  DBUG_ENTER("TYPnewNtypemap");

  TYPntypemap *new = MEMmalloc(sizeof(TYPntypemap));
  new->lhs = lhs;
  new->rhs = rhs;
  new->inputsOnly = io;

  DBUG_RETURN(new);
}

/* Frees an ntypemap */
static void *freeNtypemap(void *ntm)
{
  DBUG_ENTER("TYPfreeNtypemap");

  TYPntypemap *m = ntm;
  if (m != NULL) {
    TYPfreeNrectype(m->lhs);
    TYPfreeVrectype(m->rhs);
    MEMfree(m);
  }

  DBUG_RETURN(NULL);
}

/* ntypemap comparer */
static int ntmCompare(void *tma, void *tmb)
{
  DBUG_ENTER("ntmCompare");

  TYPntypemap *a = tma, *b = tmb;

  int rc = nrtCompare(a->lhs, b->lhs);

  DBUG_RETURN(rc);
}

/* Creates an empty ntypesig object */
TYPntypesig *TYPnewNtypesig(void)
{
  TYPntypesig *new;
  DBUG_ENTER("TYPnewNtypesig");

  new = MEMmalloc(sizeof(TYPntypesig));
  new->maps = SETnewSet(&ntmCompare);

  DBUG_RETURN(new);
}

/* Performs completion and fixation works on RHS nrectype objects before
 * they're added to an ntypesig */
static void completeNrectype(TYPnrectype *lhs, TYPnrectype *rhs)
{
  DBUG_ENTER("completeNrectype");

  if (lhs == NULL) { /* initter, rhs should not have any passes or discards */
    rhs->passes = SETclearSet(rhs->passes);
    rhs->discards = SETclearSet(rhs->discards);
  }
  else {
    /* rhs passes = (rhs passes n lhs fields) U
     *                 (lhs passes \ rhs fields \ rhs discards) */
    set *lp_rd = SETsubtract(lhs->passes, rhs->discards);
    set *lp_rd_rf = SETsubtract(lp_rd, rhs->fields);
    rhs->passes = SETintersectWith(rhs->passes, lhs->fields);
    rhs->passes = SETunionWith(rhs->passes, lp_rd_rf);
    SETfreeSet(lp_rd_rf);

    /* rhs fields = rhs fields U (lhs passes \ rhs discards) */
    rhs->fields = SETunionWith(rhs->fields, lp_rd);
    SETfreeSet(lp_rd);

    /* rhs discards = rhs discards U
     *                  (lhs fields \ lhs passes \ rhs fields)
     * but we have updated rhs fields so they will include lhs passes:
     *              = rhs discards U (lhs fields \ rhs fields') */
    set *lf_rf = SETsubtract(lhs->fields, rhs->fields);
    rhs->discards = SETunionWith(rhs->discards, lf_rf);
    SETfreeSet(lf_rf);
  }

  DBUG_VOID_RETURN;
}

/* Adds a normalised type map (nrectype lhs, vrectype rhs) into an ntypesig.
 * The objects will be owned by the ntypesig object, and maybe modified.
 * Returns FALSE if the inputsOnly param conflicts with existing typemaps */
bool TYPfeedNtypesig(TYPntypesig *nts, TYPnrectype *lhs, TYPvrectype *rhs,
    bool inputsOnly)
{
  int i;
  TYPntypemap *m, *finder;
  bool ret = TRUE;
  DBUG_ENTER("TYPfeedNtypesig");

  DBUG_ASSERT(nts != NULL,
    "TYPfeedNtypesig called with NULL nts argument");

  finder = TYPnewNtypemap(cleanCopyNrectype(lhs), TYPnewVrectype(), inputsOnly);
  m = SETfindElem(nts->maps, finder);
  if (m == NULL) { /* LHS not found, simply add finder */
    nts->maps = SETaddElem(nts->maps, m = finder);
  }
  else { /* LHS found, finder useless now */
    freeNtypemap(finder);
  }

  if (inputsOnly) {
    if (!m->inputsOnly) {
      ret = FALSE;
    }
  }
  else {
    if (m->inputsOnly) {
      ret = FALSE;
    }
    else {
      /* now add RHS variants into the ntypemap */
      if (rhs != NULL) {
        for (i = SETsize(rhs->nrts) - 1; i >= 0; i--) {
          TYPnrectype *r = SETremoveElemAt(rhs->nrts, i);
          completeNrectype(lhs, r);
          TYPfeedVrectype(m->rhs, r);
        }
      }
    }
  }

  TYPfreeNrectype(lhs);
  TYPfreeVrectype(rhs);
  DBUG_RETURN(ret);
}

/* Adds a type map (vrectype lhs and rhs) into an ntypesig. The lhs will be
 * exploded into different normalised type maps and the rhs will be copied
 * for all so produced type maps. Afterwards the vrectype objects will be
 * freed. Returns FALSE if there has been any conflict about inputOnly. */
bool TYPjoinNtypesig(TYPntypesig *nts, TYPvrectype *lhs, TYPvrectype *rhs,
    bool inputsOnly)
{
  int i, size;
  bool ret = TRUE;
  DBUG_ENTER("TYPjoinNtypesig");

  DBUG_ASSERT(nts != NULL,
    "TYPjoinNtypesig called with NULL nts argument");

  if (lhs == NULL) {
    lhs = TYPnewVrectype();
  }
  if (rhs == NULL) {
    rhs = TYPnewVrectype();
  }

  if (SETsize(lhs->nrts) == 0) { /* initter, no lhs */
    ret = TYPfeedNtypesig(nts, NULL, rhs, inputsOnly);
  }
  else {
    for (i = 0, size = SETsize(lhs->nrts); i < size && ret; i++) {
      TYPnrectype *l = TYPcopyNrectype(SETelem(lhs->nrts, i));
      ret = ret && TYPfeedNtypesig(nts, l, TYPcopyVrectype(rhs), inputsOnly);
    }
    TYPfreeVrectype(rhs);
  }

  TYPfreeVrectype(lhs);
  DBUG_RETURN(ret);
}

/* Clones an ntypesig object */
TYPntypesig *TYPcopyNtypesig(TYPntypesig *nts)
{
  DBUG_ENTER("TYPcopyNtypesig");

  TYPntypesig *out = TYPnewNtypesig();
  int i, size = SETsize(nts->maps);
  for (i = 0; i < size; i++) {
    TYPntypemap *m = SETelem(nts->maps, i);
    m = TYPnewNtypemap(TYPcopyNrectype(m->lhs), TYPcopyVrectype(m->rhs),
        m->inputsOnly);
    out->maps = SETaddElem(out->maps, m); /* shortcut */
  }

  DBUG_RETURN(out);
}

/* Frees an ntypesig object */
TYPntypesig *TYPfreeNtypesig(TYPntypesig *nts)
{
  DBUG_ENTER("TYPfreeNtypesig");

  if (nts != NULL) {
    SETfreeSetWith(nts->maps, &freeNtypemap);
    MEMfree(nts);
  }

  DBUG_RETURN(NULL);
}

/* Checks if an ntypesig is empty */
bool TYPisEmptyNtypesig(TYPntypesig *nts)
{
  DBUG_ENTER("TYPisEmptyNtypesig");

  bool out = nts == NULL || SETisEmpty(nts->maps);

  DBUG_RETURN(out);
}

/* Checks if an ntypesig has input-only typemaps */
bool TYPisIncompleteNtypesig(TYPntypesig *nts)
{
  DBUG_ENTER("TYPisIncompleteNtypesig");

  bool out = nts == NULL;

  if (!out) {
    int i = SETsize(nts->maps) - 1;
    for (; i >= 0 && !out; i--) {
      TYPntypemap *m = SETelem(nts->maps, i);
      out = m->inputsOnly;
    }
  }

  DBUG_RETURN(out);
}

/* check if initializer, all inputs are empty*/
bool TYPisInitializerNtypesig(TYPntypesig *nts)
{
  DBUG_ENTER("TYPisInitializerNtypesig");

  bool out = !(nts == NULL);

    int i = SETsize(nts->maps) - 1;
    for (; i >= 0 ; i--) {
      TYPntypemap *m = SETelem(nts->maps, i);
      out &= (m->lhs == NULL);
  }

  DBUG_RETURN(out);
}

/* helper: get init typemap */
static TYPntypemap *getInitMap(TYPntypesig *nts)
{
  DBUG_ENTER("getInitMap");

  TYPntypemap *m = NULL;
  if (nts != NULL) {
    TYPntypemap *finder = TYPnewNtypemap(NULL, NULL, FALSE);
    m = SETfindElem(nts->maps, finder);
    freeNtypemap(finder);
  }

  DBUG_RETURN(m);
}

/* Checks if an ntypesig has an init typemap (NULL-input) */
bool TYPhasInitMap(TYPntypesig *nts)
{
  DBUG_ENTER("TYPhasInitMap");

  bool out = getInitMap(nts) != NULL;

  DBUG_RETURN(out);
}

/* Gets the init results from an ntypesig that has an init typemap */
TYPvrectype *TYPgetInitResults(TYPntypesig *nts)
{
  DBUG_ENTER("TYPgetInitResults");

  TYPntypemap *m = getInitMap(nts);
  TYPvrectype *out = m == NULL ? NULL : TYPcopyVrectype(m->rhs);

  DBUG_RETURN(out);
}

/* helper: extracts the best match maps in an ntypesig */
static inline set *bestMatches(TYPntypesig *nts, TYPnrectype *nrt)
{
  DBUG_ENTER("bestMatches");

  int bm = -1, i, size = SETsize(nts->maps);
  set *bms = SETnewSet(NULL);
  for (i = 0; i < size; i++) {
    TYPntypemap *m = SETelem(nts->maps, i);
    if (m->lhs != NULL && TYPisSubtypeOfN(m->lhs, nrt)) {
      int ms = SETsize(m->lhs->fields);
      if (ms > bm) {
        bms = SETclearSet(bms);
        bm = ms;
        bms = SETaddElem(bms, m);
      }
      else if (ms == bm) {
        bms = SETaddElem(bms, m);
      }
    }
  }

  DBUG_RETURN(bms);
}

/* evaluates the outputs produced by feeding the provided input into the
 * provided sig. Inputs are expected to show correct pass-through and
 * discarded fields. Outputs contain pass-throughs and discards. NULL is 
 * returned if the sig does not accept the input. */
static TYPvrectype *evalOutputs(TYPntypesig *nts, TYPnrectype *in)
{
  DBUG_ENTER("evalOutputs");

  TYPvrectype *out = NULL;

  set *bms = bestMatches(nts, in);
  int i, size = SETsize(bms);
  if (size != 0) {
    out = TYPnewVrectype();
    for (i = 0; i < size; i++) {
      TYPntypemap *m = SETelem(bms, i);
      DBUG_ASSERT(!(m->inputsOnly),
          "evalOutputs encountered an incomplete typemap");
      if (m->rhs != NULL) {
        /* extra fields to be passed through */
        set *extra = SETsubtract(in->fields, m->lhs->fields);
        int j, jsize = SETsize(m->rhs->nrts);
        for (j = 0; j < jsize; j++) {
          /* one output variant */
          TYPnrectype *myout = SETelem(m->rhs->nrts, j);
          myout = TYPcopyNrectype(myout);
          /* what is discarded can't be passed through */
          set *myextra = SETsubtract(extra, myout->discards);
          /* add extra to new passes */
          myout->fields = SETunionWith(myout->fields, myextra);
          myout->passes = SETunionWith(myout->passes, myextra);
          /* what wasn't pass-through isn't pass-through */
          myout->passes = SETintersectWith(myout->passes, in->passes);
          /* add discards from input */
          myout->discards = SETunionWith(myout->discards, in->discards);
          myout->discards = SETsubtractFrom(myout->discards, myout->fields);
          SETfreeSet(myextra);
          TYPfeedVrectype(out, myout);
        }
        SETfreeSet(extra);
      }
    }
  }

  DBUG_RETURN(out);
}

/* Simulates inputting the nrectype into a network having the provided ntypesig
 * and returns a vrectype collecting all output possibilities. NULL is returned
 * if the ntypesig does not accept the input. Gets rid of discards. */
TYPvrectype *TYPgetOutputs(TYPntypesig *nts, TYPnrectype *in)
{
  DBUG_ENTER("TYPgetOutputs");

  TYPvrectype *out = evalOutputs(nts, in);

  if (out != NULL) {
    int i = SETsize(out->nrts) - 1;
    for (; i >= 0; i--) {
      TYPnrectype *n = SETelem(out->nrts, i);
      n->discards = SETclearSet(n->discards);
    }
  }

  DBUG_RETURN(out);
}

void TYPsetAllPass(TYPnrectype *nrt)
{
  DBUG_ENTER("TYPsetAllPass");

  if (nrt != NULL) {
    nrt->passes = SETunionWith(nrt->passes, nrt->fields);
  }

  DBUG_VOID_RETURN;
}

/* Checks the provided output types against the declaration in the ntypesig
 * for correctness. For an input-only typemap in the ntypesig, the check always
 * succeeds, and the provided outputs are copied into the typemap. */
bool TYPcheckOutputs(TYPntypesig *sig, TYPnrectype *in, TYPvrectype *outs)
{
  DBUG_ENTER("TYPcheckOutputs");

  bool out = TRUE;

  TYPntypemap *finder = TYPnewNtypemap(TYPcopyNrectype(in), NULL, FALSE);
  TYPntypemap *m = SETfindElem(sig->maps, finder);
  freeNtypemap(finder);
  if (m == NULL) { /* map not found */
    /*
     * Note: initially, an Init -> Nil inferred map is considered OK when
     * matched against a declared sig with no init map. But because init maps
     * are useful to signal the presence of any initters in the topology, it
     * is important that an init map is declared.
     * 
    if (in == NULL && (outs == NULL || SETsize(outs->nrts) == 0)) {
      out = TRUE;
    }
    else { */
      out = FALSE;
    /* } */
  }
  else if (m->inputsOnly) {
    m->inputsOnly = FALSE;
    TYPfreeVrectype(m->rhs);
    m->rhs = TYPnewVrectype();
    /* add one by one with completion instead of just assigning the rhs */
    int i = outs == NULL ? -1 : SETsize(outs->nrts) - 1;
    for (; i >= 0; i--) {
      TYPnrectype *r = SETelem(outs->nrts, i);
      r = TYPcopyNrectype(r);
      completeNrectype(m->lhs, r);
      TYPfeedVrectype(m->rhs, r);
    }
  }
  else {
    out = TYPisSubtypeOfV(m->rhs, outs);
  }

  DBUG_RETURN(out);
}

/* Gets the inputs from an ntypesig. */
TYPvrectype *TYPntypesigIns(TYPntypesig *nts)
{
  DBUG_ENTER("TYPntypesigIns");

  DBUG_ASSERT(nts != NULL, "TYPntypesigIns called with NULL nts argument");

  TYPvrectype *out = TYPnewVrectype();
  int i = SETsize(nts->maps) - 1;
  for (; i >= 0; i--) {
    TYPntypemap *m = SETelem(nts->maps, i);
    if (m->lhs != NULL) {
      TYPfeedVrectype(out, TYPcopyNrectype(m->lhs));
    }
  }

  DBUG_RETURN(out);
}

/* Merges ntsb into ntsa. Returns TRUE if successful and FALSE if there was
 * any input-only conflict. */
bool TYPntypesigMergeInto(TYPntypesig *ntsa, TYPntypesig *ntsb)
{
  DBUG_ENTER("TYPntypesigMergeInto");

  bool out = TRUE;

  DBUG_ASSERT(ntsa != NULL && ntsb != NULL,
    "TYPntypesigMergeInto called with NULL argument(s)");

  int i, isize = SETsize(ntsb->maps);
  for (i = 0; i < isize && out; i++) {
    TYPntypemap *m = SETelem(ntsb->maps, i);
    out = out && TYPfeedNtypesig(ntsa,
      TYPcopyNrectype(m->lhs), TYPcopyVrectype(m->rhs), m->inputsOnly);
  }

  DBUG_RETURN(out);
}




/***************************************************************
 *                                                             *
 *                         Type Inference                      *
 *                                                             *
 ***************************************************************/






/******************************* Synchrocells ******************************/


/* Infers the type signature for a synchrocell. */
TYPntypesig *TYPinferSync(TYPnrectype *main, TYPvrectype *aux)
{
  DBUG_ENTER("TYPinferSync");

  DBUG_ASSERT(main != NULL && aux != NULL,
      "TYPinferSync called with NULL argument");

  TYPntypesig *out = TYPnewNtypesig();

  /* the output types for main input */
  TYPvrectype *v = TYPnewVrectype();
  TYPnrectype *col = cleanCopyNrectype(main);
  /* set all fields from main as passes */
  col->passes = SETunionWith(col->passes, main->fields);
  /* col is now the result of main type passed through */
  TYPfeedVrectype(v, TYPcopyNrectype(col));
  /* go on collecting all entries into col */
  int i, size = SETsize(aux->nrts);
  for (i = 0; i < size; i++) {
    TYPnrectype *var = SETelem(aux->nrts, i);
    col->btags = SETunionWith(col->btags, var->btags);
    col->fields = SETunionWith(col->fields, var->fields);
    /* any fields in main and one of aux types may be overridden by the aux
     * type fields so they should not be in the passes */
    col->passes = SETsubtractFrom(col->passes, var->fields);
  }
  /* col is now the synced type */
  TYPfeedVrectype(v, col);
  /* main typemap, pass-through and sync outputs */
  TYPfeedNtypesig(out, main, v, FALSE);

  /* populates the output ntypesig with aux pass-through typemaps */
  for (i = 0; i < size; i++) {
    TYPnrectype *lhs = cleanCopyNrectype(SETelem(aux->nrts, i));
    TYPsetAllPass(lhs);
    TYPvrectype *rhs = TYPnewVrectype();
    TYPfeedVrectype(rhs, TYPcopyNrectype(lhs));
    TYPfeedNtypesig(out, lhs, rhs, FALSE);
  }

  TYPfreeVrectype(aux);
  DBUG_RETURN(out);
}


/********************************* Dotdots ***********************************/

/* set freer for later usage */
static void *freeSet(void *s) { return SETfreeSet(s); }


/* Variable naming scheme for dotdot helpers:
 * left sig = ls, right sig = rs,
 * one of left sig's typemaps = lm, one of right sig's typemaps = rm,
 * one of left/right sig's input (nrectype) = lin/rin,
 * all left/right sig's inputs (vrectype) = liv/riv,
 * one of left/right sig's output (nrectype) = lon/ron,
 * one left/right sig typemap's all outputs (vrectype) = lov/rov. */

/* dotdot helper: Requirement Variants (RV in the report),
 * given one output variant from left operand, and the right operand sig,
 * returns a set of sets of fields, each smaller set can be added to the input
 * of the left operand's chosen typemap to make the chosen output fit into
 * the right operand's sig. */
static set *ddhRequirementVariants(TYPnrectype *lon, TYPntypesig *rs, bool prt)
{
  DBUG_ENTER("ddhRequirementVariants");

  set *out = SETnewSet(SETcompare);

  if (rs != NULL) {
    TYPvrectype *riv = TYPntypesigIns(rs); /* to check better matches */
    int i = SETsize(rs->maps) - 1;
    for (; i >= 0; i--) {
      TYPntypemap *rm = SETelem(rs->maps, i);
      if (rm->lhs == NULL) continue;
      TYPnrectype *rin = rm->lhs;
      if (!nrtBTEquals(lon, rin)) continue;
      set *stillNeed = SETsubtract(rin->fields, lon->fields);
      set *cantProvide = SETintersect(stillNeed, lon->discards);
      if (SETisEmpty(cantProvide)) {
        /* DO NOT check lon U stillNeed will indeed go to rm 
         * because we need to revive some maps -- which are probably
         * subsumed by troublesome and more attractive maps */
        if (prt && (TSDOPRINT)) {
          TYPnrectype *toprint = TYPnewNrectype();
          toprint->fields = SETunionWith(toprint->fields, stillNeed);
          char *chrstoprint = TYPprintNrectype(toprint);
          TSPRINT("    can add %s for right map %d.", chrstoprint, i + 1);
          MEMfree(chrstoprint);
          TYPfreeNrectype(toprint);
        }
        out = SETaddElem(out, stillNeed);
        stillNeed = NULL; /* don't free me later */
      }
      SETfreeSet(stillNeed);
      SETfreeSet(cantProvide);
    }
    TYPfreeVrectype(riv);
  }

  DBUG_RETURN(out);
}

/* dotdot helper: tells if a re-route is required for the left operand.
 * Returns NULL if not required, and a vrectype of accepted inputs (Ins of
 * original sig minus those causing troubles) if so. */
TYPvrectype *TYPddhCheckRerouteNecessity(TYPntypesig *ls, TYPntypesig *rs)
{
  DBUG_ENTER("TYPddhCheckRerouteNecessity");

  DBUG_ASSERT(ls != NULL && rs != NULL,
      "TYPddhCheckRerouteNecessity called with empty args");
  TYPvrectype *out = TYPnewVrectype();
  bool allPass = TRUE, mePass;
  int i = SETsize(ls->maps) - 1, j;
  for (; i >= 0; i--) {
    TYPntypemap *lm = SETelem(ls->maps, i);
    if (lm->lhs == NULL) continue;
    DBUG_ASSERT(lm->rhs != NULL,
        "TYPddhCheckRerouteNecessity called with incomplete sig");
    mePass = TRUE;
    for (j = SETsize(lm->rhs->nrts) - 1; j >= 0; j--) {
      TYPnrectype *lon = SETelem(lm->rhs->nrts, j);
      if (TSDOPRINT) {
        TSPRINT("  Left map %d, output variant %d:", i + 1, j + 1);
      }
      set *rv = ddhRequirementVariants(lon, rs, TRUE);
      if (SETsize(rv) == 0) {
        allPass = mePass = FALSE;
        if (TSDOPRINT) {
          TSPRINT("    can never match right sig.");
        }
      }
      SETfreeSetWith(rv, freeSet);
    }
    if (mePass) {
      TYPfeedVrectype(out, TYPcopyNrectype(lm->lhs));
    }
  }

  if (allPass) { /* no need to output anything */
    out = TYPfreeVrectype(out);
  }
  DBUG_RETURN(out);
}

/* dotdot helper: Possible Augmentations (PA in the report). All possible
 * combinations of Requirement Variants, one requirement variant per output
 * variant. Return value is also a set of sets of fields, each small set
 * makes a complete augmentation for the input of the typemap in question. */
static set *ddhPossibleAugmentations(TYPvrectype *lov, TYPntypesig *rs)
{
  DBUG_ENTER("ddhPossibleAugmentations");

  set *out = SETnewSet(SETcompare);

  TYPnrectype *lon = TYPpopVrectype(lov);
  if (lon == NULL) { /* nothing left in pov */
    /* then 1 possible augmentation: add nothing */
    set *nothing = SETnewSet(reCompare);
    out = SETaddElem(out, nothing);
  }
  else {
    /* I handle the popped nrectype */
    set *myrv = ddhRequirementVariants(lon, rs, FALSE);
    /* a recursion handles the rest */
    set *otherspa = ddhPossibleAugmentations(lov, rs);
    /* do a cross join */
    int i, isize = SETsize(myrv), j, jsize = SETsize(otherspa);
    for (i = 0; i < isize; i++) {
      set *myrv0 = SETelem(myrv, i);
      for (j = 0; j < jsize; j++) {
        set *otherspa0 = SETelem(otherspa, j);
        set *mypa = SETunion(myrv0, otherspa0);
        out = SETaddElem(out, mypa);
      }
    }
    /* garbage */
    SETfreeSetWith(myrv, freeSet);
    SETfreeSetWith(otherspa, freeSet);
    /* put the popped nrectype back to the set */
    TYPfeedVrectype(lov, lon);
  }

  DBUG_RETURN(out);
}

/* dotdot helper: Augmented Inputs for one typemap (AI0 in report). */
static TYPvrectype *ddhAugmentedInputs0(TYPntypesig *ls, int i, TYPntypesig *rs,
    bool prt)
{
  DBUG_ENTER("ddhAugmentedInputs0");

  TYPvrectype *out = TYPnewVrectype();
  TYPvrectype *liv = TYPntypesigIns(ls);

  TYPntypemap *lm = SETelem(ls->maps, i);
  TYPvrectype *lov = lm->rhs;
  DBUG_ASSERT(lov != NULL, "ddhAugmentedInputs0 called with incomplete sig");

  set *pa = ddhPossibleAugmentations(lov, rs);
  int myms = SETsize(lm->lhs->fields), j = SETsize(pa) - 1;
  for (; j >= 0; j--) {
    set *req = SETelem(pa, j);
    TYPnrectype *newlon = TYPcopyNrectype(lm->lhs);
    newlon->fields = SETunionWith(newlon->fields, req);
    /* check better match */
    if (TYPbestMatchScore(newlon, liv) == myms) {
      TYPfeedVrectype(out, newlon);
    }
    else {
      TYPfreeNrectype(newlon);
    }
  }
  SETfreeSetWith(pa, freeSet);

  if (prt && TSDOPRINT) {
    char *chrsout = TYPprintVrectype(out);
    TSPRINT("  Left map %d input augmented to %s.", i + 1, chrsout);
    MEMfree(chrsout);
  }

  /* TODO: in next version where Bottom is available in the typemap,
   * an empty "out" means the source typemap should not exist at all,
   * and mark the input of typemap as "dangerous". If in the final
   * inferred sig, a typemap accepts a SUPERtype of the dangerous input,
   * then this input should be in the resulting sig as "in -> Bottom" */

  TYPfreeVrectype(liv);
  DBUG_RETURN(out);
}

/* dotdot helper: Augmented Inputs (complete version; AI in report) */
static TYPvrectype *ddhAugmentedInputs(TYPntypesig *ls, TYPntypesig *rs, bool prt)
{
  DBUG_ENTER("ddhAugmentedInputs");

  DBUG_ASSERT(ls != NULL && rs != NULL,
    "ddhAugmentedInputs called with empty argument(s)");

  TYPvrectype *out = TYPnewVrectype();

  int i = SETsize(ls->maps) - 1;
  for (; i >= 0; i--) {
    TYPntypemap *lm = SETelem(ls->maps, i);
    if (lm->lhs != NULL) {
      out = TYPcombineVrectype(out, ddhAugmentedInputs0(ls, i, rs, prt));
    }
  }

  DBUG_RETURN(out);
}

/* Infers the type signature for a serial composition. */
static TYPntypesig *inferDotDot(node *arg_node, TYPntypesig *ls,
    TYPntypesig *rs, bool prt)
{
  DBUG_ENTER("TYPinferDotDot");

  DBUG_ASSERT(ls != NULL && rs != NULL,
    "TYPinferDotDot called with NULL argument(s)");

  TYPntypesig *out = TYPnewNtypesig();

  /* first do inits */

  TYPvrectype *initouts = NULL;
  TYPntypemap *linitmap = getInitMap(ls);
  int i = linitmap == NULL ? -1 : SETsize(linitmap->rhs->nrts) - 1;
  TYPnrectype *lin, *rin;
  TYPvrectype *lov, *rov;
  for (; i >= 0 && out != NULL; i--) {
    rin = SETelem(linitmap->rhs->nrts, i);
    rov = evalOutputs(rs, rin);
    if (prt && TSDOPRINT) {
      char *chrsrov = rov == NULL ? STRcpy("error") : TYPprintVrectype(rov);
      TSPRINT("  Left init output variant %d produces %s.", i + 1, chrsrov);
      MEMfree(chrsrov);
    }
    if (rov == NULL) {
      char *chrsrin = TYPprintNrectype(rin);
      CTIwarnNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		  arg_node, "Type error in this serial composition: "
		  "Left init output variant %s not accepted by right.", chrsrin);
      out = TYPfreeNtypesig(out); /* NULL as a signal to skip everything */
      MEMfree(chrsrin);
    }
    else {
      initouts = TYPcombineVrectype(initouts, rov);
    }
  }
  if (out != NULL) { /* the abovementioned signal */
    rov = TYPgetInitResults(rs);
    if (rov != NULL && prt && TSDOPRINT) {
      char *chrsrov = TYPprintVrectype(rov);
      TSPRINT("  Right init outputs: %s.", chrsrov);
      MEMfree(chrsrov);
    }
    initouts = TYPcombineVrectype(initouts, rov);
    if (initouts != NULL) {
      TYPfeedNtypesig(out, NULL, initouts, FALSE);
    }
    /* done init map inference */

    /* next do standard maps */

    TYPvrectype *ai = ddhAugmentedInputs(ls, rs, prt);
    while ((lin = TYPpopVrectype(ai)) != NULL) {
      if (prt && TSDOPRINT) {
        char *chrslin = TYPprintNrectype(lin);
        TSPRINT("  Left augmented input %s", chrslin);
        MEMfree(chrslin);
      }
      /* set all-pass for evalOutputs to infer pass-throughs */
      TYPsetAllPass(lin);
      lov = evalOutputs(ls, lin);
      DBUG_ASSERT(lov != NULL, "Algorithm error in dotdot 1");
      if (prt && TSDOPRINT) {
        char *chrslov = TYPprintVrectype(lov);
        TSPRINT("    produces intermediate outputs %s.", chrslov);
        MEMfree(chrslov);
      }
      rov = NULL;
      bool linValid = TRUE;
      while (linValid && (rin = TYPpopVrectype(lov)) != NULL) {
        if (prt && TSDOPRINT) {
          char *chrsrin = TYPprintNrectype(rin);
          TSPRINT("    Intermediate output %s", chrsrin);
          MEMfree(chrsrin);
        }
        TYPvrectype *myrov = evalOutputs(rs, rin);
        if (myrov == NULL) {
          /* this lon(rin) doesn't work; whole left map invalidated */
          if (prt && TSDOPRINT) {
            TSPRINT("      is not accepted by right. "
                "Left augmented input invalid.");
          }
          linValid = FALSE;
        }
        else {
          if (prt && TSDOPRINT) {
            char *chrsrov = TYPprintVrectype(myrov);
            TSPRINT("      produces final outputs %s.", chrsrov);
            MEMfree(chrsrov);
          }
          rov = TYPcombineVrectype(rov, myrov);
        }
      }
      TYPfreeVrectype(lov);
      if (linValid) {
        TYPfeedNtypesig(out, lin, rov, FALSE);
      }
      else {
        TYPfreeNrectype(lin);
        TYPfreeVrectype(rov);
      }
    }
    TYPfreeVrectype(ai);
  }
  else { /* if signaled by init dotdot inference to stop due to error */
    out = TYPnewNtypesig(); /* return empty ntypesig instead of null */
  }

  DBUG_RETURN(out);
}

/* Infers the type signature for a serial composition. */
TYPntypesig *TYPinferDotDot(node *arg_node, TYPntypesig *ls, TYPntypesig *rs)
{
  return inferDotDot(arg_node, ls, rs, TRUE);
}

/********************************** Bars **********************************/


/* Adapts the routing information to the routing restriction */
bool TYPadaptRoutingInfo(TYPvrectype *ri, TYPvrectype *restr, bool exact)
{
  DBUG_ENTER("TYPadaptRoutingInfo");

  DBUG_ASSERT(ri != NULL && restr != NULL, "TYPadaptRoutingInfo called with"
      "empty args");

  bool hasChanges = FALSE;

  if (exact) {
    int oldSize = SETsize(ri->nrts);
    ri->nrts = SETintersectWith(ri->nrts, restr->nrts);
    hasChanges = SETsize(ri->nrts) != oldSize;
  }
  else {
    /* if not exact, restr is likely from beginning of signed net, allow
     * all supertypes in ri. */
    int i = SETsize(ri->nrts) - 1, j, j0 = SETsize(restr->nrts) - 1;
    for (; i >= 0; i--) {
      TYPnrectype *nri = SETelem(ri->nrts, i);
      bool notFound = TRUE;
      for (j = j0; j >= 0 && notFound; j--) {
        TYPnrectype *nrr = SETelem(restr->nrts, j);
        if (TYPisSubtypeOfN(nri, nrr)) { /* nri from routing info is super */
          notFound = FALSE;
        }
      }
      if (notFound) {
        SETremoveElemAt(ri->nrts, i); /* == nri */
        TYPfreeNrectype(nri);
        hasChanges = TRUE;
      }
    }
  }

  DBUG_RETURN(hasChanges);
}

/* Infers the type signature for a parallel branch. For default routing info
 * (no filtering) just use nts and do not call this function. */
TYPntypesig *TYPinferBarBranch(TYPntypesig *nts, TYPvrectype *ri)
{
  DBUG_ENTER("TYPinferBarBranch");

  DBUG_ASSERT(nts != NULL && ri != NULL,
    "TYPinferBarBranch called with NULL argument(s)");

  TYPntypesig *out = TYPnewNtypesig();

  int i, isize = SETsize(nts->maps);
  for (i = 0; i < isize; i++) {
    TYPntypemap *m = SETelem(nts->maps, i);
    if (m->lhs == NULL || SEThasElem(ri->nrts, m->lhs)) {
      TYPntypemap *m2 = TYPnewNtypemap(TYPcopyNrectype(m->lhs),
          TYPcopyVrectype(m->rhs), m->inputsOnly);
      out->maps = SETaddElem(out->maps, m2); /* shortcut */
    }
  }

  DBUG_RETURN(out);
}


/********************************** Star ***********************************/

/* star helper: core part of sig, the maps where inputs aren't uterms.
 * including init map. */
static TYPntypesig *shCoreSig(TYPntypesig *sig, TYPvrectype *utv)
{
  DBUG_ENTER("shCoreSig");

  TYPntypesig *out = TYPnewNtypesig();

  int i = SETsize(sig->maps) - 1;
  for (; i >= 0; i--) {
    TYPntypemap *m = SETelem(sig->maps, i);
    if (m->lhs == NULL || !TYPisCapturedBy(utv, m->lhs)) {
      DBUG_ASSERT(!m->inputsOnly, "shCoreSig encountered an incomplete sig");
      TYPntypemap *m2 = TYPnewNtypemap(TYPcopyNrectype(m->lhs),
          TYPcopyVrectype(m->rhs), FALSE);
      out->maps = SETaddElem(out->maps, m2); /* shortcut */
    }
  }
  DBUG_RETURN(out);
}

/* star helper: collects combinations of binding tags found in inputs and 
 * outputs of the sig. */
static void shCollectBTags(TYPntypesig *sig,
    TYPvrectype *inBTs, TYPvrectype *outBTs)
{
  DBUG_ENTER("shCollectBTags");

  int i = SETsize(sig->maps) - 1, j;
  for (; i >= 0; i--) {
    TYPntypemap *m = SETelem(sig->maps, i);
    if (inBTs != NULL && m->lhs != NULL) {
      TYPnrectype *copy = cleanCopyNrectype(m->lhs);
      copy->fields = SETclearSet(copy->fields);
      TYPfeedVrectype(inBTs, copy);
    }
    DBUG_ASSERT(m->rhs != NULL, "shCollectBTags encountered an incomplete sig");
    if (outBTs != NULL) {
      for (j = SETsize(m->rhs->nrts) - 1; j >= 0; j--) {
        TYPnrectype *ron = SETelem(m->rhs->nrts, j);
        TYPnrectype *copy = cleanCopyNrectype(ron);
        copy->fields = SETclearSet(copy->fields);
        TYPfeedVrectype(outBTs, copy);
      }
    }
  }

  DBUG_VOID_RETURN;
}

/* star helper: builds signature for the tagger filter (F in the report),
 * which tags the types matching termination patterns (passes and tags for
 * conditional termination patterns) and passes the other types. Provide
 * a vrectype holding all btag combinations needed */
static TYPntypesig *shTaggerSig(TYPvrectype *utv, TYPvrectype *ctv,
    TYPvrectype *bts)
{
  DBUG_ENTER("shTaggerSig");

  TYPntypesig *out = TYPnewNtypesig();

  /* pass-throughs */
  int i;
  for (i = SETsize(bts->nrts) - 1; i >= 0; i--) {
    TYPnrectype *lhs = SETelem(bts->nrts, i);
    if (TYPisCapturedBy(utv, lhs)) continue; /* this binding tag combination
                                      immediately matches unconditional term */
    lhs = TYPcopyNrectype(lhs);
    TYPvrectype *rhs = TYPnewVrectype();
    TYPfeedVrectype(rhs, TYPcopyNrectype(lhs));
    TYPfeedNtypesig(out, lhs, rhs, FALSE);
  }

  /* conditional terms */
  for (i = SETsize(ctv->nrts) - 1; i >= 0; i--) {
    TYPnrectype *lhs = SETelem(ctv->nrts, i);
    if (TYPisCapturedBy(utv, lhs)) continue; /* this conditional term matches an
                       unconditional term and should be unconditional instead */
    lhs = TYPcopyNrectype(lhs);
    TYPsetAllPass(lhs);
    TYPvrectype *rhs = TYPnewVrectype();
    /* the pass-through bit */
    TYPfeedVrectype(rhs, TYPcopyNrectype(lhs));
    /* the termination bit */
    TYPnrectype *rhs1 = TYPcopyNrectype(lhs);
    rhs1->btags = SETaddElem(rhs1->btags, NULL); /* special tag */
    TYPfeedVrectype(rhs, rhs1);
    TYPfeedNtypesig(out, lhs, rhs, FALSE);
  }

  /* unconditional terms */
  for (i = SETsize(utv->nrts) - 1; i >= 0; i--) {
    TYPnrectype *lhs = SETelem(utv->nrts, i);
    lhs = TYPcopyNrectype(lhs);
    TYPsetAllPass(lhs);
    TYPvrectype *rhs = TYPnewVrectype();
    /* the termination bit */
    TYPnrectype *rhs1 = TYPcopyNrectype(lhs);
    rhs1->btags = SETaddElem(rhs1->btags, NULL); /* special tag */
    TYPfeedVrectype(rhs, rhs1);
    TYPfeedNtypesig(out, lhs, rhs, FALSE);
  }

  DBUG_RETURN(out);
}

/* star helper: builds signature for one star unit (C .. F | P) */
static TYPntypesig* shStarUnitSig(TYPntypesig *coresig,
    TYPntypesig *taggersig, TYPvrectype *utv, TYPvrectype *ctv, bool prt)
{
  DBUG_ENTER("shStarUnitSig");

  if (prt && TSDOPRINT) {
    TSPRINT("Inferring 'Core .. Tagger'...");
  }
  TYPntypesig *out = inferDotDot(NULL, coresig, taggersig, prt);

  /* locally performs CoreSig again because during dotdot, some input may be
   * augmented too much they match a uterm. */
  int i = SETsize(out->maps) - 1;
  for (; i >= 0; i--) {
    TYPntypemap *m = SETelem(out->maps, i);
    if (m->lhs != NULL && TYPisCapturedBy(utv, m->lhs)) {
      SETremoveElemAt(out->maps, i);
      freeNtypemap(m);
    }
  }

  /* add passer maps */
  utv = TYPcopyVrectype(utv);
  TYPnrectype *one;
  while ((one = TYPpopVrectype(utv)) != NULL) {
    one->btags = SETaddElem(one->btags, NULL);
    TYPsetAllPass(one);
    TYPvrectype *rhs = TYPnewVrectype();
    TYPfeedVrectype(rhs, TYPcopyNrectype(one));
    TYPfeedNtypesig(out, one, rhs, FALSE);
  }
  TYPfreeVrectype(utv);
  ctv = TYPcopyVrectype(ctv);
  while ((one = TYPpopVrectype(ctv)) != NULL) {
    one->btags = SETaddElem(one->btags, NULL);
    TYPsetAllPass(one);
    TYPvrectype *rhs = TYPnewVrectype();
    TYPfeedVrectype(rhs, TYPcopyNrectype(one));
    TYPfeedNtypesig(out, one, rhs, FALSE);
  }
  TYPfreeVrectype(ctv);

  if (TSDOPRINT) {
    char *chrsout = TYPprintNtypesig(out);
    TSPRINT("Sig of iteration unit '(Core .. Tagger) | Passer' is\n%s.",
        chrsout);
    MEMfree(chrsout);
  }

  DBUG_RETURN(out);
}

/* star helper: tells if a re-route is required for the operand.
 * Returns NULL if not required, and a vrectype of accepted inputs (Ins of
 * original sig minus those causing troubles) if so. */
TYPvrectype *TYPshCheckRerouteNecessity(TYPntypesig *sig, TYPvrectype *utv,
    TYPvrectype *ctv)
{
  DBUG_ENTER("TYPshCheckRerouteNecessity");

  DBUG_ASSERT(sig != NULL && utv != NULL && ctv != NULL,
      "TYPshCheckRerouteNecessity called with empty args");

  TYPntypesig *core = shCoreSig(sig, utv);
  TYPvrectype *outBTs = TYPnewVrectype();
  shCollectBTags(core, NULL, outBTs);
  TYPntypesig *tagger = shTaggerSig(utv, ctv, outBTs);

  if (TSDOPRINT) {
    char *chrscore = TYPprintNtypesig(core);
    char *chrstagger = TYPprintNtypesig(tagger);
    TSPRINT("  Core sig is:\n%s,\n  Tagger sig is:\n%s.", chrscore, chrstagger);
    MEMfree(chrscore);
    MEMfree(chrstagger);
  }

  TYPntypesig *unit = shStarUnitSig(core, tagger, utv, ctv, TRUE);

  if (TSDOPRINT) {
    TSPRINT("If 'Unit .. Unit' ...");
  }
  TYPvrectype *out = TYPddhCheckRerouteNecessity(unit, unit);
  TYPfreeNtypesig(core);
  TYPfreeNtypesig(tagger);
  TYPfreeNtypesig(unit);
  TYPfreeVrectype(outBTs);

  if (out != NULL) {
    /* contains special tagged types; to remove them */
    int i = SETsize(out->nrts) - 1;
    for (; i >= 0; i--) {
      TYPnrectype *one = SETelem(out->nrts, i);
      if (SEThasElem(one->btags, NULL)) {
        SETremoveElemAt(out->nrts, i);
        TYPfreeNrectype(one);
      }
    }
    if (TSDOPRINT) {
      TSPRINT("Should refine.");
    }
  }
  else if (TSDOPRINT) {
    TSPRINT("'Unit .. Unit' is ok.");
  }

  DBUG_RETURN(out);
}

/* Infers the type signature for a serial replication. Arguments utv and
 * ctv are the termination patterns without and with conditions, collected 
 * in vrectype objects. */
TYPntypesig *TYPinferStar(node *arg_node, TYPntypesig *sig, TYPvrectype *utv,
                          TYPvrectype *ctv)
{
  DBUG_ENTER("TYPinferStar");

  DBUG_ASSERT(sig != NULL && utv != NULL && ctv != NULL,
    "TYPinferStar called with NULL argument(s)");

  TYPntypesig *core = shCoreSig(sig, utv);
  TYPvrectype *inBTs = TYPnewVrectype(), *outBTs = TYPnewVrectype();
  shCollectBTags(core, inBTs, outBTs);
  TYPntypesig *tagger = shTaggerSig(utv, ctv, outBTs);
  TYPntypesig *unit = shStarUnitSig(core, tagger, utv, ctv, FALSE);

  int iteration = 0;
  TYPntypesig *out = shTaggerSig(utv, ctv, inBTs);
  TYPvrectype *seen = TYPnewVrectype();
  int prevSeenSize = -1;
  bool errorFree = TRUE;

  TYPfreeNtypesig(core);
  TYPfreeVrectype(inBTs);
  TYPfreeVrectype(outBTs);
  TYPfreeNtypesig(tagger);

  if (TSDOPRINT) {
    char *iter0 = TYPprintNtypesig(out);
    TSPRINT("Iteration 0 (termination patterns only):\n%s.", iter0);
    MEMfree(iter0);
  }

  while (errorFree && iteration <= TYPSTAR_ITERLIMIT
      && SETsize(seen->nrts) > prevSeenSize) {
    iteration++;
    prevSeenSize = SETsize(seen->nrts);
    if (TSDOPRINT) {
      TSPRINT("Iteration %d: inferring 'previous .. Unit'...", iteration);
    }
    TYPntypesig *tmp = TYPinferDotDot(arg_node, out, unit);
    TYPfreeNtypesig(out);
    out = tmp;
    if (TYPisEmptyNtypesig(out)) {
      errorFree = FALSE;
    }
    else {
      int i = SETsize(out->maps) - 1;
      for (; i >= 0; i--) {
        TYPntypemap *m = SETelem(out->maps, i);
        int j = SETsize(m->rhs->nrts) - 1;
        for (; j >= 0; j--) {
          TYPnrectype *see = SETelem(m->rhs->nrts, j);
          TYPfeedVrectype(seen, TYPcopyNrectype(see));
        }
      }
      if (TSDOPRINT) {
        char *iterN = TYPprintNtypesig(out);
        TSPRINT("Iteration %d sig:\n%s.", iteration, iterN);
        MEMfree(iterN);
      }
    }
  }

  if (!errorFree) {
    CTIwarnNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		arg_node, "Serial replication encountered a type error at "
		"iteration %d. For safety, give an empty sig.", iteration);
    TYPfreeNtypesig(out);
    out = TYPnewNtypesig();
  }
  else if (iteration > TYPSTAR_ITERLIMIT) {
    CTIwarnNode(CTI_ERRNO_TYPE_INFERENCE_ERROR,
		arg_node, "Serial replication sig still not fixed after %d "
		"iterations. For safety, give an empty sig.", TYPSTAR_ITERLIMIT);
  }
  else { /* ok */
    if (TSDOPRINT) {
      TSPRINT("All output variants now repeat previous iterations. "
          "Preparing sig...");
    }
    /* now clean up the sig */
    int i = SETsize(out->maps) - 1, j;
    for (; i >= 0; i--) {
      TYPntypemap *m = SETelem(out->maps, i);
      TYPnrectype *in = m->lhs;
      if (in != NULL && SEThasElem(in->btags, NULL)) {
        /* input has special tag, this map is just for passing through */
        SETremoveElemAt(out->maps, i);
        freeNtypemap(m);
      }
      else if ((j = SETsize(m->rhs->nrts)) == 0) {
        /* sink map, ok to include */
      }
      else {
        for (j--; j >= 0; j--) {
          TYPnrectype *on = SETelem(m->rhs->nrts, j);
          if (SEThasElem(on->btags, NULL)) {
            /* special-tagged output is real, remove tag */
            SETremoveElem(on->btags, NULL);
          }
          else {
            /* non-tagged output is "loopback", remove */
            SETremoveElemAt(m->rhs->nrts, j);
            TYPfreeNrectype(on);
          }
        }
        if (SETsize(m->rhs->nrts) == 0) { /* was non-sink but now "sink" */
          if (TSDOPRINT) {
            TSPRINT("  In map %d, no outputs can escape the serial chain, "
                "potentially causing infinite unfolding. Usually unsafe "
                "except introduced by an aux type of synchrocell", i + 1);
          }
        }
      } /* end if nonsink map */
    } /* end for each map to clean */
  } /* end if everything ok */

  TYPfreeNtypesig(unit);
  TYPfreeVrectype(seen);

  DBUG_RETURN(out);
}


/***************************************** Ex ********************************/


/* Infers the type signature for a parallel replication. Argument tag is
 * an STagRef or BTagRef node. */
TYPntypesig *TYPinferEx(TYPntypesig *nts, node *tag)
{
  DBUG_ENTER("TYPinferEx");

  DBUG_ASSERT(nts != NULL && tag != NULL,
    "TYPinferEx called with NULL argument(s)");

  TYPntypesig *out = TYPnewNtypesig();
  TYPvrectype *initouts = TYPgetInitResults(nts);
  if (initouts != NULL) {
    TYPfeedNtypesig(out, NULL, initouts, FALSE);
  }
  TYPvrectype *ins = TYPntypesigIns(nts);
  TYPnrectype *in;
  while ((in = TYPpopVrectype(ins)) != NULL) {
    TYPsetAllPass(in);
    TYPfeedNrectype(in, tag, LQUA_pass);
    TYPvrectype *ov = evalOutputs(nts, in);
    if (ov != NULL) {
      TYPfeedNtypesig(out, in, ov, FALSE);
    }
  }
  TYPfreeVrectype(ins);

  DBUG_RETURN(out);
}

/************************** Finishing **************************/


/* Creates a FieldRef node from a field reference */
static inline node *createFieldRefNode(node *fieldref)
{
  DBUG_ENTER("createFieldRefNode");

  node *out = TBmakeFieldref(fieldref, STRcpy(FIELDS_NAME(fieldref)));
  NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(fieldref));

  DBUG_RETURN(out);
}

/* Creates a STagRef node from a stag reference */
static inline node *createSTagRefNode(node *stagref)
{
  DBUG_ENTER("createSTagRefNode");

  node *out = TBmakeStagref(stagref, STRcpy(STAGS_NAME(stagref)));
  NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(stagref));

  DBUG_RETURN(out);
}

/* Creates a BTagRef node from a btag reference */
static inline node *createBTagRefNode(node *btagref)
{
  DBUG_ENTER("createBTagRefNode");

  node *out = TBmakeBtagref(btagref, STRcpy(BTAGS_NAME(btagref)));
  NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(btagref));

  DBUG_RETURN(out);
}

/* Creates a RecEntries node from an nrectype object */
static node *createRecEntriesNode(TYPnrectype *obj)
{
  DBUG_ENTER("createRecEntriesNode");

  node *out = NULL;
  node *temp = NULL;
  int i;
  /* Constructs the node backwards -- {btags, fields{passes}, discards}
   * so start with discards */
  for (i = SETsize(obj->discards) - 1; i >= 0; i--) {
    node *ref = SETelem(obj->discards, i);
    if (NODE_TYPE(ref) == N_fields) {
      temp = createFieldRefNode(ref);
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(ref));

      out = TBmakeRecentries(temp, NULL, NULL, out);
      NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(ref));

      RECENTRIES_QUALIFIER(out) = LQUA_disc;
    }
    else { /* N_stags */
      temp = createSTagRefNode(ref);
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(ref));

      out = TBmakeRecentries(NULL, temp, NULL, out);
      NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(ref));

      RECENTRIES_QUALIFIER(out) = LQUA_disc;
    }
  }
  for (i = SETsize(obj->fields) - 1; i >= 0; i--) {
    node *ref = SETelem(obj->fields, i);
    if (NODE_TYPE(ref) == N_fields) {
      temp = createFieldRefNode(ref);
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(ref));

      out = TBmakeRecentries(temp, NULL, NULL, out);
      NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(ref));
    }
    else { /* N_stags */
      temp = createSTagRefNode(ref);
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(ref));

      out = TBmakeRecentries(NULL, temp, NULL, out);
      NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(ref));
    }
    RECENTRIES_QUALIFIER(out) =
      SEThasElem(obj->passes, ref) ? LQUA_pass : LQUA_none;
  }
  for (i = SETsize(obj->btags) - 1; i >= 0; i--) {
    node *btagsNode = SETelem(obj->btags, i);
    if (btagsNode == NULL) continue; /* star special tag */

    temp = createBTagRefNode(btagsNode);
    NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(btagsNode));

    out = TBmakeRecentries(NULL, NULL, temp, out);
    NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(btagsNode));
  }

  DBUG_RETURN(out);
}

/* Creates a Types node from a vrectype object */
node *TYPcreateTypesNode(TYPvrectype *obj)
{
  DBUG_ENTER("TYPcreateTypesNode");

  node *out = NULL;
  node *recEntries = NULL;
  node *temp = NULL;
  int i = obj == NULL ? -1 : SETsize(obj->nrts) - 1;
  for (; i >= 0; i--) {
    recEntries = createRecEntriesNode(SETelem(obj->nrts, i));

    temp = TBmakeRectype(recEntries);
    if (recEntries != NULL) {
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(recEntries));
    }

    out = TBmakeTypes(temp, out);
    if (recEntries != NULL) {
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(recEntries));
    }
  }

  DBUG_RETURN(out);
}

/* Creates a TypeMap node from an ntypemap object */
static inline node *createTypeMapNode(TYPntypemap *obj)
{
  DBUG_ENTER("createTypeMapNode");

  node *recEntries = NULL;
  node *temp = NULL;
  node *out = NULL;

  if (obj->lhs != NULL) {
    recEntries = createRecEntriesNode(obj->lhs);
    temp = TBmakeRectype(recEntries);
    if (recEntries != NULL) {
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(recEntries));
    }
    temp = TBmakeTypes(temp, NULL);
    if (recEntries != NULL) {
      NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(recEntries));
    }
  }

  out = TBmakeTypemap(/* OutputToInfer */ obj->inputsOnly,
	                    /* InType */ temp,
	                    /* OutType */ obj->rhs == NULL ? NULL :
	                                  TYPcreateTypesNode(obj->rhs));

  if (recEntries != NULL) {
    NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(recEntries));
  }

  DBUG_RETURN(out);
}

/* Creates a TypeSigns node from an ntypesig object */
node *TYPcreateTypeSigns(TYPntypesig *nts)
{
  DBUG_ENTER("TYPcreateTypeSigns");

  DBUG_ASSERT(nts != NULL, "TYPcreateTypeSigns called with NULL argument");

  node *out = NULL;
  node *temp = NULL;
  int i;
  for (i = SETsize(nts->maps) - 1; i >= 0; i--) {
    temp = createTypeMapNode(SETelem(nts->maps, i));

    out = TBmakeTypesigns(temp, out);
    NODE_ERRCODE(out) = STRcpy(NODE_ERRCODE(temp));
  }
  DBUG_RETURN(out);
}

/* helpers for pretty-print */
typedef struct {
  char *buf;
  int len;
  int pos;
} bufstr;

static bufstr *appendStr(bufstr *src, const char *app)
{
  DBUG_ENTER("appendStr");

  if (src == NULL) {
    src = MEMmalloc(sizeof(bufstr));
    src->buf = MEMmalloc(512);
    src->len = 512;
    src->pos = 0;
  }

  int applen = strlen(app);
  if (src->pos + applen >= src->len) {
    int newlen = src->len + 512;
    while (newlen <= src->pos + applen) {
      newlen += 512;
    }
    char *newbuf = MEMmalloc(newlen);
    strncpy(newbuf, src->buf, src->pos);
    MEMfree(src->buf);
    src->buf = newbuf;
    src->len = newlen;
  }
  strcpy(src->buf + src->pos, app);
  src->pos += applen;

  DBUG_RETURN(src);
}

static char *finalizeStr(bufstr *buf)
{
  DBUG_ENTER("finalizeStr");

  char *ret = buf->buf;
  MEMfree(buf);

  DBUG_RETURN(ret);
}

static bufstr *printNrectype(bufstr *buf, TYPnrectype *nrt)
{
  DBUG_ENTER("printNrectype");

  buf = appendStr(buf, "{");
  int i, btlen = SETsize(nrt->btags),
      flen = SETsize(nrt->fields), dlen = SETsize(nrt->discards);
  for (i = 0; i < btlen; i++) {
    buf = appendStr(buf, "<#");
    node *bt = SETelem(nrt->btags, i);
    buf = appendStr(buf, bt == NULL ? "*" : BTAGS_NAME(bt)); /* star special */
    buf = appendStr(buf, ">,");
  }
  for (i = 0; i < flen; i++) {
    node *f = SETelem(nrt->fields, i);
    if (SEThasElem(nrt->passes, f)) {
      buf = appendStr(buf, "=");
    }
    if (NODE_TYPE(f) == N_fields) {
      buf = appendStr(buf, FIELDS_NAME(f));
    }
    else {
      buf = appendStr(buf, "<");
      buf = appendStr(buf, STAGS_NAME(f));
      buf = appendStr(buf, ">");
    }
    buf = appendStr(buf, ",");
  }
  for (i = 0; i < dlen; i++) {
    node *f = SETelem(nrt->discards, i);
    buf = appendStr(buf, "\\");
    if (NODE_TYPE(f) == N_fields) {
      buf = appendStr(buf, FIELDS_NAME(f));
    }
    else {
      buf = appendStr(buf, "<");
      buf = appendStr(buf, STAGS_NAME(f));
      buf = appendStr(buf, ">");
    }
    buf = appendStr(buf, ",");
  }
  if ((btlen | flen | dlen) != 0) {
    buf->pos--;
  }
  buf = appendStr(buf, "}");

  DBUG_RETURN(buf);
}

char *TYPprintNrectype(TYPnrectype *nrt)
{
  DBUG_ENTER("TYPprintNrectype");
  bufstr *buf = printNrectype(NULL, nrt);
  char *out = finalizeStr(buf);
  DBUG_RETURN(out);
}

bufstr *printVrectype(bufstr *buf, TYPvrectype *vrt)
{
  DBUG_ENTER("printVrectype");

  if (vrt == NULL || SETsize(vrt->nrts) == 0) {
    buf = appendStr(buf, "/* Nil */");
  }
  else {
    int i, len = SETsize(vrt->nrts);
    for (i = 0; i < len; i++) {
      buf = printNrectype(buf, SETelem(vrt->nrts, i));
      if (i + 1 < len) {
        buf = appendStr(buf, " | ");
      }
    }
  }

  DBUG_RETURN(buf);
}

/* Constructs a string of a vrectype */
char *TYPprintVrectype(TYPvrectype *vrt)
{
  DBUG_ENTER("TYPprintNtypesig");
  bufstr *buf = printVrectype(NULL, vrt);
  char *out = finalizeStr(buf);
  DBUG_RETURN(out);
}

/* Constructs a string of an ntypesig pretty-printed */
char *TYPprintNtypesig(TYPntypesig *nts)
{
  DBUG_ENTER("TYPprintNtypesig");

  if (nts == NULL || SETsize(nts->maps) == 0) {
    DBUG_RETURN(STRcpy("()"));
  }

  bufstr *buf = appendStr(NULL, "( ");
  int i, mapsize = SETsize(nts->maps);
  for (i = 0; i < mapsize; i++) {
    TYPntypemap *map = SETelem(nts->maps, i);
    buf = map->lhs == NULL ?
        appendStr(buf, "/* Init */") : printNrectype(buf, map->lhs);
    buf = appendStr(buf, " -> ");
    buf = map->inputsOnly ?
        appendStr(buf, "...") : printVrectype(buf, map->rhs);
    buf = appendStr(buf, ",\n  ");
  }
  buf->pos -= 4;
  buf = appendStr(buf, " )");

  char *out = finalizeStr(buf);
  DBUG_RETURN(out);
}
