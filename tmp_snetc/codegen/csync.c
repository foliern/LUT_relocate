/*******************************************************************************
 *
 * $Id: csync.c 3371 2012-02-13 15:32:29Z mvn $
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
#include "csync.h"

static unsigned int guardpatternSize(node *patterns)
{
  unsigned int size = 0;

  while(patterns != NULL) {
    size++;
    patterns = GUARDPATTERNS_NEXT(patterns);
  }

  return size;
}

node *CSYNCrectype(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSYNCrectype");
  
  CODEFILEwriteNext();
  CVARdoCode(arg_node);

  DBUG_RETURN(arg_node);
}

node *CSYNCsync(node *arg_node, info *arg_info)
{
  unsigned int patternNum;

  DBUG_ENTER("CSYNCsync");

  patternNum = guardpatternSize(SYNC_AUXPATTERNS(arg_node)) + 1;

  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();
  CODEFILEwriteSnetSyncStart();
  CODEFILEwriteLocation(SYNC_LOCATION(arg_node));
  CODEFILEwriteNext();

  CODEFILEwriteVariantListStart(patternNum);

  TRAVdo(SYNC_MAINPATTERN(arg_node), arg_info);
  if(SYNC_AUXPATTERNS(arg_node) != NULL) {
    TRAVdo(SYNC_AUXPATTERNS(arg_node), arg_info);
  }
  CODEFILEwriteFunctionStop();
  CODEFILEwriteNext();

  CODEFILEwriteExprListStart(patternNum);
  CEXPRdoCode(SYNC_MAINPATTERN(arg_node));
  if (SYNC_AUXPATTERNS(arg_node) != NULL) {
    CEXPRdoCode(SYNC_AUXPATTERNS(arg_node));
  }

  CODEFILEwriteFunctionStop();

  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();

  DBUG_RETURN(arg_node);
}

node *CSYNCtypemap(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSYNCtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL) {
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CSYNCdoCode(node *syntax_tree)
{
  DBUG_ENTER("CSYNCdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CSYNCdoCode called with empty syntaxtree");

  TRAVpush(TR_csync);

  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

