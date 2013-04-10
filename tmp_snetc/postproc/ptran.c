/*******************************************************************************
 *
 * $Id: ptran.c 2502 2009-08-03 11:30:16Z jju $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   12.02.2007
 * -----
 *
 *******************************************************************************/

#include "ptran.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "str.h"
#include "free.h"

typedef enum{TTunknown, TTpkgNetDef, TTbodyDefs, 
	     TTtransIn, TTtransInConnLeft,
             TTtransInConnRight, TTtransOut, 
	     TTtransOutConnLeft, TTtransOutConnRight, 
             TTmiddle, TTin}TranslationType;

/*
 * INFO structure
 */
struct INFO {
  TranslationType transType;
  node *pkg;
};

#define INFO_TRANSTYPE(n) (n->transType)
#define INFO_PKG(n)       (n->pkg)

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_TRANSTYPE(result) = TTunknown;
  INFO_PKG(result)       = NULL;

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *PPTRANbtagref(node *arg_node, info *arg_info)
{
  node *btag = NULL;
  node *new_node;

  DBUG_ENTER("PPTRANbtagref");
 
  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
  case TTtransIn:
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOut:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
  case TTmiddle:
  case TTin:
    btag = TRAVdo(BTAGREF_BTAG(arg_node), arg_info);
    new_node = TBmakeBtagref(btag, NULL);

    NODE_ERRCODE(new_node) = STRcpy(NODE_ERRCODE(arg_node));

    arg_node = new_node;
    break;
  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PPTRANbtags(node *arg_node, info *arg_info)
{
  node *next = NULL;
  node *btag = NULL;

  DBUG_ENTER("PPTRANbtags");

  next = BTAGS_NEXT(arg_node);

  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
    if((next != NULL) && STReq(BTAGS_NAME(arg_node), BTAGS_NAME(next))) {
      arg_node = next;
    }
    else {
      btag = TBmakeBtags(STRcpy(BTAGS_NAME(arg_node)), NULL,
                               INFO_PKG(arg_info), next);

      NODE_ERRCODE(btag) = STRcpy(NODE_ERRCODE(arg_node));

      BTAGS_NEXT(arg_node) = btag;
      arg_node = btag;
    }
    break;
  case TTtransIn:
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOut:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
  case TTmiddle:
  case TTin:
    if(INFO_PKG(arg_info) != NULL) {
      arg_node = next;
    }
    break;
  default:
    break;
  }
 
  DBUG_RETURN(arg_node);
}

node *PPTRANdefs(node *arg_node, info *arg_info)
{
  node *top = arg_node;
  TranslationType tempType;
  node *newDef = NULL;
  node *newDefs = NULL;
  
  DBUG_ENTER("PPTRANdefs");
  
  while(arg_node != NULL) {
    if(DEFS_DEF(arg_node) != NULL) {

      tempType = INFO_TRANSTYPE(arg_info);
      INFO_TRANSTYPE(arg_info) = TTunknown;

      newDef = TRAVdo(DEFS_DEF(arg_node), arg_info);
      
      if((newDef != NULL) && (newDef != DEFS_DEF(arg_node))) {
        newDefs = TBmakeDefs(newDef, arg_node);

	NODE_ERRCODE(newDefs) = STRcpy(NODE_ERRCODE(arg_node));

	
        if(top == arg_node) {
	  top = newDefs;
        }

        arg_node = newDefs;
      }
      
      INFO_TRANSTYPE(arg_info) = tempType;
      
    }
    arg_node = DEFS_NEXT(arg_node);
  }
  
  DBUG_RETURN(top);
}

node *PPTRANfieldref(node *arg_node, info *arg_info)
{
  node *field = NULL;
  node *new_node;

  DBUG_ENTER("PPTRANfieldref");

  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
  case TTtransIn:
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOut:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
  case TTmiddle:
  case TTin:

    field = TRAVdo(FIELDREF_FIELD(arg_node), arg_info);
    new_node = TBmakeFieldref(field, NULL);
    NODE_ERRCODE(new_node) = STRcpy(NODE_ERRCODE(arg_node));
    arg_node = new_node;
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *PPTRANfields(node *arg_node, info *arg_info)
{
  node *next = NULL;
  node *field = NULL;

  DBUG_ENTER("PPTRANfields");

  next = FIELDS_NEXT(arg_node);

  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
    if((next != NULL) && STReq(FIELDS_NAME(arg_node), FIELDS_NAME(next))) {
      arg_node = next;
    }
    else {
      field = TBmakeFields(STRcpy(FIELDS_NAME(arg_node)), NULL,
                                  INFO_PKG(arg_info), next);
      NODE_ERRCODE(field) = STRcpy(NODE_ERRCODE(arg_node));
      FIELDS_NEXT(arg_node) = field;
      arg_node = field;
    }
    break;
  case TTtransIn:
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOut:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
  case TTmiddle:
  case TTin:
    if(INFO_PKG(arg_info) != NULL) {
      arg_node = next;
    }
  break;
  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PPTRANnetdef(node *arg_node, info *arg_info)
{
  node *pkg = NULL;
  node *sign = NULL;
  node *connect = NULL;
  node *bodyDefs = NULL;
  node *leftBox = NULL;
  node *rightBox = NULL;
  node *newBody = NULL;
  node *translateIn = NULL;
  node *translateOut = NULL;
  node *middle = NULL;
  node *in = NULL;
  node *temp = NULL;
  node *tempLeft = NULL;
  node *tempRight = NULL;

  DBUG_ENTER("PPTRANnetdef");
  
  if(NETDEF_EXTERNAL(arg_node)) {
    /* Make new package net  */
    
    pkg = TBmakeNetdef(STRcpy(NETDEF_NAME(arg_node)), 
                       STRcpy(NETDEF_NAME(arg_node)), 
                       TRUE, TRUE, NULL, NULL, NULL);

    NODE_ERRCODE(pkg) = STRcpy(NODE_ERRCODE(arg_node));
    
    INFO_TRANSTYPE(arg_info) = TTpkgNetDef;
    INFO_PKG(arg_info) = pkg;
    if(NETDEF_SIGN(arg_node) != NULL) {
      NETDEF_SIGN(pkg) = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
    }
    
    /* Make bodydefs for old external net */
    bodyDefs = TBmakeDefs(NULL, NULL);

    NODE_ERRCODE(bodyDefs) = STRcpy(NODE_ERRCODE(arg_node));

    if(NETDEF_BODY(arg_node) != NULL) {
      FREEdoFreeTree(NETDEF_BODY(arg_node));
    }

    NETDEF_BODY(arg_node) = TBmakeNetbody(bodyDefs, NULL);
    
    NODE_ERRCODE(NETDEF_BODY(arg_node)) = STRcpy(NODE_ERRCODE(arg_node));

    /* Make translate in net */ 
    INFO_TRANSTYPE(arg_info) = TTtransIn;

    if(NETDEF_SIGN(arg_node) != NULL) {
      sign = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
      INFO_TRANSTYPE(arg_info)   = TTtransInConnLeft;
      leftBox = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
      INFO_TRANSTYPE(arg_info)   = TTtransInConnRight;
      rightBox = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
      connect = TBmakeTrans(leftBox, rightBox);
      NODE_ERRCODE(connect) = STRcpy(NODE_ERRCODE(arg_node));
    }
    
    newBody = TBmakeNetbody(NULL, connect);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    translateIn = TBmakeNetdef(STRcpy("translate_in"), NULL, FALSE, 
			       TRUE, sign, newBody, NULL);
    NODE_ERRCODE(translateIn) = STRcpy(NODE_ERRCODE(arg_node));


    NETDEF_NOSHIELDS(translateIn) = TRUE;
 
    DEFS_DEF(bodyDefs) = translateIn;
    DEFS_NEXT(bodyDefs) = TBmakeDefs(NULL, NULL);
    NODE_ERRCODE(DEFS_NEXT(bodyDefs)) = STRcpy(NODE_ERRCODE(arg_node));

    bodyDefs = DEFS_NEXT(bodyDefs);
    
    /* Make translate out net */
    INFO_TRANSTYPE(arg_info) = TTtransOut;
    sign    = NULL;
    connect = NULL;
    newBody = NULL;
    if(NETDEF_SIGN(arg_node) != NULL) {
      sign =TRAVdo(NETDEF_SIGN(arg_node), arg_info);
      INFO_TRANSTYPE(arg_info)   = TTtransOutConnLeft;
      leftBox = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
      INFO_TRANSTYPE(arg_info)   = TTtransOutConnRight;
      rightBox = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
      connect = TBmakeTrans(leftBox, rightBox);
      NODE_ERRCODE(connect) = STRcpy(NODE_ERRCODE(arg_node));
    }

    newBody = TBmakeNetbody(NULL, connect);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));
    
    translateOut = TBmakeNetdef(STRcpy("translate_out"), NULL, FALSE,   
				TRUE, sign, newBody, NULL);
    NODE_ERRCODE(translateOut) = STRcpy(NODE_ERRCODE(arg_node));

    NETDEF_NOSHIELDS(translateOut) = TRUE;
    
    DEFS_DEF(bodyDefs) = translateOut;
    DEFS_NEXT(bodyDefs) = TBmakeDefs(NULL, NULL);
    NODE_ERRCODE(DEFS_NEXT(bodyDefs)) = STRcpy(NODE_ERRCODE(arg_node));
    bodyDefs = DEFS_NEXT(bodyDefs);
    
    /* Make middle net */ 
    INFO_TRANSTYPE(arg_info) = TTmiddle;
    sign = NULL;
    newBody = NULL;
    if(NETDEF_SIGN(arg_node) != NULL) {
      sign = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
    }

    temp = TBmakeNetrefs(pkg, NULL);
    NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(arg_node));
    newBody = TBmakeNetbody(NULL, temp);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));


    middle = TBmakeNetdef(STRcpy(NETDEF_NAME(arg_node)), NULL, FALSE, 
			  TRUE, sign, newBody, NULL);
    NODE_ERRCODE(middle) = STRcpy(NODE_ERRCODE(arg_node));
    NETDEF_NOSHIELDS(middle) = TRUE;
    
    DEFS_DEF(bodyDefs) = middle;
    DEFS_NEXT(bodyDefs) = TBmakeDefs(NULL, NULL);
    NODE_ERRCODE(DEFS_NEXT(bodyDefs)) = STRcpy(NODE_ERRCODE(arg_node));
    bodyDefs = DEFS_NEXT(bodyDefs);   


    /* Make in net */ 
    INFO_TRANSTYPE(arg_info) = TTin;
    sign = NULL;
    newBody = NULL;
    if(NETDEF_SIGN(arg_node) != NULL) {
      sign = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
    }

    tempLeft = TBmakeNetrefs(translateIn, NULL);
    NODE_ERRCODE(tempLeft) = STRcpy(NODE_ERRCODE(arg_node));

    tempRight = TBmakeNetrefs(middle, NULL);
    NODE_ERRCODE(tempRight) = STRcpy(NODE_ERRCODE(arg_node));

    temp = TBmakeSerial(tempLeft, tempRight);
    NODE_ERRCODE(temp) = STRcpy(NODE_ERRCODE(arg_node));

    newBody = TBmakeNetbody(NULL, temp);
    NODE_ERRCODE(newBody) = STRcpy(NODE_ERRCODE(arg_node));

    
    in = TBmakeNetdef(STRcpy("in"), NULL, FALSE, TRUE, sign, newBody, NULL);
    NODE_ERRCODE(in) = STRcpy(NODE_ERRCODE(arg_node));
    NETDEF_NOSHIELDS(in) = TRUE;

    DEFS_DEF(bodyDefs) = in;
    
    /* Set old external net to internal and make connect node */
    tempLeft = TBmakeNetrefs(in, NULL);
    NODE_ERRCODE(tempLeft) = STRcpy(NODE_ERRCODE(arg_node));

    tempRight = TBmakeNetrefs(translateOut, NULL);
    NODE_ERRCODE(tempRight) = STRcpy(NODE_ERRCODE(arg_node));

    NETBODY_CONNECT(NETDEF_BODY(arg_node)) = TBmakeSerial(tempLeft, tempRight);
    NODE_ERRCODE(NETBODY_CONNECT(NETDEF_BODY(arg_node))) = STRcpy(NODE_ERRCODE(arg_node));

    NETDEF_EXTERNAL(arg_node) = FALSE;
    NETDEF_NOSHIELDS(arg_node) = TRUE;

    arg_node = pkg;
  }
  
  DBUG_RETURN(arg_node);
}

node *PPTRANrecentries(node *arg_node, info *arg_info)
{
  node *recentries = NULL;
  
  DBUG_ENTER("PPTRANrecentries");
  
  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:  
  case TTtransIn:  
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOut:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
  case TTmiddle:
  case TTin:
    recentries = TBmakeRecentries(NULL, NULL, NULL, NULL);
    NODE_ERRCODE(recentries) = STRcpy(NODE_ERRCODE(arg_node));

    if(RECENTRIES_FIELD(arg_node) != NULL) {
      RECENTRIES_FIELD(recentries) = TRAVdo(RECENTRIES_FIELD(arg_node), arg_info);
    }
    else if(RECENTRIES_STAG(arg_node) != NULL) {
      RECENTRIES_STAG(recentries) = TRAVdo(RECENTRIES_STAG(arg_node), arg_info);
    }
    else if(RECENTRIES_BTAG(arg_node) != NULL) {
      RECENTRIES_BTAG(recentries) = TRAVdo(RECENTRIES_BTAG(arg_node), arg_info);
    }
    
    if(RECENTRIES_NEXT(arg_node) != NULL) {
      RECENTRIES_NEXT(recentries) = TRAVdo(RECENTRIES_NEXT(arg_node), arg_info);
    }
    arg_node = recentries;
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *PPTRANrectype(node *arg_node, info *arg_info)
{
  node *rectype = NULL;

  DBUG_ENTER("PPTRANrectype");

  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
  case TTtransIn:
  case TTtransOut:
  case TTmiddle:
  case TTin:
    rectype = TBmakeRectype(NULL);
    NODE_ERRCODE(rectype) = STRcpy(NODE_ERRCODE(arg_node));

    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      RECTYPE_ENTRIES(rectype) = TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }
    arg_node = rectype;
    break;
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      arg_node = TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }    
    break;
  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PPTRANstagref(node *arg_node, info *arg_info)
{
  node *stag = NULL;
  node *new_node;

  DBUG_ENTER("PPTRANstagref");
 
  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
  case TTtransIn:
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOut:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
  case TTmiddle:
  case TTin:
    stag = TRAVdo(STAGREF_STAG(arg_node), arg_info);
    new_node = TBmakeStagref(stag, NULL);
    NODE_ERRCODE(new_node) = STRcpy(NODE_ERRCODE(arg_node));
    arg_node = new_node;
    break;
  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PPTRANstags(node *arg_node, info *arg_info)
{
  node *next = NULL;
  node *stag = NULL;

  DBUG_ENTER("PPTRANstags");

  next = STAGS_NEXT(arg_node);
 
  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
    if((next != NULL) && STReq(STAGS_NAME(arg_node), STAGS_NAME(next))) {
      arg_node = next;
    }
    else {
      stag = TBmakeStags(STRcpy(STAGS_NAME(arg_node)), NULL, 
                               INFO_PKG(arg_info), next);
      NODE_ERRCODE(stag) = STRcpy(NODE_ERRCODE(arg_node));
      STAGS_NEXT(arg_node) = stag;
      arg_node = stag;
    }
    break;
  case TTtransIn:
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOut:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
  case TTmiddle:
  case TTin:
    if(INFO_PKG(arg_info) != NULL) {
      arg_node = next;
    }
    break;
  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *PPTRANtypes(node *arg_node, info *arg_info)
{
  node *types = NULL;

  DBUG_ENTER("PPTRANtypes");

  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
  case TTtransIn:
  case TTtransOut:
  case TTmiddle:
  case TTin:
    types = TBmakeTypes(NULL, NULL);
    NODE_ERRCODE(types) = STRcpy(NODE_ERRCODE(arg_node));

    if(TYPES_TYPE(arg_node) != NULL) {
      TYPES_TYPE(types) = TRAVdo(TYPES_TYPE(arg_node), arg_info);
    }

    if(TYPES_NEXT(arg_node) != NULL) {
      TYPES_NEXT(types) = TRAVdo(TYPES_NEXT(arg_node), arg_info);
    }

    arg_node = types;
    break;
  case TTtransInConnLeft:
  case TTtransInConnRight:
  case TTtransOutConnLeft:
  case TTtransOutConnRight:
    if(TYPES_TYPE(arg_node) != NULL) {
      arg_node = TRAVdo(TYPES_TYPE(arg_node), arg_info);
    }
    break;
  default:
    break;
  }
  DBUG_RETURN(arg_node);
}

node *PPTRANtypemap(node *arg_node, info *arg_info)
{
  node *pkg        = INFO_PKG(arg_info);
  node *recentries = NULL;
  node *inType     = NULL;
  node *outType    = NULL;
  node *typemaps   = NULL;
  node *boxtypes   = NULL;

  DBUG_ENTER("PPTRANtypemap");
  
  switch(INFO_TRANSTYPE(arg_info)) {
  case TTpkgNetDef:
    inType   = TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    outType  = TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    typemaps = TBmakeTypemap(TYPEMAP_OUTTYPETOINFER(arg_node), inType, outType);
    NODE_ERRCODE(typemaps) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTtransIn:
    outType  = TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    INFO_PKG(arg_info) = NULL;
    inType   = TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    typemaps = TBmakeTypemap(TYPEMAP_OUTTYPETOINFER(arg_node), inType, outType);
    NODE_ERRCODE(typemaps) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTtransInConnLeft:
    INFO_PKG(arg_info) = NULL;
    recentries = TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    boxtypes   = TBmakeBoxtypes(recentries, NULL);
    NODE_ERRCODE(boxtypes) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTtransInConnRight:
    recentries = TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    boxtypes   = TBmakeBoxtypes(recentries, NULL);
    NODE_ERRCODE(boxtypes) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTtransOut:
    inType   = TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    INFO_PKG(arg_info) = NULL;
    outType  = TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    typemaps = TBmakeTypemap(TYPEMAP_OUTTYPETOINFER(arg_node), inType, outType);
    NODE_ERRCODE(typemaps) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTtransOutConnLeft:
    recentries = TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    boxtypes   = TBmakeBoxtypes(recentries, NULL);
    NODE_ERRCODE(boxtypes) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTtransOutConnRight:
    INFO_PKG(arg_info) = NULL;
    recentries = TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    boxtypes   = TBmakeBoxtypes(recentries, NULL);
    NODE_ERRCODE(boxtypes) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTmiddle:
    inType   = TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    outType  = TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    typemaps = TBmakeTypemap(TYPEMAP_OUTTYPETOINFER(arg_node), inType, outType);
    NODE_ERRCODE(typemaps) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  case TTin:
    outType  = TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    INFO_PKG(arg_info) = NULL;
    inType   = TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    typemaps = TBmakeTypemap(TYPEMAP_OUTTYPETOINFER(arg_node), inType, outType);
    NODE_ERRCODE(typemaps) = STRcpy(NODE_ERRCODE(arg_node));
    break;
  default:
    break;
  }
  
  INFO_PKG(arg_info) = pkg;

  if(typemaps != NULL) {
    arg_node = typemaps;
  }
  else if(boxtypes != NULL) {
    arg_node = boxtypes;
  }

  
  DBUG_RETURN(arg_node);
}

node *PPTRANtypesigns(node *arg_node, info *arg_info)
{
  node *temp = NULL;
  node *tSigns = NULL;
  
  DBUG_ENTER("PPTRANtypesigns");
  
  if(TYPESIGNS_TYPESIG(arg_node) != NULL) {
    temp = TRAVdo(TYPESIGNS_TYPESIG(arg_node), arg_info);
    switch(INFO_TRANSTYPE(arg_info)) {
    case TTpkgNetDef:
    case TTtransIn:
    case TTtransOut:
    case TTmiddle:
    case TTin:
      /* if typemap returned */
      tSigns = TBmakeTypesigns(temp, NULL); 
      NODE_ERRCODE(tSigns) = STRcpy(NODE_ERRCODE(arg_node));
      
      if(TYPESIGNS_NEXT(arg_node) != NULL) {
      	TYPESIGNS_NEXT(tSigns) = TRAVdo(TYPESIGNS_NEXT(arg_node), arg_info);
      }
      arg_node = tSigns;
      
      break;
    case TTtransInConnLeft:
    case TTtransInConnRight:
    case TTtransOutConnLeft:
    case TTtransOutConnRight:
      
      /* if boxtype returned */
      arg_node = temp;

      break;
    default:
      break;
    }
  }

  DBUG_RETURN(arg_node);
}

node *PPdoTranslation(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("PPdoTranslation");
  
  DBUG_ASSERT((syntax_tree != NULL), "PPdoTranslation called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_pptran);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
