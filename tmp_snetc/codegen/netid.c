#include "codefile.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "memory.h"

#include "netid.h"

static int count;
static bool netToId;

node *NETIDstar(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("NETIDstar");

  DBUG_ASSERT(NODE_TYPE(STAR_LEFT(arg_node)) == N_netrefs,
              "Star does not contain netref as left value.");
  net = NETREFS_NET(STAR_LEFT(arg_node));

  count++;
  if (netToId) {
    CODEFILEwriteNetToId(count, NETDEF_PKGNAME(net), NETDEF_NAME(net));
  } else {
    CODEFILEwriteIdToNet(count, NETDEF_PKGNAME(net), NETDEF_NAME(net));
  }

  DBUG_RETURN(arg_node);
}

node *NETIDsplit(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("NETIDsplit");

  DBUG_ASSERT(NODE_TYPE(SPLIT_LEFT(arg_node)) == N_netrefs,
              "Split does not contain netref as left value.");
  net = NETREFS_NET(SPLIT_LEFT(arg_node));

  count++;
  if (netToId) {
    CODEFILEwriteNetToId(count, NETDEF_PKGNAME(net), NETDEF_NAME(net));
  } else {
    CODEFILEwriteIdToNet(count, NETDEF_PKGNAME(net), NETDEF_NAME(net));
  }

  DBUG_RETURN(arg_node);
}

node *NETIDdoLookup(node *syntax_tree)
{
  DBUG_ENTER("NETIDdoLookup");

  DBUG_ASSERT((syntax_tree != NULL), "NETIDdoLookup called with empty syntaxtree");

  TRAVpush(TR_netid);

  count = 0;
  netToId = TRUE;
  CODEFILEwriteNetToIdFunStart();
  syntax_tree = TRAVdo(syntax_tree, NULL);
  CODEFILEwriteNetToIdFunEnd();

  count = 0;
  netToId = FALSE;
  CODEFILEwriteIdToNetFunStart();
  syntax_tree = TRAVdo(syntax_tree, NULL);
  CODEFILEwriteIdToNetFunEnd();

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}
