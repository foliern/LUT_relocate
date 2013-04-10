/*******************************************************************************
 *
 * $Id: cvar.c 3371 2012-02-13 15:32:29Z mvn $
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

typedef enum{CTunknown, CTsync} CodeType;

/*
 * INFO structure
 */

struct INFO {  
  CodeType codeType;
  filttype filter;
};


#define INFO_FILTER(n)       (n->filter)
#define INFO_CODETYPE(n)     (n->codeType)

static void infoClear(info *inf)
{
  INFO_CODETYPE(inf) = CTunknown;
  INFO_FILTER(inf)   = FILT_empty;
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

static unsigned int fieldCount(node *recEntries)
{
  unsigned int count = 0;

  while(recEntries != NULL) {
    if(RECENTRIES_FIELD(recEntries) != NULL) {
      ++count;
    }
    recEntries = RECENTRIES_NEXT(recEntries);
  }

  return count;
}

static unsigned int stagCount(node *recEntries)
{
  unsigned int count = 0;

  while(recEntries != NULL) {
    if(RECENTRIES_STAG(recEntries) != NULL) {
      ++count;
    }
    recEntries = RECENTRIES_NEXT(recEntries);
  }

  return count;
}

static unsigned int btagCount(node *recEntries)
{
  unsigned int count = 0;

  while(recEntries != NULL) {
    if(RECENTRIES_BTAG(recEntries) != NULL) {
      ++count;
    }
    recEntries = RECENTRIES_NEXT(recEntries);
  }

  return count;
}

static unsigned int guardpatternFieldCount(node *patterns)
{
  unsigned int count = 0;
  node *recType = NULL;
  
 while(patterns != NULL) {
    recType = GUARDPATTERNS_ENTRIES(patterns);
    count += fieldCount(RECTYPE_ENTRIES(recType));
    patterns = GUARDPATTERNS_NEXT(patterns);
  }

  return count;
}

static unsigned int guardpatternStagCount(node *patterns)
{
  unsigned int count = 0;
  node *recType = NULL;

  while(patterns != NULL) {
    recType = GUARDPATTERNS_ENTRIES(patterns);
    count += stagCount(RECTYPE_ENTRIES(recType));
    patterns = GUARDPATTERNS_NEXT(patterns);
  }

  return count;
}

static unsigned int guardpatternBtagCount(node *patterns)
{
  unsigned int count = 0;
  node *recType = NULL;

  while(patterns != NULL) {
    recType = GUARDPATTERNS_ENTRIES(patterns);
    count += btagCount(RECTYPE_ENTRIES(recType));
    patterns = GUARDPATTERNS_NEXT(patterns);
  }

  return count;
}

node *CVARboxtypes(node *arg_node, info *arg_info)
{
  unsigned int fieldNum;
  unsigned int stagNum;
  unsigned int btagNum;

  DBUG_ENTER("CVARboxtypes");

  fieldNum = fieldCount(BOXTYPES_ENTRIES(arg_node));
  stagNum = stagCount(BOXTYPES_ENTRIES(arg_node));
  btagNum = btagCount(BOXTYPES_ENTRIES(arg_node));

  CODEFILEwriteVariantStart();
  
  INFO_FILTER(arg_info) = FILT_field;
  CODEFILEwriteIntListStart(fieldNum);

  if(BOXTYPES_ENTRIES(arg_node) != NULL) {
    TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
  }

  CODEFILEwriteFunctionStop();
  
  INFO_FILTER(arg_info) = FILT_stag;
  CODEFILEwriteNext();      
  CODEFILEwriteIntListStart(stagNum);

  if(BOXTYPES_ENTRIES(arg_node) != NULL) {
    TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
  }

  CODEFILEwriteFunctionStop();
  
  INFO_FILTER(arg_info) = FILT_btag;
  CODEFILEwriteNext();
  CODEFILEwriteIntListStart(btagNum);

  if(BOXTYPES_ENTRIES(arg_node) != NULL) {
    TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
  }

  CODEFILEwriteFunctionStop();
  CODEFILEwriteFunctionStop();

  DBUG_RETURN(arg_node);
}

node *CVARbtagref(node *arg_node, info *arg_info)
{
  node *btag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CVARbtagref");
  
  btag = BTAGREF_BTAG(arg_node);
  pkg = BTAGS_PKG(btag);

  if(INFO_FILTER(arg_info) == FILT_btag) {
    if(pkg == NULL) {
      CODEFILEwriteNext();
      CODEFILEwriteBtag(NULL, BTAGS_NAME(btag));
    }
    else {
      CODEFILEwriteNext();
      CODEFILEwriteBtag(NETDEF_PKGNAME(pkg), BTAGS_NAME(btag));
    }
  }
  
  DBUG_RETURN(arg_node);
}

node *CVARfieldref(node *arg_node, info *arg_info)
{
  node *field = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CVARfieldref");
  
  field = FIELDREF_FIELD(arg_node);
  pkg = FIELDS_PKG(field);
  
  if(INFO_FILTER(arg_info) == FILT_field) {
    if(pkg == NULL) {
      CODEFILEwriteNext();
      CODEFILEwriteField(NULL, FIELDS_NAME(field));
    }
    else {
      CODEFILEwriteNext();
      CODEFILEwriteField(NETDEF_NAME(pkg), FIELDS_NAME(field));
    }
  }
  
  DBUG_RETURN(arg_node);
}

node *CVARrectype(node *arg_node, info *arg_info)
{
  unsigned int fieldNum, stagNum, btagNum;

  DBUG_ENTER("CVARrectype");

  switch(INFO_CODETYPE(arg_info)){
  case CTsync:
    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }
    break;
  default:
    fieldNum = fieldCount(RECTYPE_ENTRIES(arg_node));
    stagNum  = stagCount(RECTYPE_ENTRIES(arg_node));
    btagNum  = btagCount(RECTYPE_ENTRIES(arg_node));
    
    CODEFILEwriteVariantStart();
    
    INFO_FILTER(arg_info) = FILT_field;
    CODEFILEwriteIntListStart(fieldNum);

    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }

    CODEFILEwriteFunctionStop();
    
    INFO_FILTER(arg_info) = FILT_stag;
    CODEFILEwriteNext();
    CODEFILEwriteIntListStart(stagNum);

    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }

    CODEFILEwriteFunctionStop();
    
    INFO_FILTER(arg_info) = FILT_btag;
    CODEFILEwriteNext();
    CODEFILEwriteIntListStart(btagNum);

    if(RECTYPE_ENTRIES(arg_node) != NULL) {
      TRAVdo(RECTYPE_ENTRIES(arg_node), arg_info);
    }

    CODEFILEwriteFunctionStop();
    CODEFILEwriteFunctionStop();
    break;
  }

  DBUG_RETURN(arg_node);
}

node *CVARsync(node *arg_node, info *arg_info)
{
  unsigned int fieldNum;
  unsigned int stagNum;
  unsigned int btagNum;

  DBUG_ENTER("CVARsync");

  fieldNum   = guardpatternFieldCount(SYNC_AUXPATTERNS(arg_node))
             + guardpatternFieldCount(SYNC_MAINPATTERN(arg_node));
  stagNum    = guardpatternStagCount(SYNC_AUXPATTERNS(arg_node))
             + guardpatternStagCount(SYNC_MAINPATTERN(arg_node));
  btagNum    = guardpatternBtagCount(SYNC_AUXPATTERNS(arg_node))
             + guardpatternBtagCount(SYNC_MAINPATTERN(arg_node));
  
  INFO_CODETYPE(arg_info) = CTsync;

  CODEFILEwriteVariantStart();  
  CODEFILEwriteIntListStart(fieldNum);
  INFO_FILTER(arg_info) = FILT_field; 
  
  TRAVdo(SYNC_MAINPATTERN(arg_node), arg_info);
  if(SYNC_AUXPATTERNS(arg_node) != NULL) {
    TRAVdo(SYNC_AUXPATTERNS(arg_node), arg_info);
  }

  CODEFILEwriteFunctionStop();
  
  CODEFILEwriteNext();
  CODEFILEwriteIntListStart(stagNum); 
  INFO_FILTER(arg_info) = FILT_stag; 
  
  TRAVdo(SYNC_MAINPATTERN(arg_node), arg_info);
  if(SYNC_AUXPATTERNS(arg_node) != NULL) {
    TRAVdo(SYNC_AUXPATTERNS(arg_node), arg_info);
  }

  CODEFILEwriteFunctionStop();
  
  CODEFILEwriteNext();
  CODEFILEwriteIntListStart(btagNum); 
  INFO_FILTER(arg_info) = FILT_btag; 

  TRAVdo(SYNC_MAINPATTERN(arg_node), arg_info);
  if(SYNC_AUXPATTERNS(arg_node) != NULL) {
    TRAVdo(SYNC_AUXPATTERNS(arg_node), arg_info);
  }

  CODEFILEwriteFunctionStop();
  CODEFILEwriteFunctionStop();

  DBUG_RETURN(arg_node);
}

node *CVARstagref(node *arg_node, info *arg_info)
{
  node *stag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CVARstagref");
  
  stag = STAGREF_STAG(arg_node);
  pkg = STAGS_PKG(stag);

  if(INFO_FILTER(arg_info) == FILT_stag) {
    if(pkg == NULL) {
      CODEFILEwriteNext();
      CODEFILEwriteStag(NULL, STAGS_NAME(stag));
    }
    else {
      CODEFILEwriteNext();
      CODEFILEwriteStag(NETDEF_PKGNAME(pkg), STAGS_NAME(stag));
    }
  }
  
  DBUG_RETURN(arg_node);
}

node *CVARdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CVARdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CVARdoCode called with empty syntaxtree");
  inf = infoMake();

  TRAVpush(TR_cvar);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
