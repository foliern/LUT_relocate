/*******************************************************************************
 *
 * $Id: netren.c 2047 2008-07-31 11:13:31Z jju $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   09.02.2007
 * -----
 *
 *******************************************************************************/

#include "netren.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "free_node.h"
#include "str.h"

/*
 * INFO structure
 */
struct INFO {
  node *parent;
};

#define INFO_PARENT(n)   (n->parent)

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_PARENT(result) = NULL;

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *PPRENboxdef(node *arg_node, info *arg_info)
{
  node *parentNet = NULL;
  char *oldName = NULL;

  DBUG_ENTER("PPRENboxdef");
  
  parentNet = INFO_PARENT(arg_info);
  
  if(parentNet != NULL) {

    oldName = BOXDEF_NAME(arg_node);

    if(BOXDEF_REALNAME(arg_node) == NULL) {
      BOXDEF_REALNAME(arg_node) = oldName;
      BOXDEF_NAME(arg_node) = STRcatn(3, NETDEF_NAME(parentNet), "__", oldName);
    }
    else {
      BOXDEF_NAME(arg_node) = STRcatn(3, NETDEF_NAME(parentNet), "__", oldName);
      MEMfree(oldName);
    }
  }
  
  DBUG_RETURN(arg_node);
}

node *PPRENnetdef(node *arg_node, info *arg_info)
{
  node *parentNet = NULL;
  char *oldName = NULL;
  node *oldParent = NULL;

  DBUG_ENTER("PPRENnetdef");

  parentNet = INFO_PARENT(arg_info);

  if(parentNet != NULL) {
    oldName = NETDEF_NAME(arg_node);
    NETDEF_NAME(arg_node) = STRcatn(3, NETDEF_NAME(parentNet), "__", oldName);
    MEMfree(oldName);
  }

  if(NETDEF_BODY(arg_node) != NULL) {
    oldParent = INFO_PARENT(arg_info);
    INFO_PARENT(arg_info) = arg_node;
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
    INFO_PARENT(arg_info) = oldParent;
  }

  DBUG_RETURN(arg_node);
}

node *PPdoRename(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("PPdoRename");
  
  DBUG_ASSERT((syntax_tree != NULL), "PPdoRename called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_ppren);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
