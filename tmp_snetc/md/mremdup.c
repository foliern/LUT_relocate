/*******************************************************************************
 *
 * $Id: mremdup.c 2483 2009-07-30 06:49:20Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   29.07.2009
 * -----
 *
 * Removing duplicate keys from net and box metadata
 *
 *******************************************************************************/

#include <string.h>
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "bool.h"

#include "mremdup.h"

struct INFO { 
  bool is_checking;
  node *current;
  bool is_duplicate;
};

#define INFO_IS_CHECKING(n)  (n->is_checking)
#define INFO_CURRENT(n)          (n->current)
#define INFO_IS_DUPLICATE(n)  (n->is_duplicate)

static void infoClear(info *inf)
{
  INFO_IS_CHECKING(inf) = FALSE;
  INFO_CURRENT(inf) = NULL;
  INFO_IS_DUPLICATE(inf) = FALSE;
}

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  infoClear(result);

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *MDREMDUPnetdef( node *arg_node, info *arg_info )
{

  DBUG_ENTER("MDREMDUPnetdef");

  if(NETDEF_METADATA(arg_node) != NULL) {
    NETDEF_METADATA(arg_node) = TRAVdo(NETDEF_METADATA(arg_node), arg_info);
  }

  INFO_IS_CHECKING(arg_info) = FALSE;
  INFO_CURRENT(arg_info) = NULL;
  INFO_IS_DUPLICATE(arg_info) = FALSE;

  if(NETDEF_BODY(arg_node) != NULL) {
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDREMDUPboxdef( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDREMDUPboxdef");

  if(BOXDEF_METADATA(arg_node) != NULL) {
    BOXDEF_METADATA(arg_node) = TRAVdo(BOXDEF_METADATA(arg_node), arg_info);
  }

  INFO_IS_CHECKING(arg_info) = FALSE;
  INFO_CURRENT(arg_info) = NULL;
  INFO_IS_DUPLICATE(arg_info) = FALSE;

  DBUG_RETURN(arg_node);
}


node *MDREMDUPmetadatakeylist( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("MDREMDUPmetadatakeylist");

  if(!INFO_IS_CHECKING(arg_info)) {
    /* See if this node has a duplicate */
    
    INFO_IS_CHECKING(arg_info) = TRUE;
    INFO_CURRENT(arg_info) = arg_node;
    INFO_IS_DUPLICATE(arg_info) = FALSE;

    /* Search for duplicate */
    if(METADATAKEYLIST_NEXT(arg_node) != NULL) {
      METADATAKEYLIST_NEXT(arg_node) = TRAVdo(METADATAKEYLIST_NEXT(arg_node), arg_info);
    }

    if(INFO_IS_DUPLICATE(arg_info)) {
      /* Handle rest of the lsit and then remove this node */

      INFO_IS_CHECKING(arg_info) = FALSE;

      if(METADATAKEYLIST_NEXT(arg_node) != NULL) {
	temp = TRAVdo(METADATAKEYLIST_NEXT(arg_node), arg_info);
      } 

      METADATAKEYLIST_NEXT(arg_node) = NULL;

      FREEdoFreeNode(arg_node);
      arg_node = temp;

    } else {
      /* Handle rest of the list */
      INFO_IS_CHECKING(arg_info) = FALSE;
      
      if(METADATAKEYLIST_NEXT(arg_node) != NULL) {
	METADATAKEYLIST_NEXT(arg_node) = TRAVdo(METADATAKEYLIST_NEXT(arg_node), arg_info);
      } 
    } 

  } else {
    /* Check if this node is a duplicate with the current node */
  
    if(STReq(METADATAKEYLIST_KEY(INFO_CURRENT(arg_info)), METADATAKEYLIST_KEY(arg_node))) {

      INFO_IS_DUPLICATE(arg_info) = TRUE;

    } else {

      if(METADATAKEYLIST_NEXT(arg_node) != NULL) {
	METADATAKEYLIST_NEXT(arg_node) = TRAVdo(METADATAKEYLIST_NEXT(arg_node), arg_info);
      }
    }
   
  }

  DBUG_RETURN(arg_node);
}


node *MDdoRemove(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("MDdoRemove");

  DBUG_ASSERT((syntax_tree != NULL), "MDdoRemove called with empty syntaxtree");
  
  TRAVpush(TR_mdremdup);

  inf = infoMake();

  syntax_tree = TRAVdo(syntax_tree, inf);

  inf = infoFree(inf);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

