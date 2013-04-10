/*******************************************************************************
 *
 * $Id: mcollect.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   29.07.2009
 * -----
 *
 * Gathers information about the metadata
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
#include "bool.h"
#include "mutil.h"

#include "mcollect.h"

struct INFO {
  bool has_observers;
  node *interfaces;
  int num_interfaces;
};

#define INFO_HAS_OBSERVERS(n)       (n->has_observers)
#define INFO_INTERFACES(n)          (n->interfaces)
#define INFO_NUM_INTERFACES(n)      (n->num_interfaces)

static void infoClear(info *inf)
{
  INFO_HAS_OBSERVERS(inf) = FALSE;
  INFO_INTERFACES(inf) = NULL;
  INFO_NUM_INTERFACES(inf) = 0;
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

static void add_interface(info *inf, const char *name) 
{
  node *temp = INFO_INTERFACES(inf);
  node *last = NULL;

  while(temp != NULL) {
    if(STReq(LANGUAGEINTERFACES_NAME(temp), name)) {
      return;
    }

    last = temp;
    temp = LANGUAGEINTERFACES_NEXT(temp);
  }

  if(last == NULL) {
    INFO_INTERFACES(inf) = TBmakeLanguageinterfaces(STRcpy(name), NULL);
  } else {
    LANGUAGEINTERFACES_NEXT(last) = TBmakeLanguageinterfaces(STRcpy(name), NULL);
  }

  INFO_NUM_INTERFACES(inf) += 1;
}

node *MDCOLLECTmodule( node *arg_node, info *arg_info )
{

  DBUG_ENTER("MDCOLLECTmodule");

  if(MODULE_DEFS(arg_node) != NULL) {
    MODULE_DEFS(arg_node) = TRAVdo(MODULE_DEFS(arg_node), arg_info);
  }

  MODULE_HASOBSERVERS(arg_node) = INFO_HAS_OBSERVERS(arg_info);

  MODULE_INTERFACES(arg_node) = INFO_INTERFACES(arg_info);

  if ( 0 == INFO_NUM_INTERFACES(arg_info)) {
    CTIwarn(CTI_ERRNO_INTERFACE_ERROR, "No metadata interface information could be collected!");
  }

  DBUG_RETURN(arg_node);
}

node *MDCOLLECTmetadatakeylist( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDCOLLECTmetadatakeylist");

  if(STReq(METADATAKEYLIST_KEY(arg_node), METADATA_KEY_BOX_LANGUAGE_INTERFACE)) {

    add_interface(arg_info, METADATAKEYLIST_VALUE(arg_node));

  } else if(STReq(METADATAKEYLIST_KEY(arg_node), METADATA_KEY_OBSERVER) 
	    && (METADATAKEYLIST_VALUE(arg_node) != NULL)) {

    INFO_HAS_OBSERVERS(arg_info) = TRUE;
  }

  if(METADATAKEYLIST_NEXT(arg_node) != NULL) {
    METADATAKEYLIST_NEXT(arg_node) = TRAVdo(METADATAKEYLIST_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}


node *MDdoCollect(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("MDdoCollect");

  DBUG_ASSERT((syntax_tree != NULL), "MDdoCollect called with empty syntaxtree");
  
  TRAVpush(TR_mdcollect);

  inf = infoMake();

  syntax_tree = TRAVdo(syntax_tree, inf);

  inf = infoFree(inf);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

