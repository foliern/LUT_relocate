/*
 * $Id: free.h 1723 2007-11-12 15:09:56Z cg $
 */


#ifndef _SAC_FREE_H_
#define _SAC_FREE_H_

#include "types.h"


/*
 * Top-level free functions.
 *
 * These are the only functions which should be called from outside
 * the free module for freeing syntax (sub) trees.
 */

extern node *FREEdoFreeNode( node *arg_node);
extern node *FREEdoFreeTree( node *arg_node);


/*
 * traversal functions
 */

#include "free_node.h"


#endif /* _SAC_FREE_H_ */
