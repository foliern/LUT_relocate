/*******************************************************************************
 *
 * $Id: sploc.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   05.12.2008
 * -----
 *
 *******************************************************************************/

#include "sploc.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "free.h"
#include "str.h"
#include "copy.h"

#define LOCATION_NONE -1

/*
 * INFO structure
 */
struct INFO {
  int loc;
};

#define INFO_LOC(n)   (n->loc)

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_LOC(result) = 0;

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *SPLOCat( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("SPLOCat");

  INFO_LOC(arg_info) = AT_LOCATION(arg_node);

  AT_LEFT(arg_node) = TRAVdo(AT_LEFT(arg_node), arg_info);

  temp = AT_LEFT(arg_node);

  AT_LEFT(arg_node) = NULL;

  FREEdoFreeTree(arg_node);

  arg_node = temp;

  DBUG_RETURN(arg_node);
}

node *SPLOCboxref( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCboxref");

  if(BOXREF_LOCATION(arg_node) == LOCATION_NONE) {
    BOXREF_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCchoice( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCchoice");

  if(CHOICE_LOCATION(arg_node) == LOCATION_NONE) {
    CHOICE_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCfilt( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCfilt");

  if(FILT_LOCATION(arg_node) == LOCATION_NONE) {
    FILT_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCnetrefs( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCnetrefs");

  if(NETREFS_LOCATION(arg_node) == LOCATION_NONE) {
    NETREFS_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCserial( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCserial");

  if(SERIAL_LOCATION(arg_node) == LOCATION_NONE) {
    SERIAL_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCsplit( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCsplit");

  if(SPLIT_LOCATION(arg_node) == LOCATION_NONE) {
    SPLIT_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCstar( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCstar");

  if(STAR_LOCATION(arg_node) == LOCATION_NONE) {
    STAR_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCfeedback( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCfeedback");

  if(FEEDBACK_LOCATION(arg_node) == LOCATION_NONE) {
    FEEDBACK_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCsync( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCsync");

  if(SYNC_LOCATION(arg_node) == LOCATION_NONE) {
    SYNC_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPLOCtrans( node *arg_node, info *arg_info )
{
  DBUG_ENTER("SPLOCtrans");

  if(TRANS_LOCATION(arg_node) == LOCATION_NONE) {
    TRANS_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *SPdoLocate(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("PPdoLocate");

  DBUG_ASSERT((syntax_tree != NULL), "SPdoLocate called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_sploc);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}




