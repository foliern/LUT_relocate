/*******************************************************************************
 *
 * $Id: mpreproc.c 2506 2009-08-05 08:01:58Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   30.07.2009
 * -----
 *
 * Preprocessing of metadata: Childs of each net element are
 * sorted in key-value pairs and child elements
 * 
 *
 *******************************************************************************/

#include <string.h>
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "free.h"
#include "memory.h"
#include "str.h"

#include "mpreproc.h"

typedef enum {TYPE_unknown, TYPE_key, TYPE_child} md_type_t;

struct INFO { 
  node *keys;
  node *childs;
  md_type_t type;
};

#define INFO_KEYS(n)      (n->keys)
#define INFO_CHILDS(n)    (n->childs)
#define INFO_TYPE(n)      (n->type)

static void infoClear(info *inf)
{
  INFO_KEYS(inf) = NULL;
  INFO_CHILDS(inf) = NULL;
  INFO_TYPE(inf) = TYPE_unknown;
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

  if(INFO_KEYS(inf) != NULL) {
    FREEdoFreeTree(INFO_KEYS(inf));
  }

  if(INFO_CHILDS(inf) != NULL) {
    FREEdoFreeTree(INFO_CHILDS(inf));
  }

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}


node *MDPREPROCmetadatanetdef( node *arg_node, info *arg_info )
{
  info *inf;

  DBUG_ENTER("MDPREPROCmetadatanetdefs");

  inf = infoMake();

  if(METADATANETDEF_DEFINITIONS(arg_node) != NULL) {
    TRAVdo(METADATANETDEF_DEFINITIONS(arg_node), inf);
  }

  METADATANETDEF_KEYS(arg_node) = INFO_KEYS(inf);

  INFO_KEYS(inf) = NULL;

  METADATANETDEF_CHILDS(arg_node) = INFO_CHILDS(inf);

  INFO_CHILDS(inf) = NULL;

  if(METADATANETDEF_DEFINITIONS(arg_node) != NULL) {
    METADATANETDEF_DEFINITIONS(arg_node) = FREEdoFreeTree(METADATANETDEF_DEFINITIONS(arg_node));
    METADATANETDEF_DEFINITIONS(arg_node) = NULL;
  }

  inf = infoFree(inf);

  INFO_TYPE(arg_info) = TYPE_child;
    
  DBUG_RETURN(arg_node);
}

node *MDPREPROCmetadatalist( node *arg_node, info *arg_info )
{
  node *temp;
  node *last;

  DBUG_ENTER("MDPREPROCmetadatalist");

  if(METADATALIST_ENTRY(arg_node) != NULL) {
    TRAVdo(METADATALIST_ENTRY(arg_node), arg_info);
  }

  switch(INFO_TYPE(arg_info)) {
  case TYPE_child:

    /* This is needed to keep the metadata in correct order! */

    temp = INFO_CHILDS(arg_info);

    if(temp == NULL) {
      
      INFO_CHILDS(arg_info) = TBmakeMetadatadefs(METADATALIST_ENTRY(arg_node), NULL);
      NODE_FILE(INFO_CHILDS(arg_info)) = NODE_FILE(METADATALIST_ENTRY(arg_node));
      NODE_LINE(INFO_CHILDS(arg_info)) = NODE_LINE(METADATALIST_ENTRY(arg_node));
      NODE_COL(INFO_CHILDS(arg_info)) = NODE_COL(METADATALIST_ENTRY(arg_node));

    } else {
      
      while(temp != NULL) {
	last = temp;
	temp = METADATADEFS_NEXT(temp);
      }

      METADATADEFS_NEXT(last) = TBmakeMetadatadefs(METADATALIST_ENTRY(arg_node), NULL);
      NODE_FILE(METADATADEFS_NEXT(last)) = NODE_FILE(METADATALIST_ENTRY(arg_node));
      NODE_LINE(METADATADEFS_NEXT(last)) = NODE_LINE(METADATALIST_ENTRY(arg_node));
      NODE_COL(METADATADEFS_NEXT(last)) = NODE_COL(METADATALIST_ENTRY(arg_node));
    }
    
    METADATALIST_ENTRY(arg_node) = NULL;
    break;
  case TYPE_key:
    
    /* This is needed to keep the metadata in correct order! */

    temp = INFO_KEYS(arg_info);

    if(temp == NULL) {
      
      INFO_KEYS(arg_info) = METADATALIST_ENTRY(arg_node);

    } else {
      
      while(temp != NULL) {
	last = temp;
	temp = METADATAKEYLIST_NEXT(temp);
      }

      METADATAKEYLIST_NEXT(last) = METADATALIST_ENTRY(arg_node);
    }
    
    METADATALIST_ENTRY(arg_node) = NULL;

    break;  
  case TYPE_unknown:
  default:
    break;
  }

  if(METADATALIST_NEXT(arg_node) != NULL) {
    TRAVdo(METADATALIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDPREPROCmetadatakeylist( node *arg_node, info *arg_info )
{

  DBUG_ENTER("MDPREPROCmetadatakeylist");

  INFO_TYPE(arg_info) = TYPE_key;

  DBUG_RETURN(arg_node);
}

node *MDPREPROCmetadataboxdef( node *arg_node, info *arg_info )
{

  DBUG_ENTER("MDPREPROCmetadataboxdef");

  INFO_TYPE(arg_info) = TYPE_child;

  DBUG_RETURN(arg_node);
}

node *MDPREPROCmetadatadefaultdef( node *arg_node, info *arg_info )
{

  DBUG_ENTER("MDPREPROCmetadatadefaultdef");

  INFO_TYPE(arg_info) = TYPE_child;

  DBUG_RETURN(arg_node);
}

node *MDdoPreproc(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("MDdoPreproc");

  DBUG_ASSERT((syntax_tree != NULL), "MDdoPreproc called with empty syntaxtree");
  
  TRAVpush(TR_mdpreproc);

  inf = infoMake();

  syntax_tree = TRAVdo(syntax_tree, inf);

  inf = infoFree(inf);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

