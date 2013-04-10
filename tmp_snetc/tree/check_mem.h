/*
 * $Id: check_mem.h 1723 2007-11-12 15:09:56Z cg $
 */



#ifndef _SAC_CHECK_MEM_H_
#define _SAC_CHECK_MEM_H_

#include "types.h"

#ifdef SHOW_MALLOC
extern void CHKMdeinitialize();

extern node *CHKMdoMemCheck( node *syntax_tree);
extern void CHKMtouch( void *shifted_ptr, info *arg_info); 
extern node *CHKMappendErrorNodes( node *arg_node, info *arg_info);

extern void *CHKMregisterMem( int size, void *orig_ptr);
extern void *CHKMunregisterMem( void *shifted_ptr);

extern void CHKMsetNodeType( node *shifted_ptr, nodetype newnodetype);
extern void CHKMsetLocation(node *shifted_ptr, char *file, int line);
extern void CHKMsetSubphase( node *shifted_ptr, char *subphase);
extern void CHKMsetTraversal( node *shifted_ptr, trav_t *traversal); 

extern void CHKMdoNotReport( void *shifted_ptr);

extern int CHKMgetSize( node *shifted_ptr);

#else
#define CHKMappendErrorNodes( arg_node, arg_info)
#define CHKMtouch( shifted_ptr, arg_info) 
#define CHKMdoNotReport( shifter_ptr)
#endif /* SHOW_MALLOC */


#endif /* _SAC_CHECK_MEM_H_ */
