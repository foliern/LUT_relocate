/*******************************************************************************
 *
 * $Id: cstar.c 3371 2012-02-13 15:32:29Z mvn $
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
#include "cstar.h"

node *CSTARboxref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSTARboxref");

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NULL, BOXDEF_NAME(BOXREF_BOX(arg_node)));// REALNAME

  DBUG_RETURN(arg_node);
}

node *CSTARboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSTARboxsign");

  TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *CSTARnetrefs(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("CSTARnetrefs");

  net = NETREFS_NET(arg_node);

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));

  DBUG_RETURN(arg_node);
}

node *CSTARrectype(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSTARrectype");

  CODEFILEwriteNext();

  CVARdoCode(arg_node);

  DBUG_RETURN(arg_node);
}

node *CSTARtypemap(node *arg_node, info *arg_info){
  DBUG_ENTER("CSTARtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL){
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}


node *CSTARdoCode(node *syntax_tree)
{
  DBUG_ENTER("CSTARdoCode");

  DBUG_ASSERT((syntax_tree != NULL), "CSTARdoCode called with empty syntaxtree");

  TRAVpush(TR_cstar);

  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

