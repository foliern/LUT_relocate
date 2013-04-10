/*******************************************************************************
 *
 * $Id: cserial.c 2176 2008-12-09 09:38:19Z jju $
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

#include "cserial.h"

node *CSERIALboxref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSERIALboxref");  

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NULL, BOXDEF_NAME(BOXREF_BOX(arg_node)));// REALNAME
  
  DBUG_RETURN(arg_node);
}

node *CSERIALnetrefs(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("CSERIALnetrefs");
  
  net = NETREFS_NET(arg_node);

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
  
  DBUG_RETURN(arg_node);
}


node *CSERIALserial(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSERIALserial");
   
  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();
  CODEFILEwriteSnetSerialStart();
  CODEFILEwriteLocation(SERIAL_LOCATION(arg_node));

  if(SERIAL_LEFT(arg_node) != NULL) {
    TRAVdo(SERIAL_LEFT(arg_node), arg_info);
  }

  if(SERIAL_RIGHT(arg_node) != NULL) {
    TRAVdo(SERIAL_RIGHT(arg_node), arg_info);
  }

  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();

  DBUG_RETURN(arg_node);
}


node *CSERIALdoCode(node *syntax_tree)
{
  DBUG_ENTER("CSERIALdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CSERIALdoCode called with empty syntaxtree");

  TRAVpush(TR_cserial);

  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

