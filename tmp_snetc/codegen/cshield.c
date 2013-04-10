/*******************************************************************************
 *
 * $Id: cshield.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   01.02.2008
 * -----
 *
 *******************************************************************************/

#include "codegen.h"
#include <string.h>
#include "codefile.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "set.h"

#include "cshield.h"

typedef enum{CTunknown, CTshieldIn, CTshieldOut} CodeType;


/*
 * INFO structure
 */
struct INFO {  
  CodeType codeType;
  node *def;
  set *fields;
  set *stags;
  set *btags;
};


#define INFO_CODETYPE(n)     (n->codeType)
#define INFO_DEF(n)          (n->def)
#define INFO_FIELDSET(n)     (n->fields)
#define INFO_STAGSET(n)      (n->stags)
#define INFO_BTAGSET(n)      (n->btags)

static void infoClear(info *inf)
{
  INFO_CODETYPE(inf) = CTunknown;
  INFO_DEF(inf)      = NULL;   
  INFO_FIELDSET(inf) = NULL;
  INFO_STAGSET(inf)  = NULL;
  INFO_BTAGSET(inf)  = NULL;
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


static void writeVariantFromInfo(info *arg_info)
{
  int i, size;
  node *elem = NULL;
  node *pkg = NULL;

  CODEFILEwriteVariantStart();

  CODEFILEwriteIntListStart(size = SETsize(INFO_FIELDSET(arg_info)));
  for (i = 0; i < size; i++) {
    elem= SETelem(INFO_FIELDSET(arg_info), i);
    pkg = FIELDS_PKG(elem);
    CODEFILEwriteNext();
    if (pkg == NULL) {
      CODEFILEwriteField(NULL, FIELDS_NAME(elem));
    }
    else {
      CODEFILEwriteField(NETDEF_NAME(pkg), FIELDS_NAME(elem));
    }
  }
  CODEFILEwriteFunctionStop();
  CODEFILEwriteNext();
  CODEFILEwriteIntListStart(size = SETsize(INFO_STAGSET(arg_info)));
  for (i = 0; i < size; i++) {
    elem = SETelem(INFO_STAGSET(arg_info), i);
    pkg = STAGS_PKG(elem);
    CODEFILEwriteNext();
    if (pkg == NULL) {
      CODEFILEwriteStag(NULL, STAGS_NAME(elem));
    }
    else {
      CODEFILEwriteStag(NETDEF_NAME(pkg), STAGS_NAME(elem));
    }
  }
  CODEFILEwriteFunctionStop();
  CODEFILEwriteNext();
  CODEFILEwriteIntListStart(size = SETsize(INFO_BTAGSET(arg_info)));
  for (i = 0; i < size; i++) {
    elem = SETelem(INFO_BTAGSET(arg_info), i);
    pkg = BTAGS_PKG(elem);
    CODEFILEwriteNext();
    if (pkg == NULL) {
      CODEFILEwriteBtag(NULL, BTAGS_NAME(elem));
    }
    else {
      CODEFILEwriteBtag(NETDEF_NAME(pkg), BTAGS_NAME(elem));
    }
  }
  CODEFILEwriteFunctionStop();
  CODEFILEwriteFunctionStop();
}

node *CSHIELDboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSHIELDboxsign");

  TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);
  
  DBUG_RETURN(arg_node);
}

node *CSHIELDbtagref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSHIELDbtagref");

  INFO_BTAGSET(arg_info) = SETaddElem(INFO_BTAGSET(arg_info), BTAGREF_BTAG(arg_node));

  DBUG_RETURN(arg_node);
}

node *CSHIELDfieldref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSHIELDfieldref");

  INFO_FIELDSET(arg_info) = SETaddElem(INFO_FIELDSET(arg_info), FIELDREF_FIELD(arg_node));
 
  DBUG_RETURN(arg_node);
}

node *CSHIELDnetdef(node *arg_node, info *arg_info)
{
  node *typesigns = NULL;

  DBUG_ENTER("CSHIELDnetdef");


  if(INFO_CODETYPE(arg_info) == CTunknown){
    typesigns = NETDEF_SIGN(arg_node);
    INFO_DEF(arg_info) = arg_node;

    INFO_CODETYPE(arg_info) = CTshieldIn;
    INFO_FIELDSET(arg_info) = SETnewSet(NULL);
    INFO_STAGSET(arg_info) = SETnewSet(NULL);
    INFO_BTAGSET(arg_info) = SETnewSet(NULL);
    TRAVdo(typesigns, arg_info);
    CODEFILEwriteShieldInPart1(NETDEF_NAME(arg_node), -2);
    writeVariantFromInfo(arg_info);
    CODEFILEwriteShieldPart2();
    
    CODEFILEwriteShieldBodyPart1(NETDEF_NAME(arg_node), -2);
    CODEFILEwriteShieldPart2();
    
    INFO_CODETYPE(arg_info) = CTshieldOut;
    INFO_FIELDSET(arg_info) = SETclearSet(INFO_FIELDSET(arg_info));
    INFO_STAGSET(arg_info) = SETclearSet(INFO_STAGSET(arg_info));
    INFO_BTAGSET(arg_info) = SETclearSet(INFO_BTAGSET(arg_info));
    TRAVdo(typesigns, arg_info);
    CODEFILEwriteShieldOutPart1(NETDEF_NAME(arg_node), -2);
    writeVariantFromInfo(arg_info);
    CODEFILEwriteShieldPart2();
    
    INFO_FIELDSET(arg_info) = SETfreeSet(INFO_FIELDSET(arg_info));
    INFO_STAGSET(arg_info) = SETfreeSet(INFO_STAGSET(arg_info));
    INFO_BTAGSET(arg_info) = SETfreeSet(INFO_BTAGSET(arg_info));
    
    if(!NETDEF_TOPLEVEL(arg_node)){
     CODEFILEwriteShieldNet(NETDEF_NAME(arg_node), -2);
    }
   else {   // top level network
     CODEFILEwriteShieldTopLevelNet(NETDEF_NAME(arg_node), -2);
    }
  }
    
  DBUG_RETURN(arg_node);
}

node *CSHIELDrectype(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSHIELDrectype");
  
  switch(INFO_CODETYPE(arg_info)) {
  case CTshieldIn:
  case CTshieldOut:
    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CSHIELDstagref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSHIELDstagref");

  INFO_STAGSET(arg_info) = SETaddElem(INFO_STAGSET(arg_info), STAGREF_STAG(arg_node));

  DBUG_RETURN(arg_node);
}

node *CSHIELDtrans(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("CSHIELDtrans");

  net = INFO_DEF(arg_info);

  CODEFILEwriteNext();
  CODEFILEwriteSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
  
  DBUG_RETURN(arg_node);
}

node *CSHIELDtypemap(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CSHIELDtypemap");

  if(INFO_CODETYPE(arg_info) == CTshieldOut) {
    if(TYPEMAP_OUTTYPE(arg_node) != NULL) {
      TRAVdo(TYPEMAP_OUTTYPE(arg_node), arg_info);
    }
  }
  else {
    if(TYPEMAP_INTYPE(arg_node) != NULL){
      TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
    }
  }
  DBUG_RETURN(arg_node);
}

node *CSHIELDdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CSHIELDdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CSHIELDdoCode called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_cshield);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

