/*******************************************************************************
 *
 * $Id: flat.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   19.02.2007
 * -----
 *
 *******************************************************************************/

#include <stdio.h>
#include "flat.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "str.h"
#include "free.h"

typedef enum{
  TTunknown,
  TTchoice,
  TTserialLeft,
  TTserialRight,
  TTsplit,
  TTstar,
  TTfeedback,
  TTsigned
}TravelType;

/*
 * INFO structure
 */
struct INFO {
  TravelType travelType;
  int branchIndex;
  node *netBody;
  bool inSignedNet;
};

#define INFO_TRAVELTYPE(n) (n->travelType)
#define INFO_BRANCHINDEX(n) (n->branchIndex)
#define INFO_NETBODY(n)    (n->netBody)
#define INFO_INSIGNEDNET(n) (n->inSignedNet)

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_TRAVELTYPE(result)  = TTunknown;
  INFO_BRANCHINDEX(result) = 0;
  INFO_NETBODY(result)     = NULL;
  INFO_INSIGNEDNET(result) = FALSE;

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

static void addDefs(node *netBody, node *newDefs)
{
  node *bodyDefs = NETBODY_DEFS(netBody);

  if(bodyDefs == NULL) {
    NETBODY_DEFS(netBody) = newDefs;
  } else {
    while(DEFS_NEXT(bodyDefs) != NULL) {
      bodyDefs = DEFS_NEXT(bodyDefs);
    }
    DEFS_NEXT(bodyDefs) = newDefs;
  }
}

static char tmpName[10];

static char *getNetName(info *inf)
{
  char *netName = NULL;

  switch(INFO_TRAVELTYPE(inf)) {
  case TTchoice:
    snprintf(tmpName, 10, "_P%d", INFO_BRANCHINDEX(inf));
    netName = STRcpy(tmpName);
    break;
  case TTserialLeft:
    netName = STRcpy("_SL");
    break;
  case TTserialRight:
    netName = STRcpy("_SR");
    break;
  case TTsplit:
    netName = STRcpy("_IS");
    break;
  case TTstar:
    netName = STRcpy("_ST");
    break;
  case TTfeedback:
    netName = STRcpy("_FB");
    break;
  case TTsigned:
    netName = STRcpy("_IN");
  default:
    break;
  }

  return netName;
}


node *FLATnetrefs(node *arg_node, info *arg_info)
{
  node *newNet = NULL;
  node *newBody = NULL;
  node *newDef = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATnetrefs");

  switch(INFO_TRAVELTYPE(arg_info)) {
   
    case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                          FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}


node *FLATat(node *arg_node, info *arg_info)
{
  node *body    = NULL;
  node *newBody = NULL;
  node *newNet  = NULL;
  node *newDef  = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATat");

  body = INFO_NETBODY(arg_info);

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTsigned:
    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet  = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                           FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef  = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    INFO_NETBODY(arg_info) = newBody;

    INFO_TRAVELTYPE(arg_info) = TTunknown;
    AT_LEFT(arg_node) = TRAVdo(AT_LEFT(arg_node), arg_info);

    INFO_NETBODY(arg_info) = body;

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = newExpr;

    break;

  default:
    AT_LEFT(arg_node) = TRAVdo(AT_LEFT(arg_node), arg_info);

    break;
  }

  DBUG_RETURN(arg_node);
}

node *FLATboxref(node *arg_node, info *arg_info)
{
  node *newNet = NULL;
  node *newBody = NULL;
  node *newDef = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATboxref");

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                          FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *FLATchoice(node *arg_node, info *arg_info)
{
  node *body    = NULL;
  node *newBody = NULL;
  node *newNet  = NULL;
  node *newDef  = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATchoice");

  body    = INFO_NETBODY(arg_info);

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTunknown:

    INFO_TRAVELTYPE(arg_info) = TTchoice;
    INFO_BRANCHINDEX(arg_info) = 1;
    CHOICE_BRANCHLIST(arg_node) = TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);

    break;

  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet  = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                           FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef  = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    INFO_NETBODY(arg_info) = newBody;

    INFO_BRANCHINDEX(arg_info) = 1;
    CHOICE_BRANCHLIST(arg_node) = TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);

    INFO_NETBODY(arg_info) = body;

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = newExpr;

    break;
  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *FLATbranchlist(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FLATbranchlist");

  INFO_TRAVELTYPE(arg_info) = TTchoice;
  int index = INFO_BRANCHINDEX(arg_info);
  BRANCHLIST_BRANCH(arg_node) = TRAVdo(BRANCHLIST_BRANCH(arg_node), arg_info);

  if (BRANCHLIST_NEXT(arg_node) != NULL) {
    INFO_BRANCHINDEX(arg_info) = index + 1;
    BRANCHLIST_NEXT(arg_node) = TRAVdo(BRANCHLIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *FLATfilt(node *arg_node, info *arg_info)
{
  node *newNet = NULL;
  node *newBody = NULL;
  node *newDef = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATfilt");

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                          FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *FLATnetbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FLATnetbody");

  bool inSignedNet = INFO_INSIGNEDNET(arg_info);

  if(NETBODY_DEFS(arg_node) != NULL) {
    NETBODY_DEFS(arg_node) = TRAVdo(NETBODY_DEFS(arg_node), arg_info);
  }

  if(NETBODY_CONNECT(arg_node) != NULL) {
    INFO_TRAVELTYPE(arg_info) = inSignedNet ? TTsigned : TTunknown;
    INFO_NETBODY(arg_info) = arg_node;

    NETBODY_CONNECT(arg_node) = TRAVdo(NETBODY_CONNECT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *FLATnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FLATnetdef");

  if(NETDEF_BODY(arg_node) != NULL) {
    INFO_INSIGNEDNET(arg_info) = (NETDEF_SIGNED(arg_node)
      && !NETDEF_TOPLEVEL(arg_node)) 
      || NETDEF_TOPLEVEL(arg_node); /* always make _IN wrapper for toplevel */
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  } else {
    NETDEF_BODY(arg_node) = TBmakeNetbody(NULL, NULL);
    NODE_ERRCODE(NETDEF_BODY(arg_node)) = STRcpy(NODE_ERRCODE(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *FLATserial(node *arg_node, info *arg_info)
{
  node *body    = NULL;
  node *newBody = NULL;
  node *newNet  = NULL;
  node *newDef  = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATserial");

  body    = INFO_NETBODY(arg_info);

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTunknown:

    INFO_TRAVELTYPE(arg_info) = TTserialLeft;
    SERIAL_LEFT(arg_node) = TRAVdo(SERIAL_LEFT(arg_node), arg_info);

    INFO_TRAVELTYPE(arg_info) = TTserialRight;
    SERIAL_RIGHT(arg_node) = TRAVdo(SERIAL_RIGHT(arg_node), arg_info);
    break;

  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:


    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet  = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                           FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef  = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    INFO_NETBODY(arg_info) = newBody;

    INFO_TRAVELTYPE(arg_info) = TTserialLeft;
    SERIAL_LEFT(arg_node) = TRAVdo(SERIAL_LEFT(arg_node), arg_info);

    INFO_TRAVELTYPE(arg_info) = TTserialRight;
    SERIAL_RIGHT(arg_node) = TRAVdo(SERIAL_RIGHT(arg_node), arg_info);


    INFO_NETBODY(arg_info) = body;

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *FLATsplit(node *arg_node, info *arg_info)
{
  node *body    = NULL;
  node *newBody = NULL;
  node *newNet  = NULL;
  node *newDef  = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATsplit");

  body = INFO_NETBODY(arg_info);

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTunknown:
    INFO_TRAVELTYPE(arg_info) = TTsplit;
    SPLIT_LEFT(arg_node) = TRAVdo(SPLIT_LEFT(arg_node), arg_info);
    break;

  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet  = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                           FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef  = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    INFO_NETBODY(arg_info) = newBody;

    INFO_TRAVELTYPE(arg_info) = TTsplit;
    SPLIT_LEFT(arg_node) = TRAVdo(SPLIT_LEFT(arg_node), arg_info);

    INFO_NETBODY(arg_info) = body;

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));
    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}



node *FLATstar(node *arg_node, info *arg_info)
{
  node *body    = NULL;
  node *newBody = NULL;
  node *newNet  = NULL;
  node *newDef  = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATstar");

  body    = INFO_NETBODY(arg_info);

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTunknown:
    INFO_TRAVELTYPE(arg_info) = TTstar;
    STAR_LEFT(arg_node) = TRAVdo(STAR_LEFT(arg_node), arg_info);
    break;

  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet  = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                           FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef  = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    INFO_NETBODY(arg_info) = newBody;

    INFO_TRAVELTYPE(arg_info) = TTstar;
    STAR_LEFT(arg_node) = TRAVdo(STAR_LEFT(arg_node), arg_info);

    INFO_NETBODY(arg_info) = body;

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));
    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}



node *FLATfeedback(node *arg_node, info *arg_info)
{
  node *body    = NULL;
  node *newBody = NULL;
  node *newNet  = NULL;
  node *newDef  = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATfeedback");

  body    = INFO_NETBODY(arg_info);

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTunknown:
    INFO_TRAVELTYPE(arg_info) = TTfeedback;
    FEEDBACK_LEFT(arg_node) = TRAVdo(FEEDBACK_LEFT(arg_node), arg_info);
    break;

  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet  = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                           FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef  = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    INFO_NETBODY(arg_info) = newBody;

    INFO_TRAVELTYPE(arg_info) = TTfeedback;
    FEEDBACK_LEFT(arg_node) = TRAVdo(FEEDBACK_LEFT(arg_node), arg_info);

    INFO_NETBODY(arg_info) = body;

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));
    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}



node *FLATsync(node *arg_node, info *arg_info)
{
  node *newNet = NULL;
  node *newBody = NULL;
  node *newDef = NULL;
  node *newExpr = NULL;

  DBUG_ENTER("FLATsync");

  switch(INFO_TRAVELTYPE(arg_info)) {
  case TTchoice:
  case TTserialLeft:
  case TTserialRight:
  case TTsplit:
  case TTstar:
  case TTfeedback:
  case TTsigned:

    newBody = TBmakeNetbody(NULL, arg_node);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    newNet = TBmakeNetdef(getNetName(arg_info), NULL, FALSE,
                          FALSE, NULL, newBody, NULL);
    NODE_ERRCODE(newNet) = STRcpy(NODE_ERRCODE(arg_node));

    newDef = TBmakeDefs(newNet, NULL);
    NODE_ERRCODE(newDef) = STRcpy(NODE_ERRCODE(arg_node));

    addDefs(INFO_NETBODY(arg_info), newDef);

    newExpr = TBmakeNetrefs(newNet, NULL);
    NODE_ERRCODE(newExpr) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = newExpr;

    break;

  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *FLATdoFlat(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("FLATdoFlat");

  DBUG_ASSERT((syntax_tree != NULL), "FLATdoFlat called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_flat);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
