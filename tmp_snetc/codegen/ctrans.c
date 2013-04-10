/*******************************************************************************
 *
 * $Id: ctrans.c 3371 2012-02-13 15:32:29Z mvn $
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
#include "ctrans.h"

typedef enum{CTtranslate, CTtranslateInstruction} CodeType;

/*
 * INFO structure
 */

struct INFO {  
  CodeType codeType;
  filttype filter;
  unsigned int index;
};

#define INFO_CODETYPE(n)     (n->codeType)
#define INFO_FILTER(n)       (n->filter)
#define INFO_INDEX(n)        (n->index)

static void infoClear(info *inf)
{
  INFO_CODETYPE(inf) = CTtranslate;
  INFO_FILTER(inf)   = FILT_empty;
  INFO_INDEX(inf)    = 0;
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

static unsigned int boxTypeFieldCount(node *boxType)
{
  if(BOXTYPES_ENTRIES(boxType) == NULL) {
    return 0;
  }
  return fieldCount(BOXTYPES_ENTRIES(boxType));
}

static unsigned int boxTypeStagCount(node *boxType)
{
  if(BOXTYPES_ENTRIES(boxType) == NULL) {
    return 0;
  }
  return stagCount(BOXTYPES_ENTRIES(boxType));
}

static unsigned int boxTypeBtagCount(node *boxType)
{
  if(BOXTYPES_ENTRIES(boxType) == NULL) {
    return 0;
  }
  return btagCount(BOXTYPES_ENTRIES(boxType));
}

node *CTRANSboxsign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CTRANSboxsign");

  TRAVdo(BOXSIGN_INTYPE(arg_node), arg_info);
 
  DBUG_RETURN(arg_node);
}

node *CTRANSboxtypes(node *arg_node, info *arg_info)
{
  CodeType oldType;

  DBUG_ENTER("CTRANSboxtypes");
 
  if(BOXTYPES_ENTRIES(arg_node) != NULL) {
    oldType = INFO_CODETYPE(arg_info);
    switch(oldType) {
    case CTtranslate:
      CODEFILEwriteNext();
      CVARdoCode(arg_node);
      break;      
    case CTtranslateInstruction:
      TRAVdo(BOXTYPES_ENTRIES(arg_node), arg_info);
      break; 
    default:
      break;
    }

    INFO_CODETYPE(arg_info) = oldType;
  }
  
  DBUG_RETURN(arg_node);
}

node *CTRANSbtagref(node *arg_node, info *arg_info)
{
  node *btag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CTRANSbtagref");
  
  btag = BTAGREF_BTAG(arg_node);
  pkg = BTAGS_PKG(btag);

  switch(INFO_CODETYPE(arg_info)) {
  case CTtranslateInstruction:
    if(INFO_FILTER(arg_info) == FILT_btag) {
      if(INFO_INDEX(arg_info) == 0) {
        if(pkg == NULL) {
          CODEFILEwriteBtag(NULL, BTAGS_NAME(btag));
        }
        else {
          CODEFILEwriteBtag(NETDEF_NAME(pkg), BTAGS_NAME(btag));
        }
      }
      INFO_INDEX(arg_info)--;
    }
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}


node *CTRANSfieldref(node *arg_node, info *arg_info)
{
  node *field = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CTRANSfieldref");
  
  field = FIELDREF_FIELD(arg_node);
  pkg = FIELDS_PKG(field);

  switch(INFO_CODETYPE(arg_info)) {
  case CTtranslateInstruction:
    if(INFO_FILTER(arg_info) == FILT_field) {
      if(INFO_INDEX(arg_info) == 0) {
        if(pkg == NULL) {
          CODEFILEwriteField(NULL, FIELDS_NAME(field));
        }
        else {
          CODEFILEwriteField(NETDEF_NAME(pkg), FIELDS_NAME(field));
        }
      }
      INFO_INDEX(arg_info)--;
    }
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CTRANSstagref(node *arg_node, info *arg_info)
{
  node *stag = NULL;
  node *pkg = NULL;

  DBUG_ENTER("CTRANSstagref");
  
  stag = STAGREF_STAG(arg_node);
  pkg = STAGS_PKG(stag);

  switch(INFO_CODETYPE(arg_info)) {
  case CTtranslateInstruction:
    if(INFO_FILTER(arg_info) == FILT_stag) {
      if(INFO_INDEX(arg_info) == 0) {
        if(pkg == NULL) {
          CODEFILEwriteStag(NULL, STAGS_NAME(stag));
        }
        else {
          CODEFILEwriteStag(NETDEF_NAME(pkg), STAGS_NAME(stag));
        }
      }
      INFO_INDEX(arg_info)--;
    }
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CTRANStrans(node *arg_node, info *arg_info)
{
  unsigned int fieldNum;
  unsigned int stagNum;
  unsigned int btagNum;
  unsigned int instructionNum;
  unsigned int i;;

  DBUG_ENTER("CTRANStrans");
  
  fieldNum = boxTypeFieldCount(TRANS_LEFT(arg_node));
  stagNum  = boxTypeStagCount(TRANS_LEFT(arg_node));
  btagNum  = boxTypeBtagCount(TRANS_LEFT(arg_node));
  instructionNum = fieldNum+stagNum+btagNum;
  
  CODEFILEwriteOutBufDecl();
  CODEFILEwriteNewline();
  CODEFILEwriteSnetTranslateStart();

  CODEFILEwriteLocation(TRANS_LOCATION(arg_node));

  CODEFILEwriteNext();
  CODEFILEwriteVariantListStart(1);
  
  if(TRANS_LEFT(arg_node) != NULL) {
    TRAVdo(TRANS_LEFT(arg_node), arg_info);
  }

  CODEFILEwriteFunctionStop();
  CODEFILEwriteNext();
  
  CODEFILEwriteNULL();
  CODEFILEwriteNext();
  
  if(instructionNum > 0) {
    CODEFILEwriteFilterInstrListListStart(1);
    CODEFILEwriteNext();
    
    CODEFILEwriteFilterInstrListStart(instructionNum + 1);
    CODEFILEwriteNext();
    CODEFILEwriteFilterInstrCreateRecord();
    
    INFO_CODETYPE(arg_info) = CTtranslateInstruction;
    
    INFO_FILTER(arg_info) = FILT_field;

    for(i = 0; i < fieldNum; ++i) {
      CODEFILEwriteNext();
      CODEFILEwriteFilterInstrFieldStart();
      
      CODEFILEwriteNext();
      INFO_INDEX(arg_info) = i;

      if(TRANS_RIGHT(arg_node) != NULL) {
	TRAVdo(TRANS_RIGHT(arg_node), arg_info);
      }
      
      CODEFILEwriteNext();
      INFO_INDEX(arg_info) = i;

      if(TRANS_LEFT(arg_node) != NULL) {
	TRAVdo(TRANS_LEFT(arg_node), arg_info);
      }
      
      CODEFILEwriteFunctionStop();
    }
    
    INFO_FILTER(arg_info) = FILT_stag;

    for(i = 0; i < stagNum; ++i) {
      CODEFILEwriteNext();
      CODEFILEwriteFilterInstrTagStart();
      CODEFILEwriteNext();
      
      INFO_INDEX(arg_info) = i;

      if(TRANS_RIGHT(arg_node) != NULL) {
	TRAVdo(TRANS_RIGHT(arg_node), arg_info);
      }
      CODEFILEwriteNext();
      
      CODEFILEwriteSnetEStagStart();
      INFO_INDEX(arg_info) = i;

      if(TRANS_LEFT(arg_node) != NULL) {
	TRAVdo(TRANS_LEFT(arg_node), arg_info);
      }

      CODEFILEwriteFunctionStop();
      CODEFILEwriteFunctionStop();
    }
    
    INFO_FILTER(arg_info) = FILT_btag;

    for(i = 0; i < btagNum; ++i) {
      CODEFILEwriteNext();
      CODEFILEwriteFilterInstrBtagStart();
      CODEFILEwriteNext();

      INFO_INDEX(arg_info) = i;

      if(TRANS_RIGHT(arg_node) != NULL) {
	TRAVdo(TRANS_RIGHT(arg_node), arg_info);
        }
      CODEFILEwriteNext();
      CODEFILEwriteSnetEBtagStart();
      INFO_INDEX(arg_info) = i;
      
      if(TRANS_LEFT(arg_node) != NULL) {
	TRAVdo(TRANS_LEFT(arg_node), arg_info);
      }
      CODEFILEwriteFunctionStop();
      CODEFILEwriteFunctionStop();
      
    }

    CODEFILEwriteFunctionStop();
    CODEFILEwriteFunctionStop();
  }

  CODEFILEwriteFunctionFullStop();
  CODEFILEwriteOutBufReturn();
  
  DBUG_RETURN(arg_node);
}

node *CTRANStypemap(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CTRANStypemap");
  
  if(TYPEMAP_INTYPE(arg_node) != NULL) {
    TRAVdo(TYPEMAP_INTYPE(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CTRANSdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CTRANSdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CTRANSdoCode called with empty syntaxtree");
 
  inf = infoMake();

  TRAVpush(TR_ctrans);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

