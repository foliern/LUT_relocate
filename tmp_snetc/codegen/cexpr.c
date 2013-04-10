/*******************************************************************************
 *
 * $Id: cexpr.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   11.01.2007
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

#include "cexpr.h"

node *CEXPRboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CEXPRboxsign");

  TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);
  
  DBUG_RETURN(arg_node);
}

node *CEXPRbtagref(node *arg_node, info *arg_info)
{
  node *btag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CEXPRbtagref");
  
  btag = BTAGREF_BTAG(arg_node);
  pkg = BTAGS_PKG(btag);

  if(pkg == NULL) {
    CODEFILEwriteBtag(NULL, BTAGS_NAME(btag));
  }
  else {
    CODEFILEwriteBtag(NETDEF_PKGNAME(pkg), BTAGS_NAME(btag));
  }
  
  DBUG_RETURN(arg_node);
}



node *CEXPRstagref(node *arg_node, info *arg_info)
{
  node *stag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CEXPRstagref");
  
  stag = STAGREF_STAG(arg_node);
  pkg = STAGS_PKG(stag);

  if(pkg == NULL) {
    CODEFILEwriteStag(NULL, STAGS_NAME(stag));
  }
  else {
    CODEFILEwriteStag(NETDEF_PKGNAME(pkg), STAGS_NAME(stag));
  }
 
  DBUG_RETURN(arg_node);
}

node *CEXPRtagexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CEXPRtagexpr");

  switch(TAGEXPR_OPERATOR(arg_node)){ 
  case OPER_NONE:
    if(TAGEXPR_BTAG(arg_node) != NULL) {
      CODEFILEwriteSnetEBtagStart();
      TRAVdo(TAGEXPR_BTAG(arg_node), arg_info);
      CODEFILEwriteFunctionStop();
    }
    else  if(TAGEXPR_STAG(arg_node) != NULL) {
      CODEFILEwriteSnetEStagStart();
      TRAVdo(TAGEXPR_STAG(arg_node), arg_info);
      CODEFILEwriteFunctionStop();
    }
    else {
      CODEFILEwriteSnetEconsti(TAGEXPR_INTEGERCONST(arg_node));
    }
    break;
  case OPER_NOT:
    CODEFILEwriteSnetEStart("not");
    break;
  case OPER_ABS:
    CODEFILEwriteSnetEStart("abs");
    break;
  case OPER_MUL:
    CODEFILEwriteSnetEStart("mul");
    break;
  case OPER_DIV:
    CODEFILEwriteSnetEStart("div");
    break;
  case OPER_REM:
    CODEFILEwriteSnetEStart("mod");
    break;
  case OPER_ADD:
    CODEFILEwriteSnetEStart("add");
    break;
  case OPER_SUB:
    CODEFILEwriteSnetEStart("sub");
    break;
  case OPER_MIN:
    CODEFILEwriteSnetEStart("min");
    break;
  case OPER_MAX:
    CODEFILEwriteSnetEStart("max");
    break;
  case OPER_EQ:
    CODEFILEwriteSnetEStart("eq");
    break;
  case OPER_NEQ:
    CODEFILEwriteSnetEStart("ne");
    break;
  case OPER_LT:
    CODEFILEwriteSnetEStart("lt");
    break;
  case OPER_LTQ:
    CODEFILEwriteSnetEStart("le");
    break;
  case OPER_GT:
    CODEFILEwriteSnetEStart("gt");
    break;
  case OPER_GTQ:
    CODEFILEwriteSnetEStart("ge");
    break;
  case OPER_AND:
    CODEFILEwriteSnetEStart("and");
    break;
  case OPER_OR:
    CODEFILEwriteSnetEStart("or");
    break;
  case OPER_COND:
    CODEFILEwriteSnetEStart("cond");
    break;
  default:
    break;
  }

  if(TAGEXPR_OPERATOR(arg_node) != OPER_NONE){

    if(TAGEXPR_CONDITION(arg_node) != NULL) {
      TRAVdo(TAGEXPR_CONDITION(arg_node), arg_info);
      CODEFILEwriteNext();
    }
    
    if(TAGEXPR_LEFT(arg_node) != NULL) {
      TRAVdo(TAGEXPR_LEFT(arg_node), arg_info);
      CODEFILEwriteNext();
    }
    
    if(TAGEXPR_RIGHT(arg_node) != NULL) {
      TRAVdo(TAGEXPR_RIGHT(arg_node), arg_info);
    }

    CODEFILEwriteFunctionStop();
  }

  DBUG_RETURN(arg_node);
}

node *CEXPRguardpatterns(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CEXPRguardpatterns");

  CODEFILEwriteNext();

  if(GUARDPATTERNS_CONDITION(arg_node) != NULL){
    TRAVdo(GUARDPATTERNS_CONDITION(arg_node), arg_info);
  } else {
    CODEFILEwriteSnetEconstbTrue();
  }

  if(GUARDPATTERNS_NEXT(arg_node) != NULL){
    TRAVdo(GUARDPATTERNS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CEXPRguardactions(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CEXPRguardactions");

  CODEFILEwriteNext();

  if(GUARDACTIONS_TAGEXPR(arg_node) != NULL){
    TRAVdo(GUARDACTIONS_TAGEXPR(arg_node), arg_info);
  } else {
    CODEFILEwriteSnetEconstbTrue();
  }
  if(GUARDACTIONS_NEXT(arg_node) != NULL){
    TRAVdo(GUARDACTIONS_NEXT(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}

node *CEXPRtypemap(node *arg_node, info *arg_info){
  DBUG_ENTER("CEXPRtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL){
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}


node *CEXPRdoCode(node *syntax_tree)
{
  DBUG_ENTER("CEXPRdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CEXPRdoCode called with empty syntaxtree");

  TRAVpush(TR_cexpr);

  syntax_tree = TRAVdo(syntax_tree, NULL);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

