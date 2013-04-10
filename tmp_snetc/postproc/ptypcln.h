/*************************
 * 
 * $Id: ptypcln.h 1761 2007-11-30 14:51:39Z hxc $
 * 
 * Post processing type cleaning: removes all backslashed record entries
 * and releases the NTypeSig attributes from BoxDef and NetDef nodes.
 * 
 * Author: Max (Haoxuan Cai), Imperial College London
 * 
 * 2007.11.30
 *
 ************************/

#ifndef _SNETC_PTYPCLN_H_
#define _SNETC_PTYPCLN_H_

#include "types.h"

/* top func */
extern node *PPdoTypeClean (node * syntax_tree);

/* node trav funcs */
extern node *PPTCLNboxdef(node *arg_node, info *arg_info);
extern node *PPTCLNnetdef(node *arg_node, info *arg_info);
extern node *PPTCLNnetbody(node *arg_node, info *arg_info);
extern node *PPTCLNrecentries(node *arg_node, info *arg_info);

#endif /*_SNETC_PTYPCLN_H_*/
