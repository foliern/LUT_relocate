/*
 * $Id: tree_compound.h 1723 2007-11-12 15:09:56Z cg $
 */


/*============================================================================

How to use this file ?

While tree_basic.h contains the basic structure of the new virtual syntax
tree, this file is dedicated to more elaborate macros and function
declarations.

These are sorted towards the structures they are used for similar to
tree_basic.h. Please observe that all macros in this file and all functions
in tree_compound.c should exclusively use the new virtual syntax tree.
They must not (!!) contain direct accesses to the underlying data structure.

All comments in relation to the outward behaviour of functions should be
given in the usual form in this (!) file, not in tree_compound.c.
The reason is to give a quick overview of the provided facilities
(macros and functions) in a single file. Of course, comments to the
specific implementation of a function should remain with the source code.

============================================================================*/


#ifndef _SAC_TREE_COMPOUND_H_
#define _SAC_TREE_COMPOUND_H_


#include "types.h"


/***
 ***  N_error :
 ***/

extern node *TCappendError( node *chain, node *item);

#endif  /* _SAC_TREE_COMPOUND_H_ */
