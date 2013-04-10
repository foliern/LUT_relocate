/*******************************************************************************
 *
 * $Id: tcheck.c 2507 2009-08-05 12:02:11Z jju $ 
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   16.08.2007
 * -----
 *
 *******************************************************************************/

#include "tcheck.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include <string.h>

/*
 * INFO structure
 */
struct INFO {
  node *pattern;
  int tags;
};

#define INFO_PATTERN(n) (n->pattern)
#define INFO_TAGS(n) (n->tags)

static info *infoMake()
{
  info *result;

  DBUG_ENTER("infoMake");

  result = MEMmalloc(sizeof(info));

  INFO_PATTERN(result)  = NULL;
  INFO_TAGS(result)     = 0;

  DBUG_RETURN(result);
}

static info *infoFree(info *inf)
{
  DBUG_ENTER("infofree");

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
}

static bool containsField(node *match, node *pattern){
  node *currentEntry = NULL;

  if(match != NULL && pattern != NULL){  
    for(currentEntry = RECTYPE_ENTRIES(pattern); currentEntry != NULL; 
	currentEntry = RECENTRIES_NEXT(currentEntry)){

      if(RECENTRIES_FIELD(currentEntry) != NULL){
	if(strcmp(FIELDREF_NAME(match), FIELDREF_NAME(RECENTRIES_FIELD(currentEntry))) == 0){
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}

static bool containsSTag(node *match, node *pattern){
  node *currentEntry = NULL;

  if(match != NULL && pattern != NULL){     
    for(currentEntry = RECTYPE_ENTRIES(pattern);currentEntry != NULL; 
	currentEntry = RECENTRIES_NEXT(currentEntry)){

      if(RECENTRIES_STAG(currentEntry) != NULL){
	if(strcmp(STAGREF_NAME(match), STAGREF_NAME(RECENTRIES_STAG(currentEntry))) == 0){
	  return TRUE;
	}
      }
    }
    
  }
  return FALSE;
}

static bool containsBTag(node *match, node *pattern){
  node *currentEntry = NULL;

  if(match != NULL && pattern != NULL){        
    for(currentEntry = RECTYPE_ENTRIES(pattern);currentEntry != NULL; 
	currentEntry = RECENTRIES_NEXT(currentEntry)){
      
      if(RECENTRIES_BTAG(currentEntry) != NULL){
	if(strcmp(BTAGREF_NAME(match), BTAGREF_NAME(RECENTRIES_BTAG(currentEntry))) == 0){
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}

static bool evaluatesToInteger(node *expr){
  switch(TAGEXPR_OPERATOR(expr)){

    /* always integer: */
  case OPER_NONE: case OPER_ABS: case OPER_MUL:
  case OPER_DIV:  case OPER_REM: case OPER_ADD: 
  case OPER_SUB:  case OPER_MIN: case OPER_MAX:
    return TRUE;

    /* depends on the children: */
  case OPER_COND: 
    if(evaluatesToInteger(TAGEXPR_LEFT(expr)) && evaluatesToInteger(TAGEXPR_RIGHT(expr))) {
      return TRUE;
    }

    return FALSE;

    /* always boolean: */
  default:
    return FALSE;
  }
  return FALSE;
}


static bool evaluatesToBoolean(node *expr){
  switch(TAGEXPR_OPERATOR(expr)){

    /* always boolean: */
  case OPER_LT:  case OPER_LTQ: case OPER_GT: 
  case OPER_GTQ: case OPER_AND: case OPER_OR: 
  case OPER_NOT: case OPER_EQ:  case OPER_NEQ:
    return TRUE;

    /* depends on the children: */
  case OPER_COND:
    if(evaluatesToBoolean(TAGEXPR_LEFT(expr)) && evaluatesToBoolean(TAGEXPR_RIGHT(expr))) {
      return TRUE;
    } 

    return FALSE;

    /* always integer: */
  default:
    return FALSE;
  }
  return FALSE;
}

static void errorExpectingI(node *arg_node, const char *help){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting integer operand (%s).\n", 
	       help);
}

static void errorExpectingB(node *arg_node, const char *help){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting boolean operand (%s).\n", 
	       help);
}

static void errorExpectingIL(node *arg_node, const char *help){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting integer as left operand (%s).\n", 
	       help);
}

static void errorExpectingIR(node *arg_node, const char *help){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting integer as right operand (%s).\n", 
	       help);
}

static void errorExpectingBL(node *arg_node, const char *help){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting boolean as left operand (%s).\n", 
	       help);
}

static void errorExpectingBR(node *arg_node, const char *help){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting boolean as right operand (%s).\n", 
	       help);
}

static void errorExpectingBC(node *arg_node, const char *help){  
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting boolean as condition operand (%s).\n", 
	       help);
}

static void errorExpectingST(node *arg_node, const char *help){  
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Expecting same types for both operands (%s).\n", 
	       help);
}

static void errorInvalidInitField(node *arg_node, const char *name){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "New field declared without assign (%s).\n", 
	       name);
}

static void errorUnknownField(node *arg_node, const char *name){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Reference to unknown field (%s).\n", 
	       name);
}

static void errorUnknownSTag(node *arg_node, const char *name){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Reference to unknown tag (%s).\n", 
	       name);
}

static void errorUnknownBTag(node *arg_node, const char *name){
  CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	       arg_node,
	       "Reference to unknown token (#%s).\n", 
	       name);
}
 
static void warnStaticGuard(node *arg_node){
  CTIwarnNode(CTI_ERRNO_TYPE_CHECK_ERROR,
	      arg_node,
	      "Static guard expression.\n");
}

node *PREPTCHECKtagexpr(node *arg_node, info *arg_info)
{

  DBUG_ENTER("PREPTCHECKtagexpr");

  switch(TAGEXPR_OPERATOR(arg_node)){
  case OPER_NONE:
    /* Base type, no need to check */

    /* tag names must appear in filters/guardpatterns pattern: */

    if(TAGEXPR_STAG(arg_node) != NULL) {
      INFO_TAGS(arg_info)++;
      if(containsSTag(TAGEXPR_STAG(arg_node), INFO_PATTERN(arg_info)) == FALSE){
	errorUnknownSTag(arg_node, STAGREF_NAME(TAGEXPR_STAG(arg_node)));
      }
    }
    if(TAGEXPR_BTAG(arg_node) != NULL) {
      INFO_TAGS(arg_info)++;
      if(containsBTag(TAGEXPR_BTAG(arg_node), INFO_PATTERN(arg_info)) == FALSE){
	errorUnknownBTag(arg_node, BTAGREF_NAME(TAGEXPR_BTAG(arg_node)));
      }
    }
    break;

    /* Check that the children of the node evaluate to right types.
     * Right type is determined by the operator.
     */
  case OPER_NOT:
    if(evaluatesToBoolean(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingB(arg_node, "!");
    }
    break; 
  case OPER_ABS:
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingI(arg_node, "abs");
    }
    break;
  case OPER_MUL:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "*");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "*");
    }
    break;
  case OPER_DIV:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "/");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "/");
    }
    break;
  case OPER_REM:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "%");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "%");
    }
    break;
  case OPER_ADD:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "+");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "+");
    }
    break;
  case OPER_SUB:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "-");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "-");
    }
    break;
  case OPER_MIN:
     if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "min");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "min");
    }
    break;
  case OPER_MAX:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "max");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "max");
    }
    break;
  case OPER_LT: 
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "<");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "<");
    }
    break;
  case OPER_LTQ: 
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, "<=");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, "<=");
    }
    break;   
  case OPER_GT:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, ">");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, ">");
    }
    break;  
  case OPER_GTQ: 
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingIL(arg_node, ">=");
    }
    if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingIR(arg_node, ">=");
    }
    break;
  
  case OPER_EQ:
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == TRUE){
      if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
	errorExpectingST(arg_node, "==");
      }
    } 
    else{
      if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == TRUE){
	errorExpectingST(arg_node, "==");
      }
    }
    break;
  case OPER_NEQ: 
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == TRUE){
      if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
	errorExpectingST(arg_node, "!=");
      }
    } 
    else{
      if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == TRUE){
	errorExpectingST(arg_node, "!=");
      }
    }
     break; 
  case OPER_AND:
    if(evaluatesToBoolean(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingBL(arg_node, "&&");
    }
    if(evaluatesToBoolean(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingBR(arg_node, "&&");
    }
    break; 
  case OPER_OR:  
    if(evaluatesToBoolean(TAGEXPR_LEFT(arg_node)) == FALSE){
      errorExpectingBL(arg_node, "||");
    }
    if(evaluatesToBoolean(TAGEXPR_RIGHT(arg_node)) == FALSE){
      errorExpectingBR(arg_node, "||");
    }
    break; 
  case OPER_COND:
    if(evaluatesToBoolean(TAGEXPR_CONDITION(arg_node)) == FALSE) {
      errorExpectingBC(arg_node, "?");
    }
    if(evaluatesToInteger(TAGEXPR_LEFT(arg_node)) == TRUE){
      if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == FALSE){
	errorExpectingST(arg_node, ":");
      }
    } 
    else{
      if(evaluatesToInteger(TAGEXPR_RIGHT(arg_node)) == TRUE){
	errorExpectingST(arg_node, ":");
      }
    }
    break;
  default:
    CTIerrorNode(CTI_ERRNO_TYPE_CHECK_ERROR,
		 arg_node, "Unknown operator.\n");
    break; 
  }

  /* Do type check for the children: */

  if(TAGEXPR_CONDITION(arg_node) != NULL) {
    TAGEXPR_CONDITION(arg_node) = TRAVdo(TAGEXPR_CONDITION(arg_node), arg_info);
  }

  if(TAGEXPR_LEFT(arg_node) != NULL) {
    TAGEXPR_LEFT(arg_node) = TRAVdo(TAGEXPR_LEFT(arg_node), arg_info);
  }

  if(TAGEXPR_RIGHT(arg_node) != NULL) {
    TAGEXPR_RIGHT(arg_node) = TRAVdo(TAGEXPR_RIGHT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}


node *PREPTCHECKfilt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPTCHECKfilt");

  /* Store the filter pattern for checks and continue traveling as normal */
  INFO_PATTERN(arg_info) = FILT_PATTERN(arg_node);

  if(FILT_PATTERN(arg_node) != NULL) {
    FILT_PATTERN(arg_node) = TRAVdo(FILT_PATTERN(arg_node), arg_info);
  }  
  if(FILT_GUARDACTIONS(arg_node) != NULL){
    FILT_GUARDACTIONS(arg_node) = TRAVdo(FILT_GUARDACTIONS(arg_node), arg_info);
  }
  
  DBUG_RETURN(arg_node);
}

node *PREPTCHECKguardpatterns(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPTCHECKguardpatterns");

  /* Store the guardpattern pattern for checks and continue traveling as normal */

  INFO_PATTERN(arg_info) = GUARDPATTERNS_ENTRIES(arg_node);

  if(GUARDPATTERNS_ENTRIES(arg_node) != NULL) {
    GUARDPATTERNS_ENTRIES(arg_node) = TRAVdo(GUARDPATTERNS_ENTRIES(arg_node), arg_info);
  }
  if(GUARDPATTERNS_CONDITION(arg_node) != NULL) {

    if(evaluatesToBoolean(GUARDPATTERNS_CONDITION(arg_node)) == FALSE){
	errorExpectingB(arg_node, "if");
    }

    /* if sentence must have expression that evaluates to boolean value */
    INFO_TAGS(arg_info) = 0;
    GUARDPATTERNS_CONDITION(arg_node) = TRAVdo(GUARDPATTERNS_CONDITION(arg_node), arg_info);

    if(INFO_TAGS(arg_info) == 0) {
      warnStaticGuard(arg_node);
    }
  }
  if(GUARDPATTERNS_NEXT(arg_node) != NULL) {
    GUARDPATTERNS_NEXT(arg_node) = TRAVdo(GUARDPATTERNS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}
node *PREPTCHECKguardactions(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPTCHECKguardactions");

  /* Just the normal traveling, with one type check */

  if(GUARDACTIONS_TAGEXPR(arg_node) != NULL) {
    /* if sentence must have expression that evaluates to boolean value */
    if(evaluatesToBoolean(GUARDACTIONS_TAGEXPR(arg_node)) == FALSE){
      errorExpectingB(arg_node, "if");
    }
    INFO_TAGS(arg_info) = 0;
    GUARDACTIONS_TAGEXPR(arg_node) = TRAVdo(GUARDACTIONS_TAGEXPR(arg_node), arg_info);

    if(INFO_TAGS(arg_info) == 0) {
      warnStaticGuard(arg_node);
    }
  } 
     
  if(GUARDACTIONS_ACTION(arg_node) != NULL) {
    GUARDACTIONS_ACTION(arg_node) = TRAVdo(GUARDACTIONS_ACTION(arg_node), arg_info);
  }    

  if(GUARDACTIONS_NEXT(arg_node) != NULL) {
    GUARDACTIONS_NEXT(arg_node) = TRAVdo(GUARDACTIONS_NEXT(arg_node), arg_info);
  }  

  DBUG_RETURN(arg_node);
}


node *PREPTCHECKoutputfields(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PREPTCHECKoutputfields");

  /* Just the normal traveling, with sone type checks */

    if(OUTPUTFIELDS_LEFTFIELD(arg_node) != NULL) {    
      OUTPUTFIELDS_LEFTFIELD(arg_node) = TRAVdo(OUTPUTFIELDS_LEFTFIELD(arg_node), arg_info);    
    }
    
    if(OUTPUTFIELDS_RIGHTFIELD(arg_node) != NULL) {
 
     /* Fields that are renamed/copied/etc. to other fields must have been declared 
       * in the pattern
       */

      if(containsField(OUTPUTFIELDS_RIGHTFIELD(arg_node), INFO_PATTERN(arg_info)) == FALSE) {
	errorUnknownField(arg_node, FIELDREF_NAME(OUTPUTFIELDS_RIGHTFIELD(arg_node)));
      }

      OUTPUTFIELDS_RIGHTFIELD(arg_node) = TRAVdo(OUTPUTFIELDS_RIGHTFIELD(arg_node), arg_info);
    } else {
      if(OUTPUTFIELDS_LEFTFIELD(arg_node) != NULL) {  
	if(containsField(OUTPUTFIELDS_LEFTFIELD(arg_node), INFO_PATTERN(arg_info)) == FALSE) {
	  errorInvalidInitField(arg_node, FIELDREF_NAME(OUTPUTFIELDS_LEFTFIELD(arg_node)));
	}
      }
    }
        
    if(OUTPUTFIELDS_STAG(arg_node) != NULL) {    
      OUTPUTFIELDS_STAG(arg_node) = TRAVdo(OUTPUTFIELDS_STAG(arg_node), arg_info);    
    }    

    if(OUTPUTFIELDS_BTAG(arg_node) != NULL) {    
      OUTPUTFIELDS_BTAG(arg_node) = TRAVdo(OUTPUTFIELDS_BTAG(arg_node), arg_info);    
    }

    if(OUTPUTFIELDS_TAGEXPR(arg_node) != NULL) {

      /* Values assigned to tags (or tags used to rename tags) must evaluate to integer value */

      if(evaluatesToInteger(OUTPUTFIELDS_TAGEXPR(arg_node)) == FALSE){
	errorExpectingIR(arg_node, "=");
      }

      OUTPUTFIELDS_TAGEXPR(arg_node) = TRAVdo(OUTPUTFIELDS_TAGEXPR(arg_node), arg_info);
    }  
  
    if(OUTPUTFIELDS_NEXT(arg_node) != NULL) {
      OUTPUTFIELDS_NEXT(arg_node) = TRAVdo(OUTPUTFIELDS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *PREPdoTypeCheck(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("PREPdoTypeCheck");
  
  DBUG_ASSERT((syntax_tree!= NULL), "PREPdoTypeCheck called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_preptcheck);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
