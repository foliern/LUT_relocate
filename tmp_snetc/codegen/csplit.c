/*******************************************************************************
 *
 * $Id: csplit.c 3371 2012-02-13 15:32:29Z mvn $
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

#include "csplit.h"

/*
 * INFO structure
 */
struct INFO {
  char *range;
};


#define INFO_RANGE(n)        (n->range)

static void infoClear(info *inf)
{
  INFO_RANGE(inf)    = NULL;
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

node *CSPLITboxref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSPLITboxref");

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NULL, BOXDEF_NAME(BOXREF_BOX(arg_node)));// REALNAME

  DBUG_RETURN(arg_node);
}

node *CSPLITboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSPLITboxsign");

  TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *CSPLITbtagref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSPLITbtagref");

  INFO_RANGE(arg_info) = BTAGS_NAME(BTAGREF_BTAG(arg_node));

  DBUG_RETURN(arg_node);
}

node *CSPLITnetrefs(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("CSPLITnetrefs");

  net = NETREFS_NET(arg_node);

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));

  DBUG_RETURN(arg_node);
}

node *CSPLITrange(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSPLITrange");

  if(RANGE_STAGSTART(arg_node) != NULL) {
    TRAVdo(RANGE_STAGSTART(arg_node), arg_info);
    CODEFILEwriteNext();
    CODEFILEwriteStag(NULL, INFO_RANGE(arg_info));

    if(RANGE_STAGSTOP(arg_node) != NULL) {
      TRAVdo(RANGE_STAGSTOP(arg_node), arg_info);
      CODEFILEwriteNext();
      CODEFILEwriteStag(NULL, INFO_RANGE(arg_info));
    }
    else if(RANGE_BTAGSTOP(arg_node) != NULL) {
      TRAVdo(RANGE_BTAGSTOP(arg_node), arg_info);
      CODEFILEwriteNext();
      CODEFILEwriteBtag(NULL, INFO_RANGE(arg_info));
    }
    else {
      CODEFILEwriteNext();
      CODEFILEwriteStag(NULL, INFO_RANGE(arg_info));
    }
  }
  if(RANGE_BTAGSTART(arg_node) != NULL) {
    TRAVdo(RANGE_BTAGSTART(arg_node), arg_info);
    CODEFILEwriteNext();
    CODEFILEwriteBtag(NULL, INFO_RANGE(arg_info));

    if(RANGE_STAGSTOP(arg_node) != NULL) {
      TRAVdo(RANGE_STAGSTOP(arg_node), arg_info);
      CODEFILEwriteNext();
      CODEFILEwriteStag(NULL, INFO_RANGE(arg_info));
    }
    else if(RANGE_BTAGSTOP(arg_node) != NULL) {
      TRAVdo(RANGE_BTAGSTOP(arg_node), arg_info);
      CODEFILEwriteNext();
      CODEFILEwriteBtag(NULL, INFO_RANGE(arg_info));
    }
    else {
      CODEFILEwriteNext();
      CODEFILEwriteStag(NULL, INFO_RANGE(arg_info));
    }
  }

  DBUG_RETURN(arg_node);
}


node *CSPLITsplit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSPLITsplit");

  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();

  if (global.enable_dist_snet && SPLIT_ISDISTRIBUTED(arg_node)) {
    if(SPLIT_ISDETERM(arg_node)) {
      CODEFILEwriteSnetLocSplitDetStart();
    } else {
      CODEFILEwriteSnetLocSplitStart();
    }
  } else {
    if(SPLIT_ISDETERM(arg_node)) {
      CODEFILEwriteSnetSplitDetStart();
    } else {
      CODEFILEwriteSnetSplitStart();
    }
  }

  CODEFILEwriteLocation(SPLIT_LOCATION(arg_node));

  if(SPLIT_LEFT(arg_node) != NULL) {
    TRAVdo(SPLIT_LEFT(arg_node), arg_info);
  }

  if(SPLIT_RANGE(arg_node) != NULL) {
    TRAVdo(SPLIT_RANGE(arg_node), arg_info);
  }

  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();

  DBUG_RETURN(arg_node);
}

node *CSPLITstagref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSPLITstagref");

  INFO_RANGE(arg_info) = STAGS_NAME(STAGREF_STAG(arg_node));

  DBUG_RETURN(arg_node);
}

node *CSPLITtypemap(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSPLITtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL){
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CSPLITdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CSPLITdoCode");

  DBUG_ASSERT((syntax_tree != NULL), "CSPLITdoCode called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_csplit);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

