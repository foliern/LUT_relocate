/*******************************************************************************
 *
 * $Id: mredist.c 2507 2009-08-05 12:02:11Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   20.05.2007
 * -----
 *
 * Moving metadata to the same scope where the corresponding component
 * is declared
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

#include "mredist.h"

typedef struct dequeue_element {
  char **path;
  node *def;
  struct dequeue_element *next;
} dequeue_element_t;

typedef struct dequeue {
  dequeue_element_t *first;
  dequeue_element_t *last;
} dequeue_t;


struct INFO { 
  char **path;
  dequeue_t *dequeue;
  char *fullpath;
  node *defs;
};

#define INFO_PATH(n)      (n->path)
#define INFO_DEQUEUE(n)   (n->dequeue)
#define INFO_DEFS(n)      (n->defs)
#define INFO_FULLPATH(n)  (n->fullpath)

static bool isPath(const char *path)
{
  return (strchr(path, '/') != NULL) ? TRUE : FALSE;
}

static bool equalsPath(const char *name, const char *path){
  char *c = strchr(path, '/');

  if(c != NULL) {

    if((strncmp(name, path, strlen(name)) == 0)
       && (strlen(name) == (c - path))){
      return TRUE;
    }
  }
  
  return FALSE;
}

static char *cutPath(const char *path){
  char *c = strchr(path, '/');

  if(c != NULL) { 
    return STRcpy(c + 1);
  }
  
  return NULL;
}

static dequeue_element_t * dequeueElementCreate(node *def, char **path) 
{
  dequeue_element_t *result;

  result = MEMmalloc(sizeof(dequeue_element_t));

  result->path = path;
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
	      *(element->path));

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

static node *dequeuePopNextMatch(dequeue_t * dequeue, char *name)
{
  char *str = NULL;
  dequeue_element_t *element = dequeue->first;
  dequeue_element_t *last = NULL;
  node *temp = NULL;

  while(element != NULL) {

    if(equalsPath(name, *(element->path))) {

      str = cutPath(*(element->path));
      
      MEMfree(*(element->path));
      
      *(element->path) = str;
      
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
  INFO_PATH(inf) = NULL;
  INFO_DEQUEUE(inf) = dequeueCreate();
  INFO_DEFS(inf) = NULL;
  INFO_FULLPATH(inf) = NULL;
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

  dequeueDestroy(INFO_DEQUEUE(inf));

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}


node *MDREDISTmodule( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDREDISTmodule");

  if(MODULE_DEFS(arg_node) != NULL) {
    MODULE_DEFS(arg_node) = TRAVdo(MODULE_DEFS(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDREDISTdefs( node *arg_node, info *arg_info )
{
  DBUG_ENTER("MDREDISTmodule");

  // Handle removed defs!

  if(DEFS_DEF(arg_node) != NULL) {
    DEFS_DEF(arg_node) = TRAVdo(DEFS_DEF(arg_node), arg_info);
  }

  if(DEFS_NEXT(arg_node) != NULL) {
    DEFS_NEXT(arg_node) = TRAVdo(DEFS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDREDISTnetdef( node *arg_node, info *arg_info )
{
  node *temp;
  node *last;

  DBUG_ENTER("MDREDISTnetdef");

  INFO_DEFS(arg_info) = dequeuePopNextMatch(INFO_DEQUEUE(arg_info), NETDEF_NAME(arg_node));

  if(INFO_DEFS(arg_info) != NULL) {

    last = INFO_DEFS(arg_info);

    while((temp = dequeuePopNextMatch(INFO_DEQUEUE(arg_info), NETDEF_NAME(arg_node))) != NULL) { 
      METADATADEFS_NEXT(last) = temp;
      last = temp;
    }
  }

  if(NETDEF_BODY(arg_node) != NULL) {
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *MDREDISTnetbody( node *arg_node, info *arg_info )
{
  info *inf;

  DBUG_ENTER("MDREDISTnetbody");

  if(INFO_DEFS(arg_info) != NULL) {

    NETBODY_DEFS(arg_node) = TBmakeDefs(INFO_DEFS(arg_info), NETBODY_DEFS(arg_node));

    INFO_DEFS(arg_info) = NULL;
  }

  inf = infoMake(); 

  if(NETBODY_DEFS(arg_node) != NULL) {
    NETBODY_DEFS(arg_node) = TRAVdo(NETBODY_DEFS(arg_node), inf);
  }

  infoFree(inf);


  DBUG_RETURN(arg_node);
}

node *MDREDISTmetadatadefs( node *arg_node, info *arg_info )
{
  char **path;
  node *temp;

  DBUG_ENTER("MDREDISTmetadatadefs");

  if(METADATADEFS_DEF(arg_node) != NULL) {

    INFO_PATH(arg_info) = NULL;

    METADATADEFS_DEF(arg_node) = TRAVdo(METADATADEFS_DEF(arg_node), arg_info);

    if(INFO_PATH(arg_info) != NULL) {
      dequeuePushBack(INFO_DEQUEUE(arg_info), dequeueElementCreate(arg_node, INFO_PATH(arg_info)));
    }

    path = INFO_PATH(arg_info);
    INFO_PATH(arg_info) = NULL;
  }

  if(METADATADEFS_NEXT(arg_node) != NULL) {
    METADATADEFS_NEXT(arg_node) = TRAVdo(METADATADEFS_NEXT(arg_node), arg_info);
  }

  if(path != NULL) {

    temp = METADATADEFS_NEXT(arg_node);

    METADATADEFS_NEXT(arg_node) = NULL;

    arg_node = temp;
  }

  DBUG_RETURN(arg_node);
}

node *MDREDISTmetadatanetdef( node *arg_node, info *arg_info )
{
  char *temp;
  char *fullpath;

  DBUG_ENTER("MDREDISTmetadatanetdef");

  if(INFO_FULLPATH(arg_info) != NULL) {
    temp = STRcatn(3, INFO_FULLPATH(arg_info), "/", METADATANETDEF_NAME(arg_node));
    
    MEMfree(METADATANETDEF_NAME(arg_node));
    
    METADATANETDEF_NAME(arg_node) = temp;
  }
  
  if(isPath(METADATANETDEF_NAME(arg_node))) {
    INFO_PATH(arg_info) = &METADATANETDEF_NAME(arg_node);
  } else {

    fullpath = INFO_FULLPATH(arg_info);
    
    INFO_FULLPATH(arg_info) = METADATANETDEF_NAME(arg_node);
    
    if(METADATANETDEF_CHILDS(arg_node) != NULL) {
      METADATANETDEF_CHILDS(arg_node) = TRAVdo(METADATANETDEF_CHILDS(arg_node), arg_info);
    } 
    
    INFO_FULLPATH(arg_info) = fullpath;
  }

  DBUG_RETURN(arg_node);
}

node *MDREDISTmetadataboxdef( node *arg_node, info *arg_info )
{
  char *temp;

  DBUG_ENTER("MDREDISTmetadataboxdef");

  if(INFO_FULLPATH(arg_info) != NULL) {
    temp = STRcatn(3, INFO_FULLPATH(arg_info), "/", METADATABOXDEF_NAME(arg_node));
    
    MEMfree(METADATABOXDEF_NAME(arg_node));
    
    METADATABOXDEF_NAME(arg_node) = temp;
  }

  if(isPath(METADATABOXDEF_NAME(arg_node))) {
    INFO_PATH(arg_info) = &METADATABOXDEF_NAME(arg_node);
  }

  DBUG_RETURN(arg_node);
}

node *MDdoDistribute(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("MDdoDistribute");

  DBUG_ASSERT((syntax_tree != NULL), "MDdoDistribute called with empty syntaxtree");
  
  TRAVpush(TR_mdredist);

  inf = infoMake();

  syntax_tree = TRAVdo(syntax_tree, inf);

  inf = infoFree(inf);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}

