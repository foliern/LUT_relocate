/*******************************************************************************
 *
 * $Id: mprogdef.c 2483 2009-07-30 06:49:20Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   30.07.2009
 * -----
 *
 * Propagates default metadata elements to all boxes and nets
 *
 *******************************************************************************/

#include <string.h>
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "free.h"
#include "copy.h"
#include "memory.h"
#include "str.h"
#include "bool.h"
#include "mutil.h"

#include "mprogdef.h"

struct INFO { 
  node *boxdefault;
  node *netdefault;
};

#define INFO_NETDEFAULT(n)   (n->netdefault)
#define INFO_BOXDEFAULT(n)   (n->boxdefault)

static void infoClear(info *inf)
{
  INFO_NETDEFAULT(inf) = NULL;
  INFO_BOXDEFAULT(inf) = NULL;
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

  if(INFO_NETDEFAULT(inf)) {
    FREEdoFreeTree(INFO_NETDEFAULT(inf));
  }
  
  if(INFO_BOXDEFAULT(inf) != NULL) {
    FREEdoFreeTree(INFO_BOXDEFAULT(inf));
  }

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

static node *add(node *a, node *b)
{
  node *temp;

  if(a == NULL) {
    return b;
  }

  if(b == NULL) {
    return a;
  }

  temp = a;

  while(METADATAKEYLIST_NEXT(temp) != NULL) {
    temp = METADATAKEYLIST_NEXT(temp);
  }  

  METADATAKEYLIST_NEXT(temp) = b;

  return a;
}

node *MDPROGDEFmodule( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDPROGDEFmodule");

  if(MODULE_DEFS(arg_node) != NULL) {
    MODULE_DEFS(arg_node) = TRAVdo(MODULE_DEFS(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDPROGDEFdefs( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("MDPROGDEFmodule");

  if(DEFS_DEF(arg_node) != NULL) {
    DEFS_DEF(arg_node) = TRAVdo(DEFS_DEF(arg_node), arg_info);
  }

  if(DEFS_NEXT(arg_node) != NULL) {
    DEFS_NEXT(arg_node) = TRAVdo(DEFS_NEXT(arg_node), arg_info);
  }

  if(DEFS_DEF(arg_node) == NULL) {

    temp = DEFS_NEXT(arg_node);

    FREEdoFreeNode(arg_node);

    arg_node = temp;
  }

  DBUG_RETURN(arg_node);
}

node *MDPROGDEFnetdef( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDPROGDEFnetdef");

  if(INFO_NETDEFAULT(arg_info) != NULL) {
    NETDEF_METADATA(arg_node) = add(COPYdoCopyTree(INFO_NETDEFAULT(arg_info)), NETDEF_METADATA(arg_node));
  }

  /* Add original name key *
   * Used by observers to find out the original net name
   */

  NETDEF_METADATA(arg_node) = TBmakeMetadatakeylist(STRcpy(METADATA_KEY_SNET_ORIGINAL_NAME), STRcpy(NETDEF_NAME(arg_node)), NETDEF_METADATA(arg_node));

  if(NETDEF_BODY(arg_node) != NULL) {
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDPROGDEFboxdef( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MbPROGDEFoxdef");

  if(INFO_BOXDEFAULT(arg_info) != NULL) {
    BOXDEF_METADATA(arg_node) = add(COPYdoCopyTree(INFO_BOXDEFAULT(arg_info)), BOXDEF_METADATA(arg_node));
  }

  /* Add original name key *
   * Used by observers to find out the original box name
   */

  BOXDEF_METADATA(arg_node) = TBmakeMetadatakeylist(STRcpy(METADATA_KEY_SNET_ORIGINAL_NAME), STRcpy(BOXDEF_REALNAME(arg_node)), BOXDEF_METADATA(arg_node));

  DBUG_RETURN(arg_node);
}

node *MDPROGDEFnetbody( node *arg_node, info *arg_info )
{
  info *inf;

  DBUG_ENTER("MDPROGDEFnetbody");

  inf = infoMake(); 

  INFO_NETDEFAULT(inf) = COPYdoCopyTree(INFO_NETDEFAULT(arg_info));
  INFO_BOXDEFAULT(inf) = COPYdoCopyTree(INFO_BOXDEFAULT(arg_info));

  if(NETBODY_DEFS(arg_node) != NULL) {
    NETBODY_DEFS(arg_node) = TRAVdo(NETBODY_DEFS(arg_node), inf);
  }

  infoFree(inf);


  DBUG_RETURN(arg_node);
}

node *MDPROGDEFmetadatadefs( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("MDPROGDEFmetadatadefs");

  if(METADATADEFS_DEF(arg_node) != NULL) {
    METADATADEFS_DEF(arg_node) = TRAVdo(METADATADEFS_DEF(arg_node), arg_info);
  }

  if(METADATADEFS_NEXT(arg_node) != NULL) {
    METADATADEFS_NEXT(arg_node) = TRAVdo(METADATADEFS_NEXT(arg_node), arg_info);
  }

  if(METADATADEFS_DEF(arg_node) == NULL) {

    temp = METADATADEFS_NEXT(arg_node);

    METADATADEFS_NEXT(arg_node) = NULL;

    FREEdoFreeTree(arg_node);

    arg_node = temp;
  }

  DBUG_RETURN(arg_node);
}

node *MDPROGDEFmetadatadefaultdef( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDPROGDEFmetadatadefaultddef");

  switch(METADATADEFAULTDEF_TYPE(arg_node)) {
  case MD_box:
    INFO_BOXDEFAULT(arg_info) = add(INFO_BOXDEFAULT(arg_info), METADATADEFAULTDEF_KEYS(arg_node));
    break;
  case MD_net:
    INFO_NETDEFAULT(arg_info) = add(INFO_NETDEFAULT(arg_info), METADATADEFAULTDEF_KEYS(arg_node));
    break;
  case MD_all:
  default:
    INFO_BOXDEFAULT(arg_info) = add(INFO_BOXDEFAULT(arg_info), COPYdoCopyTree(METADATADEFAULTDEF_KEYS(arg_node)));
    INFO_NETDEFAULT(arg_info) = add(INFO_NETDEFAULT(arg_info), METADATADEFAULTDEF_KEYS(arg_node));
    break;
  } 

  METADATADEFAULTDEF_KEYS(arg_node) = NULL;

  FREEdoFreeTree(arg_node);

  arg_node = NULL;

  DBUG_RETURN(arg_node);
}

node *MDdoPropagate(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("MDdoPropagate");

  DBUG_ASSERT((syntax_tree != NULL), "MDdoPropagate called with empty syntaxtree");
  
  TRAVpush(TR_mdprogdef);

  inf = infoMake();

  syntax_tree = TRAVdo(syntax_tree, inf);

  inf = infoFree(inf);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

