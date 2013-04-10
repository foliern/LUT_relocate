/******************************************************************************
 * $Id: copy.c 1723 2007-11-12 15:09:56Z cg $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   06.06.2007
 *
 * Description:
 *
 * Top-level copy function implementation.
 *
 *****************************************************************************/

#include "copy.h"
#include "dbug.h"

#include "traverse.h"
#include "tree_basic.h"

/******************************************************************************
 *
 * Function:
 *   node *COPYdoCOPYTree( node *copy_node)
 *
 * Description: Copies node referred by copy_node and all it's
 *              children. Returns the copy if succesful, NULL otherwise.
 * 
 *              NOTICE: NodePtr -attributes will point to the original
 *                      nodes, not to any copied node. (Otherwise deep copy)
 *
 ******************************************************************************/

node *
COPYdoCopyTree (node * copy_node)
{
  DBUG_ENTER ("COPYdoCopyTree");

  TRAVpush (TR_copy);

  if(copy_node != NULL){
    copy_node = TRAVdo(copy_node, NULL);
  }

  TRAVpop ();

  DBUG_RETURN (copy_node);
}


