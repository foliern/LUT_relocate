/*
 * $Id: traverse.c 2489 2009-07-30 13:52:55Z hxc $
 */


#include "traverse.h"
#include "dbug.h"
#include "traverse_tables.h"
#include "traverse_helper.h"
#include "globals.h"
#include "memory.h"
#include "tree_basic.h"

struct TRAVSTACK_T {
  struct TRAVSTACK_T *next;
  travfun_p           *funs;
  trav_t              traversal;
};

typedef struct TRAVSTACK_T travstack_t;

static travstack_t *travstack = NULL;


node *TRAVdo(node *arg_node, info *arg_info)
{
  nodetype arg_node_type;
  char *old_file = global.nodefile;
  int old_line = global.nodeline;
  int old_col = global.nodecol;

  /*
   * check whether the node is non null as this
   * is a common reason for segfaults
   */
  DBUG_ASSERT( (arg_node != NULL),
               "tried to traverse a node which is NULL!");

  /*
   * Make sure the location will be set
   * correctly in case MakeXxx is called.
   */
  global.nodefile = NODE_FILE(arg_node);
  global.nodeline = NODE_LINE(arg_node);
  global.nodecol  = NODE_COL( arg_node);

  /*
   * Save node type as it might be modified during traversal
   */
  arg_node_type = NODE_TYPE( arg_node);

  if (pretable[ travstack->traversal] != NULL) {
    arg_node = pretable[ travstack->traversal]( arg_node, arg_info);
  }

  arg_node = (travstack->funs[ arg_node_type])( arg_node, arg_info);

  if (posttable[ travstack->traversal] != NULL) {
    arg_node = posttable[ travstack->traversal]( arg_node, arg_info);
  }

  global.nodefile = old_file;
  global.nodeline = old_line;
  global.nodecol  = old_col;

  return( arg_node);
}

node *TRAVcont( node *arg_node, info *arg_info)
{
  arg_node = TRAVsons( arg_node, arg_info);

  return( arg_node);
}

void TRAVpush( trav_t traversal)
{
  travstack_t *new;

  DBUG_ENTER("TRAVpush");

  new = MEMmalloc( sizeof( travstack_t));

  new->next = travstack;
  new->traversal = traversal;
  new->funs = travtables[ traversal];

  travstack = new;

  DBUG_VOID_RETURN;
}

trav_t TRAVpop() {
  travstack_t *tmp;
  trav_t result;

  DBUG_ENTER("TRAVpop");

  DBUG_ASSERT( (travstack != NULL),
      "no traversal on stack!");

  tmp = travstack;
  travstack = tmp->next;
  result = tmp->traversal;

  tmp = MEMfree( tmp);

  DBUG_RETURN( result);
}

const char *TRAVgetName()
{
  const char *result;

  DBUG_ENTER("TRAVgetName");

  if (travstack == NULL) {
    result = "no_active_traversal";
  } else {
    result = travnames[ travstack->traversal];
  }

  DBUG_RETURN( result);
}

void TRAVsetPreFun( trav_t traversal, travfun_p prefun)
{
  DBUG_ENTER("TRAVsetPreFun");

  pretable[traversal] = prefun;

  DBUG_VOID_RETURN;
}

void TRAVsetPostFun( trav_t traversal, travfun_p postfun)
{
  DBUG_ENTER("TRAVsetPreFun");

  posttable[traversal] = postfun;

  DBUG_VOID_RETURN;
}
