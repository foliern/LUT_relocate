/*
 *
 * $Log$
 * Revision 1.2  2004/11/25 23:49:44  sah
 * added ugly funs for CMPtree
 *
 * Revision 1.1  2004/11/23 22:21:41  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_TRAVERSE_HELPER_H_
#define _SAC_TRAVERSE_HELPER_H_

#include "types.h"

extern node *TRAVsons( node *arg_node, info *arg_info);
extern node *TRAVnone( node *arg_node, info *arg_info);
extern node *TRAVerror( node *arg_node, info *arg_info);

/* these functions are a ugly hack for CMPtree! */
extern int TRAVnumSons( node *parent);
extern node *TRAVgetSon( int no, node *parent);

#endif /* _SAC_TRAVERSE_HELPER_H_ */       







