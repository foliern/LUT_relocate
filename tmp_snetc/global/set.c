/*******************************************************************************
 *
 * $Id: set.c 2381 2009-05-16 05:44:52Z hxc $
 *
 * Implementation file for Set operations
 * 
 * Max Troy (Haoxuan Cai), Imperial College London
 * 
 * 071107
 *
 *******************************************************************************/

#include "set.h"
#include "dbug.h"
#include "memory.h"

/* a set */
struct SET {
  int size;
  int cap;
  bool found;
  SETcomparer comp;
  void *elem[];
};

/* pointer equality comparer */
static int ptrcompare(void *a, void *b) {
  return a - b;
}

/* makes a new set from an existing set */
set *SETnewSetFrom(set *another)
{
  set *result;
  
  DBUG_ENTER("SETnewSetFrom");
  
  DBUG_ASSERT(another != NULL, "SETnewSetFrom called with NULL origin set");
  
  result = ILIBmemCopy(sizeof(set) + another->cap * sizeof(void *), another);
  
  DBUG_RETURN(result);
}

/* makes a new set with a specified initial capacity */
set *SETnewSetOf(int cap, SETcomparer comp)
{
  set *result;
  
  DBUG_ENTER("SETnewSet");
  
  if (cap <= 0) {
    cap = 1;
  }
  if (comp == NULL) {
    comp = &ptrcompare;
  }
  
  result = MEMmalloc(sizeof(set) + cap * sizeof(void *));
  result->cap = cap;
  result->comp = comp;
  result->size = 0;
  result->found = FALSE;
  
  DBUG_RETURN(result);
}

/* frees a set. Returns NULL. */
set *SETfreeSetWith(set *s, SETfreer freer)
{
  DBUG_ENTER("SETfreeSetWith");
  
  if (s != NULL) {
    s = SETclearSetWith(s, freer);
    MEMfree(s);
  }
  
  DBUG_RETURN(NULL);
}

/* clears all elements from a set. */
set *SETclearSetWith(set *s, SETfreer freer)
{
  DBUG_ENTER("SETclearSetWith");
  
  DBUG_ASSERT(s != NULL, "SETclearSetWith called with a NULL set");
  
  if (freer != NULL) {
    int i = 0;
    for (; i < s->size; i++) {
      freer(s->elem[i]);
    }
  }

  s->size = 0;
  
  DBUG_RETURN(s);
}


/* gets the size of a set */
int SETsize(set *s)
{
  DBUG_ENTER("SETsize");
  
  DBUG_ASSERT(s != NULL, "SETsize called with a NULL set");

  DBUG_RETURN(s->size);
}

/* checks if a set is empty */
bool SETisEmpty(set *s)
{
  DBUG_ENTER("SETisEmpty");
  
  DBUG_ASSERT(s != NULL, "SETisEmpty called with a NULL set");

  DBUG_RETURN(s->size == 0);
}

/* gets an element from the set */
void *SETelem(set *s, int loc)
{
  DBUG_ENTER("SETelem");
  
  DBUG_ASSERT(s != NULL, "SETelem called with a NULL set");
  DBUG_ASSERT(0 <= loc && loc < s->size,
    "SETelem called with an out of bound location");
  
  DBUG_RETURN(s->elem[loc]);
}

/* Makes sure the set can accommodate one more element */
static inline set *ensureCapacity(set *s)
{
  DBUG_ENTER("ensureCapacity");
  
  if (s->size == s->cap) {
    set *new = SETnewSetOf((s->cap << 1) | 1, s->comp);
    int i = 0;
    for (; i < s->size; i++) {
      new->elem[i] = s->elem[i];
    }
    new->size = i;
    new->found = s->found;
    SETfreeSet(s);
    s = new;
  }
  
  DBUG_RETURN(s);
}

/* Finds the location for an element */
static inline int findLoc(set *s, void *e)
{
  DBUG_ENTER("findLoc");
  
  s->found = FALSE;
  
  int l = 0, h = s->size, m, cr;
  while (l < h) {
    m = (l + h) >> 1;
    cr = s->comp(s->elem[m], e);
    if (cr == 0) {
      l = h = m;
      s->found = TRUE;
    }
    else if (cr < 0) /* s[m] < e */ {
      l = m + 1;
    }
    else if (cr > 0) /* s[m] > e */ {
      h = m;
    }
  }
  
  DBUG_RETURN(l);
}

/* adds an element into the set at the specified location */
static inline set *addElemAt(set *s, int loc, void *e)
{
  DBUG_ENTER("addElemAt");
  
  s = ensureCapacity(s);
  int i = s->size - 1;
  for (; i >= loc; i--) {
    s->elem[i + 1] = s->elem[i];
  }
  s->elem[loc] = e;
  s->size++;
  
  DBUG_RETURN(s);
}

/* adds an element into the set. Returns a possibly updated pointer. */
set *SETaddElem(set *s, void *e)
{
  DBUG_ENTER("SETaddElem");
  
  DBUG_ASSERT(s != NULL, "SETaddElem called with an empty set");
  
  int i = findLoc(s, e);
  if (!s->found) {
    s = addElemAt(s, i, e);
  }
  
  DBUG_RETURN(s);
}

/* checks whether an element is in the set */
bool SEThasElem(set *s, void *e)
{
  DBUG_ENTER("SEThasElem");
  
  DBUG_ASSERT(s != NULL, "SEThasElem called with an empty set");
  
  findLoc(s, e);
  
  DBUG_RETURN(s->found);
}

/* finds an element. Returns the pointer to the element in the set,
 * or NULL if not found. */
void *SETfindElem(set *s, void *e)
{
  DBUG_ENTER("SETfindElem");
  
  DBUG_ASSERT(s != NULL, "SETfindElem called with an empty set");
  
  int i = findLoc(s, e);
  
  DBUG_RETURN(s->found ? s->elem[i] : NULL);
}

/* removes an element from the specified location of a set */
static inline void removeElemAt(set *s, int loc)
{
  DBUG_ENTER("removeElemAt");
  
  for (; loc < s->size - 1; loc++) {
    s->elem[loc] = s->elem[loc + 1];
  }
  s->size--;
  
  DBUG_VOID_RETURN;
}

/* removes an element from a set if present. Returns the removed element. */
void *SETremoveElem(set *s, void *e)
{
  int i;
  void *founde;
  
  DBUG_ENTER("SETremoveElem");
  
  DBUG_ASSERT(s != NULL, "SETremoveElem called with an empty set");
  
  i = findLoc(s, e);
  if (!s->found) {
    DBUG_RETURN(e);
  }
  founde = s->elem[i];
  removeElemAt(s, i);
  
  DBUG_RETURN(founde);
}

/* returns whether an element was found */
bool SETwasFound(set *s)
{
  DBUG_ENTER("SETwasFound");
  
  DBUG_RETURN(s->found);
}

/* removes an element from the specified location of a set and returns
 * the removed element */
void *SETremoveElemAt(set *s, int loc)
{
  DBUG_ENTER("SETremoveElemAt");
  
  DBUG_ASSERT(s != NULL, "SETremoveElemAt called with a NULL set");
  DBUG_ASSERT(loc >= 0 && loc < s->size,
    "SETremoveElemAt called with an out of bound location");
  
  void *e = s->elem[loc];
  removeElemAt(s, loc);
  
  DBUG_RETURN(e);
}

/* checks equality of two sets. */
bool SETequals(set *a, set *b)
{
  DBUG_ENTER("SETequals");
  
  DBUG_ASSERT(a != NULL && b != NULL, "SETequals called with NULL set(s)");
  
  if (a->comp != b->comp || a->size != b->size) {
    DBUG_RETURN(FALSE);
  }
  
  /* no call to SEThasElem: because sorted, hand-code a faster algo */
  int i = 0;
  for (; i < a->size; i++) {
    if (a->comp(a->elem[i], b->elem[i]) != 0) {
      DBUG_RETURN(FALSE);
    }
  }
  
  DBUG_RETURN(TRUE);
}

/* checks subset relationship */
bool SETisSubsetOf(set *super, set *sub)
{
  DBUG_ENTER("SETisSubsetOf");
  
  DBUG_ASSERT(super != NULL && sub != NULL,
    "SETisSubsetOf called with empty set(s)");
  
  if (super->comp != sub->comp || super->size < sub->size) {
    DBUG_RETURN(FALSE);
  }
  
  int i = 0;
  for (; i < sub->size; i++) {
    if (!SEThasElem(super, sub->elem[i])) {
      DBUG_RETURN(FALSE);
    }
  }
  
  DBUG_RETURN(TRUE);
}

/* set intersection. Returns the first set */
set *SETintersectWith(set *a, set *b)
{
  DBUG_ENTER("SETintersectWith");
  
  DBUG_ASSERT(a != NULL && b != NULL,
    "SETintersectWith called with empty set(s)");
  DBUG_ASSERT(a->comp == b->comp,
    "SETintersectWith called with incompatible sets");

  int i = a->size - 1;
  for (; i >= 0; i--) {
    if (!SEThasElem(b, a->elem[i])) {
      removeElemAt(a, i);
    }
  }
  
  DBUG_RETURN(a);
}

/* set intersection. Returns a new set. */
set *SETintersect(set *a, set *b)
{
  DBUG_ENTER("SETintersect");
  
  DBUG_ASSERT(a != NULL && b != NULL,
    "SETintersect called with empty set(s)");
  DBUG_ASSERT(a->comp == b->comp,
    "SETintersect called with incompatible sets");
  
  set *new = SETnewSet(a->comp);
  int i = 0;
  for (; i < a->size; i++) {
    if (SEThasElem(b, a->elem[i])) {
      new = addElemAt(new, new->size, a->elem[i]);
    }
  }
  
  DBUG_RETURN(new);
}

/* set union. Returns the first set whose pointer is possibly updated */
set *SETunionWith(set *a, set *b)
{
  DBUG_ENTER("SETunionWith");
  
  DBUG_ASSERT(a != NULL && b != NULL,
    "SETunionWith called with empty set(s)");
  DBUG_ASSERT(a->comp == b->comp,
    "SETunionWith called with incompatible sets");
  
  int i = 0;
  for (; i < b->size; i++) {
    a = SETaddElem(a, b->elem[i]);
  }

  DBUG_RETURN(a);
}

/* set union. Returns a new set. */
set *SETunion(set *a, set *b)
{
  DBUG_ENTER("SETunion");
  
  DBUG_ASSERT(a != NULL && b != NULL, "SETunion called with empty set(s)");
  DBUG_ASSERT(a->comp == b->comp, "SETunion called with incompatible sets");

  set *out = SETunionWith(SETnewSetFrom(a), b);
  
  DBUG_RETURN(out);
}

/* set subtraction. Returns the first set. */
set *SETsubtractFrom(set *a, set *b)
{
  DBUG_ENTER("SETsubtractFrom");
  
  DBUG_ASSERT(a != NULL && b != NULL,
    "SETsubtractFrom called with empty set(s)");
  DBUG_ASSERT(a->comp == b->comp,
    "SETsubtractFrom called with incompatible sets");
  
  int i = 0;
  for (; i < b->size; i++) {
    SETremoveElem(a, b->elem[i]);
  }

  DBUG_RETURN(a);
}

/* set subtraction. Returns a new set. */
set *SETsubtract(set *a, set *b)
{
  DBUG_ENTER("SETsubtract");
  
  DBUG_ASSERT(a != NULL && b != NULL, "SETsubtract called with empty set(s)");
  DBUG_ASSERT(a->comp == b->comp, "SETsubtract called with incompatible sets");
  
  set *new = SETnewSet(a->comp);
  int i = 0;
  for (; i < a->size; i++) {
    if (!SEThasElem(b, a->elem[i])) {
      new = addElemAt(new, new->size, a->elem[i]);
    }
  }

  DBUG_RETURN(new);
}

/* set comparer */
int SETcompare(void *setA, void *setB)
{
  DBUG_ENTER("SETcomparer");
  
  set *a = (set *)setA, *b = (set *)setB;
  
  DBUG_ASSERT(a != NULL && b != NULL, "SETcomparer called with empty set(s)");
  
  int cr = (void *)a->comp - (void *)b->comp;

  if (cr == 0) {
    int i = 0;
    for (; i < a->size && i < b->size && cr == 0; i++) {
      cr = a->comp(a->elem[i], b->elem[i]);
    }
    if (cr == 0) {
      cr = a->size - b->size;
    }
  }
  
  DBUG_RETURN(cr);
}
