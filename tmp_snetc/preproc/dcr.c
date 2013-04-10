/*******************************************************************************
 *
 * $Id: dcr.c 2507 2009-08-05 12:02:11Z jju $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   14.02.2007
 * -----
 *
 *******************************************************************************/

#include "dcr.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "str.h"
#include "free.h"
#include "string.h"

struct INFO
{
  bool nothing_removed;
  bool top_level_net_found;
  int depth;
};

#define INFO_NOTHING_REMOVED(i) (i->nothing_removed)
#define INFO_TOP_LEVEL_NET_FOUND(i) (i->top_level_net_found)
#define INFO_DEPTH(i) (i->depth)

static info *infoMake(void)
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_NOTHING_REMOVED(result) = FALSE;
  INFO_TOP_LEVEL_NET_FOUND(result) = FALSE;
  INFO_DEPTH(result) = 0;
  
  DBUG_RETURN(result);
}

/* Releases an info struct. */
static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

node *PREPDCRboxdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRboxdef");

  if(BOXDEF_ISNOTREFERRED(arg_node)) {
    CTIwarnNode(CTI_ERRNO_DEAD_CODE,
		arg_node,
		"Unused box %s (will be removed)\n", BOXDEF_NAME(arg_node));
    FREEdoFreeTree(arg_node);
    arg_node = NULL;
    INFO_NOTHING_REMOVED(arg_info) = FALSE;
  } else{
    BOXDEF_ISNOTREFERRED(arg_node) = TRUE;
  }
  
  DBUG_RETURN(arg_node);
}

node *PREPDCRboxref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRboxref");

  BOXDEF_ISNOTREFERRED(BOXREF_BOX(arg_node)) = FALSE;

  DBUG_RETURN(arg_node);
}

node *PREPDCRdefs( node *arg_node, info *arg_info )
{
  node *temp = NULL;
  DBUG_ENTER("PREPDCRdef");

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

node *PREPDCRnetdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRnetdef");
  
  if(INFO_DEPTH(arg_info) == 0 && STReq(NETDEF_NAME(arg_node), global.filebase)) {
    NETDEF_ISNOTREFERRED(arg_node) = FALSE;
    INFO_TOP_LEVEL_NET_FOUND(arg_info) = TRUE;
    NETDEF_TOPLEVEL(arg_node) = TRUE;
  }
  
  if(NETDEF_SIGN(arg_node) != NULL) {
    NETDEF_SIGN(arg_node) = TRAVdo(NETDEF_SIGN(arg_node), arg_info);
  }
  
  if(NETDEF_BODY(arg_node) != NULL) {
    INFO_DEPTH(arg_info)++;
    NETDEF_BODY(arg_node) = TRAVdo(NETDEF_BODY(arg_node), arg_info);
    INFO_DEPTH(arg_info)--;
  }

  if(NETDEF_ISNOTREFERRED(arg_node)) {
    CTIwarnNode(CTI_ERRNO_DEAD_CODE,
		arg_node,
		"Unused net %s (will be removed)\n", NETDEF_NAME(arg_node));
    
    FREEdoFreeTree(arg_node);
    arg_node = NULL;
    INFO_NOTHING_REMOVED(arg_info) = FALSE;
  } else{
    NETDEF_ISNOTREFERRED(arg_node) = TRUE;
  }

  DBUG_RETURN(arg_node);
}

node *PREPDCRnetrefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRnetrefs");

  NETDEF_ISNOTREFERRED( NETREFS_NET(arg_node)) = FALSE;

  DBUG_RETURN(arg_node);
}


node *PREPDCRtypedef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRtypedef");

  if(TYPEDEF_TYPE(arg_node) != NULL){
    TYPEDEF_TYPE(arg_node) =  TRAVdo(TYPEDEF_TYPE(arg_node), arg_info);
  }

  if(TYPEDEF_ISNOTREFERRED(arg_node)) {
    CTIwarnNode(CTI_ERRNO_DEAD_CODE,
		arg_node,
		"Unused type %s (will be removed)\n", TYPEDEF_NAME(arg_node));
    FREEdoFreeTree(arg_node);
    arg_node = NULL;
    INFO_NOTHING_REMOVED(arg_info) = FALSE;
  } else {   
    TYPEDEF_ISNOTREFERRED(arg_node) = TRUE;
  }

  DBUG_RETURN(arg_node);
}

node *PREPDCRtyperef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRtyperef");

  TYPEDEF_ISNOTREFERRED(TYPEREF_TYPE(arg_node)) = FALSE;

  DBUG_RETURN(arg_node);
}

node *PREPDCRtypesigdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRtypesigdef");

  if(TYPESIGDEF_TYPESIGN(arg_node) != NULL) {
    TYPESIGDEF_TYPESIGN(arg_node) = TRAVdo(TYPESIGDEF_TYPESIGN(arg_node), arg_info);
  }

  if(TYPESIGDEF_ISNOTREFERRED(arg_node)) {
    CTIwarnNode(CTI_ERRNO_DEAD_CODE,
		arg_node,
		"Unused typesignature %s (will be removed)\n", TYPESIGDEF_NAME(arg_node));
    FREEdoFreeTree(arg_node);
    arg_node = NULL;
    INFO_NOTHING_REMOVED(arg_info) = FALSE;
  } else {
    TYPESIGDEF_ISNOTREFERRED(arg_node) = TRUE;
  }

  DBUG_RETURN(arg_node);
}

node *PREPDCRtypesigref(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPDCRtypesigref");
  
  TYPESIGDEF_ISNOTREFERRED(TYPESIGREF_TYPESIG(arg_node)) = FALSE;
  
  DBUG_RETURN(arg_node);
}

node *PREPdoRemove(node *syntax_tree)
{
  info *arg_info;

  DBUG_ENTER("PREPdoRemove");

  DBUG_ASSERT((syntax_tree != NULL), "PREPdoRemove called with empty syntaxtree");

  TRAVpush(TR_prepdcr);
  
  arg_info = infoMake();

  while(!INFO_NOTHING_REMOVED(arg_info)) {
    INFO_NOTHING_REMOVED(arg_info) = TRUE; 
    
    // First one marks, second removes
    syntax_tree = TRAVdo(syntax_tree, arg_info);
    syntax_tree = TRAVdo(syntax_tree, arg_info);
 
  }

  if(INFO_TOP_LEVEL_NET_FOUND(arg_info) == FALSE) {
    CTIerror(CTI_ERRNO_PARSING_ERROR,
	     "Top-level net definition not found!");
  }  

  arg_info = infoFree(arg_info);

  TRAVpop();

  DBUG_RETURN(syntax_tree);
}
