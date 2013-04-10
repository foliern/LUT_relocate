/*******************************************************************************
 *
 * $Id: munpack.c 2507 2009-08-05 12:02:11Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   30.07.2009
 * -----
 *
 * Unpack metadata to NetDef and BoxDef nodes.
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

#include "munpack.h"

typedef struct dequeue_element {
  char *name;
  node *def;
  struct dequeue_element *next;
} dequeue_element_t;

typedef struct dequeue {
  dequeue_element_t *first;
  dequeue_element_t *last;
} dequeue_t;


struct INFO { 
  dequeue_t *net_dequeue;
  dequeue_t *box_dequeue;
  node *childs;
};

#define INFO_NETDEQUEUE(n)   (n->net_dequeue)
#define INFO_BOXDEQUEUE(n)   (n->box_dequeue)
#define INFO_CHILDS(n)       (n->childs)

static dequeue_element_t *dequeueElementCreate(node *def, char *name) 
{
  dequeue_element_t *result;

  result = MEMmalloc(sizeof(dequeue_element_t));

  result->name = name;
  result->def = def;

  result->next = NULL;

  return result;
}

static void dequeueElementDestroy(dequeue_element_t *element) 
{
  MEMfree(element);
}

static dequeue_element_t *dequeueElementFree(dequeue_element_t *element) 
{
  dequeue_element_t *temp;
  
  temp = element->next;

  CTIwarnNode(CTI_ERRNO_DEAD_CODE,
	      element->def, 
	      "Discarding unused metadata (name=\"%s\")", 
	      element->name);

  FREEdoFreeTree(element->def);

  MEMfree(element);

  return temp;
}

static dequeue_t *dequeueCreate() 
{
  dequeue_t *result;

  result = MEMmalloc(sizeof(dequeue_t));

  result->first = NULL;
  result->last = NULL;

  return result;
}

static void dequeueDestroy(dequeue_t *dequeue) 
{
  dequeue_element_t *temp;

  temp = dequeue->first;
  
  while(temp != NULL) {
    
    temp = dequeueElementFree(temp);
  } 

  MEMfree(dequeue);
}


static void dequeuePushBack(dequeue_t *dequeue, dequeue_element_t *element) 
{
  if(dequeue->first == NULL) {
    dequeue->first = element;
    dequeue->last = element;
  } else {
    dequeue->last->next = element;
    dequeue->last = element;
    element->next = NULL;
  }
}

static node *dequeuePopNextMatch(dequeue_t *dequeue, char *name)
{
  dequeue_element_t *element = dequeue->first;
  dequeue_element_t *last = NULL;
  node *temp = NULL;

  while(element != NULL) {

    if((strlen(name) == strlen( element->name))
       && (strcmp(name, element->name) == 0)) {
      
      // remove from dequeue
      
      if(last == NULL) {
	dequeue->first = element->next;
	
	if(dequeue->first == NULL) {
	  dequeue->last = NULL;
	}

      } else {
	last->next = element->next;

	if(element == dequeue->last) {
	  dequeue->last = last;
	}
      }

      temp = element->def;

      dequeueElementDestroy(element);

      return temp;
    }

    last = element;
    element = element->next;
  }

  return NULL;
}
static void infoClear(info *inf)
{
  INFO_NETDEQUEUE(inf) = dequeueCreate();
  INFO_BOXDEQUEUE(inf) = dequeueCreate();
  INFO_CHILDS(inf) = NULL;
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

  dequeueDestroy(INFO_NETDEQUEUE(inf));
  dequeueDestroy(INFO_BOXDEQUEUE(inf));

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *MDUNPACKmodule( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDUNPACKmodule");

  if(MODULE_DEFS(arg_node) != NULL) {
    MODULE_DEFS(arg_node) = TRAVdo(MODULE_DEFS(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDUNPACKdefs( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("MDUNPACKmodule");

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

node *MDUNPACKnetdef( node *arg_node, info *arg_info )
{
  node *temp;
  node *keys;
  node *childs;

  DBUG_ENTER("MDUNPACKnetdef");

  temp = dequeuePopNextMatch(INFO_NETDEQUEUE(arg_info), NETDEF_NAME(arg_node));

  if(temp != NULL) {
    NETDEF_METADATA(arg_node) = METADATANETDEF_KEYS(temp);

    METADATANETDEF_KEYS(temp) = NULL;

    INFO_CHILDS(arg_info) = METADATANETDEF_CHILDS(temp);

    METADATANETDEF_CHILDS(temp) = NULL;

    FREEdoFreeTree(temp);

    while((temp = dequeuePopNextMatch(INFO_NETDEQUEUE(arg_info), NETDEF_NAME(arg_node))) != NULL) { 
      
      keys = NETDEF_METADATA(arg_node);

      if(keys != NULL) {
	while(METADATAKEYLIST_NEXT(keys) != NULL) {
	  keys = METADATAKEYLIST_NEXT(keys);
	}
      
	METADATAKEYLIST_NEXT(keys) = METADATANETDEF_KEYS(temp);

	METADATANETDEF_KEYS(temp) = NULL;
      }

      childs = INFO_CHILDS(arg_info);

      if(childs != NULL) {

	while(METADATADEFS_NEXT(childs) != NULL) {
	  childs = METADATADEFS_NEXT(childs);
	}

	METADATADEFS_NEXT(childs) = METADATANETDEF_CHILDS(temp);

	METADATANETDEF_CHILDS(temp) = NULL;
      }

      FREEdoFreeTree(temp);

    }
  }

  if(NETDEF_BODY(arg_node) != NULL) {
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDUNPACKboxdef( node *arg_node, info *arg_info )
{
  node *temp;
  node *keys;

  DBUG_ENTER("MDUNPACKboxdef");

  temp = dequeuePopNextMatch(INFO_BOXDEQUEUE(arg_info), BOXDEF_NAME(arg_node));

  if(temp != NULL) {
    BOXDEF_METADATA(arg_node) = METADATABOXDEF_KEYS(temp);

    METADATABOXDEF_KEYS(temp) = NULL;

    FREEdoFreeTree(temp);

    while((temp = dequeuePopNextMatch(INFO_BOXDEQUEUE(arg_info), BOXDEF_NAME(arg_node))) != NULL) { 
      
      keys = BOXDEF_METADATA(arg_node);

      while(METADATAKEYLIST_NEXT(keys) != NULL) {
	keys = METADATAKEYLIST_NEXT(keys);
      }
      
      METADATAKEYLIST_NEXT(keys) = METADATABOXDEF_KEYS(temp);

      METADATABOXDEF_KEYS(temp) = NULL;

      FREEdoFreeTree(temp);
    }
  }

  DBUG_RETURN(arg_node);
}

node *MDUNPACKnetbody( node *arg_node, info *arg_info )
{
  info *inf;

  DBUG_ENTER("MDUNPACKnetbody");

  inf = infoMake(); 

  if(INFO_CHILDS(arg_info) != NULL) {
    NETBODY_DEFS(arg_node) = TBmakeDefs(INFO_CHILDS(arg_info), NETBODY_DEFS(arg_node));

    INFO_CHILDS(arg_info) = NULL;
  }

  if(NETBODY_DEFS(arg_node) != NULL) {
    NETBODY_DEFS(arg_node) = TRAVdo(NETBODY_DEFS(arg_node), inf);
  }

  infoFree(inf);


  DBUG_RETURN(arg_node);
}

node *MDUNPACKmetadatadefs( node *arg_node, info *arg_info )
{
  node *temp;

  DBUG_ENTER("MDUNPACKmetadatadefs");

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

node *MDUNPACKmetadatanetdef( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDUNPACKmetadatanetdef");

  dequeuePushBack(INFO_NETDEQUEUE(arg_info), dequeueElementCreate(arg_node, METADATANETDEF_NAME(arg_node))); 

  arg_node = NULL;

  DBUG_RETURN(arg_node);
}

node *MDUNPACKmetadataboxdef( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDUNPACKmetadataboxdef");

  dequeuePushBack(INFO_BOXDEQUEUE(arg_info), dequeueElementCreate(arg_node, METADATABOXDEF_NAME(arg_node))); 

  arg_node = NULL;

  DBUG_RETURN(arg_node);
}

node *MDdoUnpack(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("MDdoUnpack");

  DBUG_ASSERT((syntax_tree != NULL), "MDdoUnpack called with empty syntaxtree");
  
  TRAVpush(TR_mdunpack);

  inf = infoMake();

  syntax_tree = TRAVdo(syntax_tree, inf);

  inf = infoFree(inf);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

