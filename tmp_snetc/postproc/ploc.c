/*******************************************************************************
 *
 * $Id: ploc.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   05.12.2008
 * -----
 *
 *******************************************************************************/

#include "ploc.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "free.h"
#include "str.h"
#include "copy.h"
#include "str.h"

#define LOCATION_NONE    -1
#define LOCATION_UNKNOWN -2

/*
 * INFO structure
 */
struct INFO {
  int loc;
  node *top;
};

#define INFO_LOC(n)   (n->loc)
#define INFO_TOP(n)   (n->top)

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_LOC(result) = LOCATION_UNKNOWN; //0
  INFO_TOP(result) = NULL;

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *PPLOCat( node *arg_node, info *arg_info)
{
  DBUG_ENTER("PPLOCat");

  int loc;
  node *temp;
  node *top = NULL;
  node *ref;

  if(INFO_TOP(arg_info) != NULL) {
    top = INFO_TOP(arg_info);

    ref = TBmakeNetrefs(top, NULL);
    NODE_ERRCODE(ref) = STRcpy(NODE_ERRCODE(arg_node));

    NETREFS_LOCATION(ref) = AT_LOCATION(arg_node);

    temp = TBmakeNetbody(NULL, ref);

    NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(arg_node));

    temp = TBmakeNetdef(STRcpy(NETDEF_NAME(top)),
           NULL,
           FALSE, FALSE, NULL,
           temp,
           NULL);

    NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(arg_node));

    top = temp;

    INFO_TOP(arg_info) = NULL;
  }

  loc = INFO_LOC(arg_info);

  INFO_LOC(arg_info) = AT_LOCATION(arg_node);

  AT_LEFT(arg_node) = TRAVdo(AT_LEFT(arg_node), arg_info);

  INFO_LOC(arg_info) = loc;

  temp = AT_LEFT(arg_node);

  AT_LEFT(arg_node) = NULL;

  FREEdoFreeTree(arg_node);

  arg_node = temp;

  INFO_TOP(arg_info) = top;

  DBUG_RETURN(arg_node);
}

node *PPLOCboxref( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCboxref");

  INFO_TOP(arg_info) = NULL;

  if(BOXREF_LOCATION(arg_node) == LOCATION_NONE) {
    BOXREF_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCchoice( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCchoice");

  INFO_TOP(arg_info) = NULL;

  if(CHOICE_LOCATION(arg_node) == LOCATION_NONE) {
    CHOICE_LOCATION(arg_node) = INFO_LOC(arg_info);

/*
    CHOICE_LEFT(arg_node) = TRAVdo(CHOICE_LEFT(arg_node), arg_info);

    INFO_LOC(arg_info) = CHOICE_LOCATION(arg_node);

    CHOICE_RIGHT(arg_node) = TRAVdo(CHOICE_RIGHT(arg_node), arg_info);

    INFO_LOC(arg_info) = CHOICE_LOCATION(arg_node);
*/
    CHOICE_BRANCHLIST(arg_node) = TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCbranchlist( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCbranchlist");

  int loc = INFO_LOC(arg_info);
  BRANCHLIST_BRANCH(arg_node) = TRAVdo(BRANCHLIST_BRANCH(arg_node), arg_info);
  INFO_LOC(arg_info) = loc;

  if (BRANCHLIST_NEXT(arg_node) != NULL) {
    BRANCHLIST_NEXT(arg_node) = TRAVdo(BRANCHLIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCdefs( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("PPLOCdefs");

  if(DEFS_NEXT(arg_node) != NULL) {
    DEFS_NEXT(arg_node) = TRAVdo(DEFS_NEXT(arg_node), arg_info);
  }

  if(DEFS_DEF(arg_node) != NULL) {
    temp = TRAVdo(DEFS_DEF(arg_node), arg_info);

    if(temp != DEFS_DEF(arg_node)) {
      DEFS_NEXT(arg_node) = TBmakeDefs(temp, DEFS_NEXT(arg_node));
      NODE_ERRCODE(DEFS_NEXT(arg_node)) = STRcpy(NODE_ERRCODE(arg_node));
    }
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCfilt( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCfilt");

  INFO_TOP(arg_info) = NULL;

  if(FILT_LOCATION(arg_node) == LOCATION_NONE) {
    FILT_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCnetbody( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("PPLOCnetbody");


  if(NETBODY_CONNECT(arg_node) != NULL) {
    NETBODY_CONNECT(arg_node) = TRAVdo(NETBODY_CONNECT(arg_node), arg_info);
  }

  temp = INFO_TOP(arg_info);

  INFO_TOP(arg_info) = NULL;

  if(NETBODY_DEFS(arg_node) != NULL) {
    NETBODY_DEFS(arg_node) = TRAVdo(NETBODY_DEFS(arg_node), arg_info);
  }

  INFO_TOP(arg_info) = temp;

  DBUG_RETURN(arg_node);
}


node *PPLOCnetdef( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("PPLOCnetdef");

  INFO_TOP(arg_info) = NULL;

  if(NETDEF_TOPLEVEL(arg_node)) {
    INFO_TOP(arg_info) = arg_node;
  }

  if(NETDEF_BODY(arg_node) != NULL) {
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }

  if(INFO_TOP(arg_info) != NULL) {

    temp = INFO_TOP(arg_info);

    MEMfree(NETDEF_NAME(arg_node));

    NETDEF_NAME(arg_node) = STRcat(NETDEF_NAME(temp), "__LOC");

    NETDEF_SIGN(temp) = /*COPYdoCopyTree(*/NETDEF_SIGN(arg_node)/*)*/;
    NETDEF_SIGNED(temp) = NETDEF_SIGNED(arg_node);

    NETDEF_SIGN(arg_node) = NULL;
    NETDEF_SIGNED(arg_node) = FALSE;

    NETDEF_TOPLEVEL(temp) = TRUE;
    NETDEF_NOSHIELDS(temp) = TRUE;

    NETDEF_TOPLEVEL(arg_node) = FALSE;
    NETDEF_NOSHIELDS(arg_node) = TRUE;

    arg_node = temp;

    INFO_TOP(arg_info) = NULL;
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCnetrefs( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCnetrefs");

  INFO_TOP(arg_info) = NULL;

  if(NETREFS_LOCATION(arg_node) == LOCATION_NONE) {
    NETREFS_LOCATION(arg_node) = INFO_LOC(arg_info);

    NETREFS_NET(arg_node) = TRAVdo(NETREFS_NET(arg_node), arg_info);

    INFO_LOC(arg_info) = NETREFS_LOCATION(arg_node);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCserial( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCserial");

  INFO_TOP(arg_info) = NULL;

  if(SERIAL_LOCATION(arg_node) == LOCATION_NONE) {
    SERIAL_LOCATION(arg_node) = INFO_LOC(arg_info);

    SERIAL_LEFT(arg_node) = TRAVdo(SERIAL_LEFT(arg_node), arg_info);

    INFO_LOC(arg_info) = SERIAL_LOCATION(arg_node);

    SERIAL_RIGHT(arg_node) = TRAVdo(SERIAL_RIGHT(arg_node), arg_info);

    INFO_LOC(arg_info) = SERIAL_LOCATION(arg_node);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCsplit( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCsplit");

  INFO_TOP(arg_info) = NULL;

  if(SPLIT_LOCATION(arg_node) == LOCATION_NONE) {
    SPLIT_LOCATION(arg_node) = INFO_LOC(arg_info);

    if(SPLIT_ISDISTRIBUTED(arg_node)){
      INFO_LOC(arg_info) = LOCATION_UNKNOWN;
    } else {
      INFO_LOC(arg_info) = SPLIT_LOCATION(arg_node);
    }

    SPLIT_LEFT(arg_node) = TRAVdo(SPLIT_LEFT(arg_node), arg_info);

    INFO_LOC(arg_info) = SPLIT_LOCATION(arg_node);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCstar( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCstar");

  INFO_TOP(arg_info) = NULL;

  if(STAR_LOCATION(arg_node) == LOCATION_NONE) {
    STAR_LOCATION(arg_node) = INFO_LOC(arg_info);

    STAR_LEFT(arg_node) = TRAVdo(STAR_LEFT(arg_node), arg_info);

    INFO_LOC(arg_info) = STAR_LOCATION(arg_node);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCfeedback( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCfeedback");

  INFO_TOP(arg_info) = NULL;

  if(FEEDBACK_LOCATION(arg_node) == LOCATION_NONE) {
    FEEDBACK_LOCATION(arg_node) = INFO_LOC(arg_info);

    FEEDBACK_LEFT(arg_node) = TRAVdo(FEEDBACK_LEFT(arg_node), arg_info);

    INFO_LOC(arg_info) = FEEDBACK_LOCATION(arg_node);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCsync( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCsync");

  INFO_TOP(arg_info) = NULL;

  if(SYNC_LOCATION(arg_node) == LOCATION_NONE) {
    SYNC_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PPLOCtrans( node *arg_node, info *arg_info )
{
  DBUG_ENTER("PPLOCtrans");

  INFO_TOP(arg_info) = NULL;

  if(TRANS_LOCATION(arg_node) == LOCATION_NONE) {
    TRANS_LOCATION(arg_node) = INFO_LOC(arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PPdoLocate(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("PPdoLocate");

  DBUG_ASSERT((syntax_tree != NULL), "PPdoLocate called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_pploc);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}




