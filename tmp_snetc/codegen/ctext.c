/*******************************************************************************
 *
 * $Id: ctext.c 3576 2012-07-31 12:56:35Z fpz $
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
#include "mutil.h"

#include "ctext.h"

typedef enum{CTextInclude, CTboxDef, CTboxAssign, CTboxComp} CodeType;

/*
 * INFO structure
 */

struct INFO {  
  CodeType codeType;
  node *def;
  lblqual qualifier;

};


#define INFO_CODETYPE(n)     (n->codeType)
#define INFO_DEF(n)          (n->def)
#define INFO_QUALIFIER(n)    (n->qualifier)

static void infoClear(info *inf)
{
  INFO_CODETYPE(inf) = CTextInclude;
  INFO_DEF(inf)      = NULL;
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

node *CTEXTboxdef(node *arg_node, info *arg_info)
{
  const char *name = NULL;

  DBUG_ENTER("CTEXTboxdef");
  
  INFO_CODETYPE(arg_info) = CTboxDef;
  
  if( BOXDEF_WRAPCODE( arg_node) != NULL) {
    CODEFILEwriteBoxWrap( BOXDEF_WRAPCODE( arg_node));
  }

  CODEFILEwriteNewline();

  CODEFILEwriteBoxStart(BOXDEF_NAME(arg_node));
 
  CODEFILEwriteRecDecl();
  
  INFO_DEF(arg_info) = arg_node;
  INFO_CODETYPE(arg_info) = CTboxDef;

  if(BOXDEF_SIGN(arg_node) != NULL){
    TRAVdo(BOXDEF_SIGN(arg_node), arg_info);
  }

  CODEFILEwriteNewline();
    
  CODEFILEwriteGetRecord();
  
  INFO_CODETYPE(arg_info) = CTboxAssign;
  
  if(BOXDEF_SIGN(arg_node) != NULL){
    TRAVdo(BOXDEF_SIGN(arg_node), arg_info);
  } 
  
  INFO_CODETYPE(arg_info) = CTboxComp;
  
  if(BOXDEF_METADATA( arg_node) != NULL) {
    name = MDUtilMetadataGetKey(BOXDEF_METADATA( arg_node), METADATA_KEY_BOX_REAL_NAME);
    
    if(name == NULL) {
      name = BOXDEF_REALNAME(arg_node);
    }

  } 
  else {
    name = BOXDEF_REALNAME(arg_node);
  }
  
  CODEFILEwriteBoxCompStart(name);


  if(BOXDEF_SIGN(arg_node) != NULL){
    TRAVdo(BOXDEF_SIGN(arg_node), arg_info);
  }

  CODEFILEwriteBoxFullStop();
    
  CODEFILEwriteScopeStop();
  
  if(BOXDEF_BODY(arg_node) != NULL){
    TRAVdo(BOXDEF_BODY(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}

node *CTEXTboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CTEXTboxsign");

  if(BOXSIGN_INTYPE(arg_node) != NULL) {
    TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CTEXTboxtypes(node *arg_node, info *arg_info)
{
  CodeType oldType;

  DBUG_ENTER("CTEXTboxtypes");
 
  if(BOXTYPES_ENTRIES(arg_node) != NULL) {
    oldType = INFO_CODETYPE(arg_info);
    
    switch(oldType) {
    case CTboxDef:
    case CTboxAssign:
    case CTboxComp:
      TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
      break;
    default:
      break;
    }

    INFO_CODETYPE(arg_info) = oldType;
  }
  
  DBUG_RETURN(arg_node);
}

node *CTEXTbtagref(node *arg_node, info *arg_info)
{
  node *btag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CTEXTbtagref");
  
  btag = BTAGREF_BTAG(arg_node);
  pkg = BTAGS_PKG(btag);

  switch(INFO_CODETYPE(arg_info)) {
  case CTboxDef:
    CODEFILEwriteInitBtag(BTAGS_NAME(btag));
    break;
  case CTboxAssign:
    if(pkg == NULL) {
      CODEFILEwriteTakeBtag(NULL, BTAGS_NAME(btag));
    }
    else {
      CODEFILEwriteTakeBtag(NETDEF_PKGNAME(pkg), BTAGS_NAME(btag));
    }
    break;
  case CTboxComp:
    CODEFILEwriteNext();
    CODEFILEwriteBoxCompBtag(BTAGS_NAME(btag));
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CTEXTbtags(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CTEXTbtags");

  pkg = BTAGS_PKG(arg_node);

  if(pkg != NULL) {
    CODEFILEwriteBtagDefine(NETDEF_PKGNAME(pkg), BTAGS_NAME(arg_node));
  }
  else {
    CODEFILEwriteBtagDefine(NULL, BTAGS_NAME(arg_node));
  }
  
  if(BTAGS_NEXT(arg_node) != NULL) {
    TRAVdo(BTAGS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CTEXTfieldref(node *arg_node, info *arg_info)
{
  node *field = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CTEXTfieldref");
  
  field = FIELDREF_FIELD(arg_node);
  pkg = FIELDS_PKG(field);

  switch(INFO_CODETYPE(arg_info)) {
  case CTboxDef:
    CODEFILEwriteInitField(FIELDS_NAME(field));
    break;
  case CTboxAssign:
    if(pkg == NULL) {
      if (INFO_QUALIFIER(arg_info) == LQUA_pass) {
        CODEFILEwriteGetField(NULL, FIELDS_NAME(field));
      }
      else {
        CODEFILEwriteTakeField(NULL, FIELDS_NAME(field));
      }
    }
    else {
      if (INFO_QUALIFIER(arg_info) == LQUA_pass) {
        CODEFILEwriteGetField(NETDEF_NAME(pkg), FIELDS_NAME(field));
      }
      else {
        CODEFILEwriteTakeField(NETDEF_NAME(pkg), FIELDS_NAME(field));
      }
    }
    break;   
  case CTboxComp:
    CODEFILEwriteNext();
    CODEFILEwriteBoxCompField(FIELDS_NAME(field));
    break;
    
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CTEXTfields(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CTEXTfields");
  
  pkg = FIELDS_PKG(arg_node);

  if(pkg != NULL) {
    CODEFILEwriteFieldDefine(NETDEF_PKGNAME(pkg), FIELDS_NAME(arg_node));
  }
  else {
    CODEFILEwriteFieldDefine(NULL, FIELDS_NAME(arg_node));
  }
  
  if(FIELDS_NEXT(arg_node) != NULL) {
    TRAVdo(FIELDS_NEXT(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}


node *CTEXTnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CTEXTnetdef");

  /* This blocks entry to field/tag refs in net. */

  DBUG_RETURN(arg_node);
}

node *CTEXTstagref(node *arg_node, info *arg_info)
{
  node *stag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CTEXTstagref");
  
  stag = STAGREF_STAG(arg_node);
  pkg = STAGS_PKG(stag);

  switch(INFO_CODETYPE(arg_info)) {
  case CTboxDef:
    CODEFILEwriteInitStag(STAGS_NAME(stag));
    break;
  case CTboxAssign:
    if(pkg == NULL) {
      if (INFO_QUALIFIER(arg_info) == LQUA_pass) {
        CODEFILEwriteGetStag(NULL, STAGS_NAME(stag));
      }
      else {
        CODEFILEwriteTakeStag(NULL, STAGS_NAME(stag));
      }
    }
    else {
      if (INFO_QUALIFIER(arg_info) == LQUA_pass) {
        CODEFILEwriteGetStag(NETDEF_PKGNAME(pkg), STAGS_NAME(stag));
      }
      else {
        CODEFILEwriteTakeStag(NETDEF_PKGNAME(pkg), STAGS_NAME(stag));
      }
    }
    break;    
  case CTboxComp:
    CODEFILEwriteNext();
    CODEFILEwriteBoxCompStag(STAGS_NAME(stag));
    break;

  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CTEXTstags(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CTEXTstags");
  
  pkg = STAGS_PKG(arg_node);

  if(pkg != NULL) {
    CODEFILEwriteStagDefine(NETDEF_PKGNAME(pkg), STAGS_NAME(arg_node));
  }
  else {
    CODEFILEwriteStagDefine(NULL, STAGS_NAME(arg_node));
  }
  
  if(STAGS_NEXT(arg_node) != NULL) {
    TRAVdo(STAGS_NEXT(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}



node *CTEXTtypemap(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CTEXTtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL) {
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }
 
  DBUG_RETURN(arg_node);
}

node *CTEXTdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CTEXTdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CTEXTdoCode called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_ctext);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

