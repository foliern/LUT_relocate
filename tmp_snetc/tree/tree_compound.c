/*
 * $Id: tree_compound.c 1723 2007-11-12 15:09:56Z cg $
 */



#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"


/*--------------------------------------------------------------------------*/

/*
 * macro template for append functions
 */

#define APPEND( ret, type, atype, chain, item) \
  if (item != NULL) {                          \
    if (chain == NULL) {                       \
      chain = item;                            \
    }                                          \
    else {                                     \
      type __tmp;                              \
      __tmp = chain;                           \
      while( atype##_NEXT( __tmp) != NULL) {   \
        __tmp = atype##_NEXT( __tmp);          \
      }                                        \
      atype##_NEXT( __tmp) = item;             \
    }                                          \
  }                                            \
  ret = chain



/*--------------------------------------------------------------------------*/

/***
 ***  ERROR :
 ***/


node *TCappendError( node *chain, node *item)
{
  node *ret;

  DBUG_ENTER("TCappendError");

  APPEND( ret, node *, ERROR, chain, item);

  DBUG_RETURN( ret);
}



