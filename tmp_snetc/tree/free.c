/*
 * $Id: free.c 1723 2007-11-12 15:09:56Z cg $
 */



#include "free.h"
#include "dbug.h"

#include "traverse.h"
#include "memory.h"
#include "free_info.h"
#include "tree_basic.h"


/*
 * INFO functions
 */

static 
info *MakeInfo ()
{
  info *result;

  DBUG_ENTER ("MakeInfo");

  result = MEMmalloc (sizeof (info));

  INFO_FREE_FLAG (result) = NULL;
  INFO_FREE_ASSIGN (result) = NULL;

  DBUG_RETURN (result);
}

static 
info *FreeInfo (info * info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree (info);

  DBUG_RETURN (info);
}

/*
 *  Important Remarks:
 *
 *  All removals of parts of or the entire syntax tree should be done
 *  by using the functions in this file.
 *
 */


/******************************************************************************
 *
 * Function:
 *   node *FREEdoFreeNode( node *free_node)
 *
 * Description:
 *   - if 'free_node' is not a N_fundef node:
 *        Removes the given node and returns a pointer to the NEXT node if it
 *        exists, NULL otherwise.
 *   - if 'free_node' is a N_fundef node:
 *        Transforms the given fundef into a zombie and returns it.
 *
 ******************************************************************************/

node *
FREEdoFreeNode (node * free_node)
{
  info *arg_info;
  
  DBUG_ENTER ("FREEfreeNode");

  arg_info = MakeInfo ();

  INFO_FREE_FLAG (arg_info) = free_node;

  TRAVpush (TR_free);

  free_node = TRAVdo (free_node, arg_info);

  TRAVpop ();

  arg_info = FreeInfo (arg_info);

  DBUG_RETURN (free_node);
}


/******************************************************************************
 *
 * Function:
 *   node *FREEdoFreeTree( node *free_node)
 *
 * Description:
 *   - if 'free_node' is not a N_fundef node:
 *        Removes the whole sub tree behind the given pointer.
 *   - if 'free_node' is a N_fundef node:
 *        Transforms the whole fundef chain into zombies and returns it.
 *
 ******************************************************************************/

node *
FREEdoFreeTree (node * free_node)
{
  info *arg_info;

  DBUG_ENTER ("FREEfreeTree");

  arg_info = MakeInfo ();
  INFO_FREE_FLAG (arg_info) = NULL;

  TRAVpush (TR_free);

  free_node = TRAVdo (free_node, arg_info);

  TRAVpop ();

  arg_info = FreeInfo (arg_info);

  DBUG_RETURN (free_node);
}


