/*******************************************************************************
 *
 * $Id: tres.c 2047 2008-07-31 11:13:31Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   20.05.2007
 * -----
 *
 *******************************************************************************/

#include "tres.h"
#include <string.h>
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "ctinfo.h"
#include "free.h"
#include "copy.h"

typedef struct def{
  node *node;
  struct def *next;
}def_t;

static def_t *defs = NULL;

static void addDef(node *arg){
  def_t *temp = (def_t *)MEMmalloc(sizeof(def_t));

  temp->node = arg;
  temp->next = defs;
  defs = temp;
}

static void freeDefs(){
  def_t* temp;

  while(defs != NULL) {

    if(defs->node != NULL) {
      FREEdoFreeTree(defs->node);
    }

    temp = defs;
    defs = defs->next;

    MEMfree(temp);
  }
}

node *PREPTRESdefs(node *arg_node, info *arg_info)
{ 
  node *temp = NULL;

  DBUG_ENTER("PREPTRESdefs");

  if(DEFS_DEF(arg_node) != NULL) {
    DEFS_DEF(arg_node) = TRAVdo(DEFS_DEF(arg_node), arg_info);
  }  

  if(DEFS_NEXT(arg_node) != NULL) {
    DEFS_NEXT(arg_node) = TRAVdo(DEFS_NEXT(arg_node), arg_info);    
  }

  
  if(DEFS_DEF(arg_node) == NULL) {

    temp = DEFS_NEXT(arg_node);
    DEFS_NEXT(arg_node) = NULL;

    FREEdoFreeNode(arg_node);
    arg_node = temp;
  }
 
  DBUG_RETURN(arg_node);
}

node *PREPTREStypedef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPTREStypedef");

  if(TYPEDEF_TYPE(arg_node) != NULL) {
    TYPEDEF_TYPE(arg_node) = TRAVdo(TYPEDEF_TYPE(arg_node), arg_info);
  }

  addDef(arg_node);
  arg_node = NULL;  

  DBUG_RETURN(arg_node);
}

node *PREPTREStyperef(node *arg_node, info *arg_info)
{
  node *temp = NULL;

  DBUG_ENTER("PREPTREStyperef");

  temp = TYPEREF_TYPE(arg_node);

  if(temp != NULL) {
    temp = COPYdoCopyTree((TYPEDEF_TYPE(temp)));
  }

  DBUG_RETURN(temp);
}

node *PREPTREStypes(node *arg_node, info *arg_info)
{
  node *temp = NULL;
  node *type = NULL;
  node *list = NULL;
  node *tmp  = NULL;

  DBUG_ENTER("PREPTREStypes");

   if(TYPES_TYPE(arg_node) != NULL) {
     temp = TYPES_TYPE(arg_node);

     type = TRAVdo(TYPES_TYPE(arg_node), arg_info);

     if(type != temp){
       list = TYPES_NEXT(arg_node);

       tmp = type;

       while(TYPES_NEXT(tmp) != NULL) {
	 tmp = TYPES_NEXT(tmp);
       }

       TYPES_NEXT(tmp) = list;

       FREEdoFreeNode(arg_node);

       arg_node = type;

     }
   }

   if(TYPES_NEXT(arg_node) != NULL) {
     TYPES_NEXT(arg_node) = TRAVdo(TYPES_NEXT(arg_node), arg_info);
   }

   DBUG_RETURN(arg_node);
}

node *PREPTREStypesigdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPTREStypesigdef");

  if(TYPESIGDEF_TYPESIGN(arg_node) != NULL) {
    TYPESIGDEF_TYPESIGN(arg_node) = TRAVdo(TYPESIGDEF_TYPESIGN(arg_node), arg_info);
  }
  
  addDef(arg_node);  
  arg_node = NULL;
  
  DBUG_RETURN(arg_node);
}

node *PREPTREStypesigref(node *arg_node, info *arg_info)
{
  node *temp = NULL;

  DBUG_ENTER("PREPTREStypesigref"); 

  temp = TYPESIGREF_TYPESIG(arg_node);
  
  if(temp != NULL){
    if(TYPESIGDEF_TYPESIGN(temp) != NULL) {
      temp = COPYdoCopyTree(TYPESIGDEF_TYPESIGN(temp)); 
    }
  }

  FREEdoFreeTree(arg_node);

  arg_node = NULL;

  DBUG_RETURN(temp);
}

node *PREPdoTypeRes(node *syntax_tree)
{
  DBUG_ENTER("PREPdoTypeRes");

  DBUG_ASSERT((syntax_tree != NULL), "PREPdoTypeRes called with empty syntaxtree");

  TRAVpush(TR_preptres);  

  syntax_tree = TRAVdo(syntax_tree, NULL);

  freeDefs();
  
  TRAVpop();

  DBUG_RETURN(syntax_tree);
}
