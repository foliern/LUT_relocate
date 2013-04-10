/*******************************************************************************
 *
 * $Id: netflat.c 2047 2008-07-31 11:13:31Z jju $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   07.02.2007
 * -----
 *
 *******************************************************************************/

#include "netflat.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "free.h"

node *PPFLATdefs(node *arg_node, info *arg_info)
{
  node *temp = NULL;
  node *tmp = NULL;

  DBUG_ENTER("PPFLATdefs");

  if(DEFS_NEXT(arg_node) != NULL) {
     DEFS_NEXT(arg_node) = TRAVdo(DEFS_NEXT(arg_node), arg_info); 
  }

  if(DEFS_DEF(arg_node) != NULL) {
    temp = TRAVdo(DEFS_DEF(arg_node), arg_info);

    if(temp != DEFS_DEF(arg_node)) {
      tmp = temp;

      for(;DEFS_NEXT(tmp) != NULL; tmp = DEFS_NEXT(tmp));
     
      DEFS_NEXT(tmp) = arg_node;
      arg_node = temp;
    }
  }

  DBUG_RETURN(arg_node);
}

node *PPFLATnetbody(node *arg_node, info *arg_info)
{
  node *bodyDefs = NULL;

  DBUG_ENTER("PPFLATnetbody");

  if(NETBODY_DEFS(arg_node) != NULL) {
    bodyDefs = TRAVdo(NETBODY_DEFS(arg_node), arg_info);

    NETBODY_DEFS(arg_node) = NULL;

    arg_node = bodyDefs;
  }
  else{
    arg_node = NULL;
  }  

  DBUG_RETURN(arg_node);
}

node *PPFLATnetdef(node *arg_node, info *arg_info)
{
  node *bodyDefs  = NULL;

  DBUG_ENTER("PPFLATnetdef");

  if(NETDEF_BODY(arg_node) != NULL) {

    bodyDefs = TRAVdo(NETDEF_BODY(arg_node), arg_info);

    if(bodyDefs != NULL) {
      arg_node = bodyDefs;
    }
  } 

  DBUG_RETURN(arg_node);
}

node *PPdoFlat(node *syntax_tree)
{
  DBUG_ENTER("PPdoFlat");
  
  DBUG_ASSERT((syntax_tree != NULL), "PPdoFlat called with empty syntaxtree");

  TRAVpush(TR_ppflat);

  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}
