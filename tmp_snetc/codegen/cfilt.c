/*******************************************************************************
 *
 * $Id: cfilt.c 3371 2012-02-13 15:32:29Z mvn $
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

#include "cvar.h"
#include "cexpr.h"
#include "cfilt.h"


typedef enum{CTfilter, CTfilterInstruction} CodeType;

/*
 * INFO structure
 */

struct INFO {  
  CodeType codeType;
  node *pattern;
};


#define INFO_CODETYPE(n)     (n->codeType)
#define INFO_PATTERN(n)      (n->pattern)


static void infoClear(info *inf)
{
  INFO_CODETYPE(inf) = CTfilter;
  INFO_PATTERN(inf)  = NULL;
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

static unsigned int numberOfFilterActions(node *filterOuts)
{
  unsigned int size = 0;

  while(filterOuts != NULL) {
    size++;
    filterOuts = GUARDACTIONS_NEXT(filterOuts);
  }

  return size;
}

static unsigned int recoutsCount(node *recOuts)
{
  unsigned int size = 0;
  while(recOuts != NULL) {
    size++;    
    recOuts = RECOUTS_NEXT(recOuts);
  }

  return size;
}

static unsigned int outFieldCount(node *out)
{
  unsigned int count = 0;

  while(out != NULL) {
    if(OUTPUTFIELDS_LEFTFIELD(out) != NULL) {
      ++count;
    }
    out = OUTPUTFIELDS_NEXT(out);
  }

  return count;
}

static unsigned int outStagCount(node *out)
{
  unsigned int count = 0;

  while(out != NULL) {
    if(OUTPUTFIELDS_STAG(out) != NULL) {
      ++count;
    }
    out = OUTPUTFIELDS_NEXT(out);
  }

  return count;
}

static unsigned int outBtagCount(node *out)
{
  unsigned int count = 0;

  while(out != NULL) {
    if(OUTPUTFIELDS_BTAG(out) != NULL) {
      ++count;
    }
    out = OUTPUTFIELDS_NEXT(out);
  }

  return count;
}
static bool isStagInPattern(node *match, node *pattern)
{
  node *currentEntry = NULL;

  if(match != NULL && pattern != NULL){     
    for(currentEntry = RECTYPE_ENTRIES(pattern);currentEntry != NULL; 
        currentEntry = RECENTRIES_NEXT(currentEntry)){

      if(RECENTRIES_STAG(currentEntry) != NULL){
        if(strcmp(STAGREF_NAME(match), STAGREF_NAME(RECENTRIES_STAG(currentEntry))) == 0){
          return TRUE;
        }
      }
    }
    
  }
  return FALSE;
}


static bool isBtagInPattern(node *match, node *pattern)
{
  node *currentEntry = NULL;

  if(match != NULL && pattern != NULL){        
    for(currentEntry = RECTYPE_ENTRIES(pattern);currentEntry != NULL; 
        currentEntry = RECENTRIES_NEXT(currentEntry)){
      
      if(RECENTRIES_BTAG(currentEntry) != NULL){
        if(strcmp(BTAGREF_NAME(match), BTAGREF_NAME(RECENTRIES_BTAG(currentEntry))) == 0){
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

node *CFILTbtags(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CODEbtags");

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

node *CFILTbtaginit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEbtaginit");
  
  if(BTAGINIT_BTAG(arg_node) != NULL) {
    TRAVdo(BTAGINIT_BTAG(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}

node *CFILTbtagref(node *arg_node, info *arg_info)
{
  node *btag = NULL;

  DBUG_ENTER("CFILTbtagref");
  
  btag = BTAGREF_BTAG(arg_node);

  switch(INFO_CODETYPE(arg_info)) {
  case CTfilterInstruction:
    CODEFILEwriteNext();
    CODEFILEwriteBtag(NULL, BTAGS_NAME(btag));
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CFILTfields(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CODEfields");
  
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

node *CFILTfieldinit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEfieldinit");

  TRAVdo(FIELDINIT_FIELD(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *CFILTfieldref(node *arg_node, info *arg_info)
{
  node *field = NULL;

  DBUG_ENTER("CFILTfieldref");
  
  field = FIELDREF_FIELD(arg_node);

  switch(INFO_CODETYPE(arg_info)) {
  case CTfilterInstruction:
    CODEFILEwriteNext();
    CODEFILEwriteField(NULL, FIELDS_NAME(field));
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CFILTfilt(node *arg_node, info *arg_info)
{
  unsigned int instrsNum;

  DBUG_ENTER("CFILTfilt");

  instrsNum = numberOfFilterActions(FILT_GUARDACTIONS(arg_node));

  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();
  CODEFILEwriteSnetFilterStart();
  CODEFILEwriteLocation(FILT_LOCATION(arg_node));
  CODEFILEwriteNext();

  if(FILT_PATTERN(arg_node) != NULL) {
    TRAVdo(FILT_PATTERN(arg_node), arg_info);
  } else {
    CODEFILEwriteEmptyVariant();
  }

  CODEFILEwriteNext();

  INFO_PATTERN(arg_info) = FILT_PATTERN(arg_node);

  if (instrsNum > 0) {
    CODEFILEwriteExprListStart(instrsNum);
    CEXPRdoCode(FILT_GUARDACTIONS(arg_node));
  } else {
    //Special case for the empty filter box: []
    CODEFILEwriteExprListStart(1);
    CODEFILEwriteNext();
    CODEFILEwriteSnetEconstbTrue();
  }
  CODEFILEwriteFunctionStop();

  INFO_CODETYPE(arg_info) = CTfilterInstruction;

  if(FILT_GUARDACTIONS(arg_node) != NULL) {
    TRAVdo(FILT_GUARDACTIONS(arg_node), arg_info);
  } else { // filter: [ ], which means [{}->{}]
    CODEFILEwriteNext();
    CODEFILEwriteFilterInstrListListStart(1);
    CODEFILEwriteNext();
    CODEFILEwriteFilterInstrListStart(1);
    CODEFILEwriteNext();
    CODEFILEwriteFilterInstrCreateRecord();
    CODEFILEwriteFunctionStop();
    CODEFILEwriteFunctionStop();
  }

  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();

  INFO_PATTERN(arg_info) = NULL;

  DBUG_RETURN(arg_node);
}


node *CFILTrectype(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CFILTrectype");
  
  switch(INFO_CODETYPE(arg_info)) {
  case CTfilter: 
    CVARdoCode(arg_node);
  case CTfilterInstruction:
    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CFILTstags(node *arg_node, info *arg_info)
{
  node *pkg = NULL;

  DBUG_ENTER("CODEstags");
  
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

node *CFILTstaginit(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CODEstaginit");

  TRAVdo(STAGINIT_STAG(arg_node), arg_info);

  DBUG_RETURN(arg_node);

}

node *CFILTstagref(node *arg_node, info *arg_info)
{
  node *stag = NULL;

  DBUG_ENTER("CFILTstagref");
  
  stag = STAGREF_STAG(arg_node);

  switch(INFO_CODETYPE(arg_info)) {
  case CTfilterInstruction:
    CODEFILEwriteNext();
    CODEFILEwriteStag(NULL, STAGS_NAME(stag));
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CFILTguardactions(node *arg_node, info *arg_info)
{
  unsigned int recoutNum;

  DBUG_ENTER("CFILTguardactions");

  recoutNum = recoutsCount(GUARDACTIONS_ACTION(arg_node));

  switch(INFO_CODETYPE(arg_info)){
  case CTfilterInstruction:
    CODEFILEwriteNext();
    
    if(recoutNum == 0) {
      CODEFILEwriteNULL();

      if(GUARDACTIONS_NEXT(arg_node) != NULL){
        TRAVdo(GUARDACTIONS_NEXT(arg_node), arg_info);        
      }
    }    
    else {
      CODEFILEwriteFilterInstrListListStart(recoutNum);
      
      if(GUARDACTIONS_ACTION(arg_node) != NULL){
        TRAVdo(GUARDACTIONS_ACTION(arg_node), arg_info);
      }
      CODEFILEwriteFunctionStop();
      
      if(GUARDACTIONS_NEXT(arg_node) != NULL){
        TRAVdo(GUARDACTIONS_NEXT(arg_node), arg_info);        
      }
    }
    
    break;
  default:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *CFILTtypemap(node *arg_node, info *arg_info){
  DBUG_ENTER("CFILTtypemap");

  if(TYPEMAP_INTYPE(arg_node) != NULL){
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}


node *CFILTrecouts(node *arg_node, info *arg_info)
{
  unsigned int fieldNum;
  unsigned int stagNum;
  unsigned int btagNum;
  unsigned int outNum;

  DBUG_ENTER("CFILTrecouts"); 
  
  fieldNum = outFieldCount(RECOUTS_FIELDS(arg_node));
  stagNum  = outStagCount(RECOUTS_FIELDS(arg_node));
  btagNum  = outBtagCount(RECOUTS_FIELDS(arg_node));
  outNum = fieldNum + stagNum +  btagNum;

  switch(INFO_CODETYPE(arg_info)) {

  case CTfilterInstruction:

    CODEFILEwriteNext();

    CODEFILEwriteFilterInstrListStart(outNum + 1);
    CODEFILEwriteNext();
    CODEFILEwriteFilterInstrCreateRecord();

    if(RECOUTS_FIELDS(arg_node) != NULL){
      TRAVdo(RECOUTS_FIELDS(arg_node), arg_info);
    }

    CODEFILEwriteFunctionStop();

    if(RECOUTS_NEXT(arg_node) != NULL)
      TRAVdo(RECOUTS_NEXT(arg_node), arg_info);
    break;
  default:
    break;
  }
  DBUG_RETURN(arg_node);
}

node *CFILToutputfields(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CFILToutputfields");

  switch(INFO_CODETYPE(arg_info)) {
  case CTfilterInstruction:
    CODEFILEwriteNext();

    if(OUTPUTFIELDS_LEFTFIELD(arg_node) != NULL){  
      if(OUTPUTFIELDS_RIGHTFIELD(arg_node) != NULL){
        CODEFILEwriteFilterInstrFieldStart();
        TRAVdo(OUTPUTFIELDS_LEFTFIELD(arg_node), arg_info);
        TRAVdo(OUTPUTFIELDS_RIGHTFIELD(arg_node), arg_info);        
        CODEFILEwriteFunctionStop();
      }
      else {
        CODEFILEwriteFilterInstrFieldStart();
        TRAVdo(OUTPUTFIELDS_LEFTFIELD(arg_node), arg_info);
        TRAVdo(OUTPUTFIELDS_LEFTFIELD(arg_node), arg_info);
        CODEFILEwriteFunctionStop();
      }
    }
    else if(OUTPUTFIELDS_STAG(arg_node) != NULL){
      if(OUTPUTFIELDS_TAGEXPR(arg_node) != NULL){
        CODEFILEwriteFilterInstrTagStart();
        TRAVdo(OUTPUTFIELDS_STAG(arg_node), arg_info);
        CODEFILEwriteNext();
        CEXPRdoCode(OUTPUTFIELDS_TAGEXPR(arg_node));
        CODEFILEwriteFunctionStop(); 
      }
      else {
        CODEFILEwriteFilterInstrTagStart();
        TRAVdo(OUTPUTFIELDS_STAG(arg_node), arg_info);
        CODEFILEwriteNext();

        if(isStagInPattern(OUTPUTFIELDS_STAG(arg_node), INFO_PATTERN(arg_info)) == TRUE) {
          CODEFILEwriteSnetEStagStart();
          CEXPRdoCode(OUTPUTFIELDS_STAG(arg_node));
          CODEFILEwriteFunctionStop();
        }
        else {
          CODEFILEwriteSnetEconsti(0);
        }
        
        CODEFILEwriteFunctionStop();
      }
    }
    else if(OUTPUTFIELDS_BTAG(arg_node) != NULL){
      if(OUTPUTFIELDS_TAGEXPR(arg_node) != NULL){
        CODEFILEwriteFilterInstrBtagStart();
        TRAVdo(OUTPUTFIELDS_BTAG(arg_node), arg_info);
        CODEFILEwriteNext();
        CEXPRdoCode(OUTPUTFIELDS_TAGEXPR(arg_node));
        CODEFILEwriteFunctionStop(); 
      }
      else {
        CODEFILEwriteFilterInstrBtagStart();
        TRAVdo(OUTPUTFIELDS_BTAG(arg_node), arg_info);
        CODEFILEwriteNext();

        if(isBtagInPattern(OUTPUTFIELDS_BTAG(arg_node), INFO_PATTERN(arg_info)) == TRUE) {
          CODEFILEwriteSnetEBtagStart();
          CEXPRdoCode(OUTPUTFIELDS_BTAG(arg_node));
          CODEFILEwriteFunctionStop();
        }
        else {
          CODEFILEwriteSnetEconsti(0);
        }
        
        CODEFILEwriteFunctionStop();
      }
    }

    if(OUTPUTFIELDS_NEXT(arg_node) != NULL) {
      TRAVdo(OUTPUTFIELDS_NEXT(arg_node), arg_info);
    }

    break;

  default:
    break;
  }
  DBUG_RETURN(arg_node);
}

node *CFILTdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CFILTdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CFILTdoCode called with empty syntaxtree");
  
  inf = infoMake();

  TRAVpush(TR_cfilt);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

