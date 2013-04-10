/*
 *
 * $Log$
 * Revision 3.91  2004/12/01 14:33:07  sah
 * added support for TRAVsetPreFun TRAVsetPostFun
 *
 * Revision 3.90  2004/11/24 20:41:15  sah
 * added TRAVgetName
 *
 * Revision 3.89  2004/11/24 14:01:18  sah
 * added TRAVcont
 *
 * Revision 3.88  2004/11/23 22:22:03  sah
 * rewrite
 *
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 */

#ifndef _SAC_TRAVERSE_H_
#define _SAC_TRAVERSE_H_

#include "types.h"

extern node  *TRAVdo( node *arg_node, info *arg_info);
extern node  *TRAVcont( node *arg_node, info *arg_info);
extern void   TRAVpush( trav_t traversal);
extern trav_t TRAVpop();
extern const char *TRAVgetName();
extern void TRAVsetPreFun( trav_t traversal, travfun_p prefun);
extern void TRAVsetPostFun( trav_t traversal, travfun_p postfun);

#endif /* _SAC_TRAVERSE_H_ */       







