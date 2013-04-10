/*******************************************************************************
 *
 * $Id: cchoice.c 3371 2012-02-13 15:32:29Z mvn $
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
#include "cchoice.h"

typedef enum{CTparallel, CTparType} CodeType;

/*
 * INFO structure
 */

struct INFO {  
  CodeType codeType;
  node *def;
};


#define INFO_CODETYPE(n)     (n->codeType)
#define INFO_DEF(n)          (n->def)

static void infoClear(info *inf)
{
  INFO_CODETYPE(inf) = CTparallel;
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

static unsigned int typesSize(node *types)
{
  unsigned int size = 0;

  while(types != NULL) {
    ++size;
    types = TYPES_NEXT(types);
  }

  return size;
}

node *CCHOICEboxref(node *arg_node, info *arg_info)
{
  node *box = NULL;

  DBUG_ENTER("CCHOICEboxref");
  
  box = BOXREF_BOX(arg_node);

  switch(INFO_CODETYPE(arg_info)) {
  case CTparallel:
    CODEFILEwriteNext();
    CODEFILEwriteSnetEntity(NULL, BOXDEF_NAME(box)); /* REALNAME */
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CCHOICEchoice(node *arg_node, info *arg_info)
{
  unsigned int parts;

  DBUG_ENTER("CCHOICEchoice");
  
    CODEFILEwriteOutBufDecl();
    CODEFILEwriteNewline();

    if(CHOICE_ISDETERM(arg_node)) {
      CODEFILEwriteSnetParallelDetStart();
    } else {
      CODEFILEwriteSnetParallelStart();
    }
    CODEFILEwriteLocation(CHOICE_LOCATION(arg_node));

    CODEFILEwriteNext();
    parts = BRANCHLIST_COUNT(CHOICE_BRANCHLIST(arg_node));
    CODEFILEwriteVariantListListStart(parts);
    
    INFO_CODETYPE(arg_info) = CTparType;

    TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);
        
    CODEFILEwriteFunctionStop();
    
    INFO_CODETYPE(arg_info) = CTparallel;

    TRAVdo(CHOICE_BRANCHLIST(arg_node), arg_info);
        
    CODEFILEwriteFunctionFullStop();
    CODEFILEwriteOutBufReturn();
  
  DBUG_RETURN(arg_node);
}

node *CCHOICEbranchlist(node *arg_node, info *arg_info)
{
  node *types;
  int size;

  DBUG_ENTER("CCHOICEbranchlist");

  switch (INFO_CODETYPE(arg_info)) {
    case CTparType:
      types = BRANCHLIST_ATTRACTS(arg_node);
      size = typesSize(types);
      CODEFILEwriteNext();
      CODEFILEwriteVariantListStart(size);
      if (types != NULL) {
        TRAVdo(types, arg_info);
      }
      CODEFILEwriteFunctionStop();
      break;
    case CTparallel:
      TRAVdo(BRANCHLIST_BRANCH(arg_node), arg_info);
      break;
  }

  if (BRANCHLIST_NEXT(arg_node) != NULL) {
    TRAVdo(BRANCHLIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *CCHOICEnetrefs(node *arg_node, info *arg_info)
{
  node *net = NULL;

  DBUG_ENTER("CCHOICEnetrefs");
  
  net = NETREFS_NET(arg_node);

  switch(INFO_CODETYPE(arg_info)) {
  case CTparallel:
    CODEFILEwriteNext();
    CODEFILEwriteSnetEntity(NETDEF_PKGNAME(net), NETDEF_NAME(net));
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}


node *CCHOICErectype(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CCHOICErectype");
  
  switch(INFO_CODETYPE(arg_info)) { 
  case CTparType:   
    CODEFILEwriteNext();
    
    CVARdoCode(arg_node);
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CCHOICEtypes(node *arg_node, info *arg_info)
{
  DBUG_ENTER("CCHOICEtypes");
  
  switch(INFO_CODETYPE(arg_info)) {
  case CTparType:
    if(TYPES_TYPE(arg_node) != NULL) {
      TRAVdo(TYPES_TYPE(arg_node), arg_info);
    }
    if(TYPES_NEXT(arg_node) != NULL) {
      TRAVdo(TYPES_NEXT(arg_node), arg_info);
    }
    break;
  default:
    break;
  }
  
  DBUG_RETURN(arg_node);
}

node *CCHOICEdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CCHOICEdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CCHOICEdoCode called with empty syntaxtree");
  
  inf = infoMake();

  TRAVpush(TR_cchoice);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

