/*************************
 * 
 * $Id: ptypcln.c 2420 2009-06-11 17:26:29Z hxc $
 * 
 * Post processing type cleaning: removes all backslashed record entries
 * and releases the NTypeSig attributes from BoxDef and NetDef nodes.
 * 
 * Author: Max (Haoxuan Cai), Imperial College London
 * 
 * 2007.11.30
 *
 ************************/

#include "ptypcln.h"
#include "typing.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "free.h"
#include "print.h"

/* top func */
node *PPdoTypeClean(node * syntax_tree)
{
  DBUG_ENTER("PPdoTypeClean");
  
  DBUG_ASSERT((syntax_tree != NULL), "TIdoTypeInf called with empty syntaxtree");
  
  TRAVpush(TR_pptcln);
  
  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();
  
  DBUG_RETURN(syntax_tree);
}

/* node trav funcs */
node *PPTCLNboxdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PPTCLNboxdef");
  
  BOXDEF_NTYPESIG(arg_node) = TYPfreeNtypesig(BOXDEF_NTYPESIG(arg_node));
  
  DBUG_RETURN(arg_node);
}

node *PPTCLNnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PPTCLNnetdef");
  
  NETDEF_NTYPESIG(arg_node) = TYPfreeNtypesig(NETDEF_NTYPESIG(arg_node));
  
  if (NETDEF_SIGNED(arg_node)) {
    NETDEF_SIGN(arg_node) = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
  }
  else if (NETDEF_SIGN(arg_node) != NULL) {
    NETDEF_SIGN(arg_node) = FREEdoFreeTree(NETDEF_SIGN(arg_node));
  }
  
  if (NETDEF_BODY(arg_node) != NULL) {
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}

node *PPTCLNnetbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PPTCLNnetbody");
  
  if (NETBODY_DEFS(arg_node) != NULL) {
    NETBODY_DEFS(arg_node) = TRAVdo(NETBODY_DEFS(arg_node), arg_info);
  }
  
  /* does not traverse connect expression to save time */
  
  DBUG_RETURN(arg_node);
}

node *PPTCLNrecentries(node *arg_node, info *arg_info)
{
  node *next = NULL;

  DBUG_ENTER("PPTCLNrecentries");
  
  if (RECENTRIES_NEXT(arg_node) != NULL) {
    RECENTRIES_NEXT(arg_node) = TRAVdo(RECENTRIES_NEXT(arg_node), arg_info);
  }
  
  if (RECENTRIES_QUALIFIER(arg_node) == LQUA_disc) {
    next = RECENTRIES_NEXT(arg_node);
    RECENTRIES_NEXT(arg_node) = NULL;
    FREEdoFreeTree(arg_node);
    arg_node = next;
  }
  
  DBUG_RETURN(arg_node);
}
