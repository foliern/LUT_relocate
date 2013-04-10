/*******************************************************************************
 *
 * $Id: set.h 1733 2007-11-14 18:33:31Z hxc $
 *
 * Header file for Set operations
 * 
 * Max Troy (Haoxuan Cai), Imperial College London
 * 
 * 071107
 *
 *******************************************************************************/

#ifndef _SNET_SET_H_
#define _SNET_SET_H_

#include "types.h"

/* comparer that compares two non-NULL set elements;
 * returns 0 for a==b, <0 for a<b and >0 for a>b */ 
typedef int (*SETcomparer)(void *a, void *b);

/* a set */
typedef struct SET set;

/* makes a new set. Not specifying a comparer causes the internal logic
 * to compare entries by pointer value. */
#define SETnewSet(comp) SETnewSetOf(3, comp)

/* makes a new set from an existing set (clones) */
set *SETnewSetFrom(set *another);

/* makes a new set with a specified initial capacity */
set *SETnewSetOf(int cap, SETcomparer comp);

/* element freer that releases resources held by an element */
typedef void *(*SETfreer)(void *e);

/* frees a set. */
#define SETfreeSet(s) SETfreeSetWith(s, NULL)

/* frees a set by first freeing the elements with the freer. Returns NULL. */
set *SETfreeSetWith(set *s, SETfreer freer);

/* gets the size of a set */
int SETsize(set *s);

/* checks if a set is empty */
bool SETisEmpty(set *s);

/* gets an element from the set */
void *SETelem(set *s, int loc);

/* adds an element into the set. Returns a possibly updated pointer. */
set *SETaddElem(set *s, void *e);

/* checks whether an element is in the set */
bool SEThasElem(set *s, void *e);

/* finds an element. Returns the pointer to the element in the set,
 * or NULL if not found. */
void *SETfindElem(set *s, void *e);

/* removes an element from a set if present. Returns the removed element. */
void *SETremoveElem(set *s, void *e);

/* returns whether the element was found. Value is updated after SETaddElem,
 * SEThasElem, SETfindElem, and SETremoveElem. addElem + TRUE means the 
 * argument element was NOT added due to duplication; removeElem + FALSE 
 * means the argument element was NOT removed and the return value of the 
 * call to removeElem was the argument element instead of the removed one. */
bool SETwasFound(set *s);

/* removes an element from the specified location of a set and returns
 * the removed element */
void *SETremoveElemAt(set *s, int loc);

/* clears all elements from a set. */
#define SETclearSet(s) SETclearSetWith(s, NULL)

/* clears all elements from a set by first freeing the elements with the
 * freer. Returns the original pointer. */
set *SETclearSetWith(set *s, SETfreer freer);

/* checks equality of two sets. */
bool SETequals(set *a, set *b);

/* checks subset relationship */
bool SETisSubsetOf(set *super, set *sub);

/* set intersection. Returns the first set. */
set *SETintersectWith(set *a, set *b);

/* set intersection. Returns a new set. */
set *SETintersect(set *a, set *b);

/* set union. Returns the first set whose pointer is possibly updated */
set *SETunionWith(set *a, set *b);

/* set union. Returns a new set. */
set *SETunion(set *a, set *b);

/* set subtraction. Returns the first set. */
set *SETsubtractFrom(set *a, set *b);

/* set subtraction. Returns a new set. */
set *SETsubtract(set *a, set *b);

/* set comparer */
int SETcompare(void *setA, void *setB);

#endif /*_SNET_SET_H_*/
