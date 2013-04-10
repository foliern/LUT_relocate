/*******************************************************************************
 *
 * $Id$
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   01.02.2008
 * -----
 *
 *******************************************************************************/

#include <string.h>
#include "codefile.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"

#include "cvar.h"
#include "cexpr.h"
#include "cfeedback.h"

node *CFEEDBACKboxref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CFEEDBACKboxref");

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NULL, BOXDEF_NAME(BOXREF_BOX(arg_node)));// REALNAME

  DBUG_RETURN(arg_node);
}

node *CFEEDBACKboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CFEEDBACKboxsign");

  TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *CFEEDBACKnetrefs(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("CFEEDBACKnetrefs");

  net = NETREFS_NET(arg_node);

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));

  DBUG_RETURN(arg_node);
}

node *CFEEDBACKrectype(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CFEEDBACKrectype");

  CODEFILEwriteNext();

  CVARdoCode(arg_node);

  DBUG_RETURN(arg_node);
}

node *CFEEDBACKtypemap(node *arg_node, info *arg_info){
  DBUG_ENTER("CFEEDBACKtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL){
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}



node *CFEEDBACKdoCode(node *syntax_tree)
{
  DBUG_ENTER("CFEEDBACKdoCode");

  DBUG_ASSERT((syntax_tree != NULL), "CFEEDBACKdoCode called with empty syntaxtree");

  TRAVpush(TR_cfeedback);

  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

