/*
 * $Id: check_lib.c 2507 2009-08-05 12:02:11Z jju $
 */

#include "check_lib.h"

#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"
#include "checktst.h"
#include "globals.h"
#include "ctinfo.h"
#include "dbug.h"



/** <!--**********************************************************************-->
 *
 * @fn node *CHKinsertError( node *arg_node, char *string)
 *******************************************************************************/

node *CHKinsertError( node *arg_node, char *string)
{
  DBUG_ENTER( "CHKinsertError");

  if ( arg_node == NULL) {
    
    /*
     * CTIwarn internaly frees memory that was allocated before the 
     * memtab has been copied in CHKMdoAnalyzeMemtab. Therefore it must
     * not be used to print the error string when the memcheck mechanism
     * is active
     */
#ifdef SHOW_MALLOC
    if ( global.memcheck) {
      fprintf( stderr, "WARNING: %s\n", string);
    }
    else 
#endif /* SHOW_MALLOC */
    {
      CTIwarn( CTI_ERRNO_INTERNAL_ERROR, "%s", string);
    }

    arg_node = TBmakeError( STRcpy( string), 
                            arg_node);
  } 
  else {
    
    if ( !(STReq( string, ERROR_MESSAGE( arg_node)))) {
      
      ERROR_NEXT( arg_node) = CHKinsertError( ERROR_NEXT( arg_node), string);
    }
  } 
  
  DBUG_RETURN( arg_node);
}


/** <!--**********************************************************************-->
 *
 * @fn node *CHKexistSon( node *child, node *arg_node, char *string)
 *
 *******************************************************************************/
node *CHKexistSon( node *son, node *arg_node, char *string)
{
  DBUG_ENTER( "CHKexistSon");
  
  if ( son == NULL) {

    NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                            string);
  }

  DBUG_RETURN( son);
}


/** <!--**********************************************************************-->
 *
 * @fn node *CHKexistAttribute( node *attribute, node *arg_node, char *string)
 *
 *******************************************************************************/
node *CHKexistAttribute( void *attribute, node *arg_node, char *string)
{
  DBUG_ENTER( "CHKexistAttribute");

  if ( attribute == NULL) {
      
    NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                            string);
  }
  
  DBUG_RETURN( attribute);
}


/** <!--**********************************************************************-->
 *
 * @fn node *CHKnotExist( node *son_attribute, node *arg_node, char *string)
 *
 *******************************************************************************/

node *CHKnotExist( void *son_attribute, node *arg_node, char *string)
{
  DBUG_ENTER( "CHKnotExist");

  if ( son_attribute != NULL) {

    NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                            string);
  }

  DBUG_RETURN( son_attribute);
}


/** <!--********************************************************************-->
 *
 * @fn node *CHKnotExistAttribute( node *attribute, node *arg_node, char *string)
 *
 *****************************************************************************/
node *CHKnotExistAttribute( void *attribute, node *arg_node, char *string)
{
  DBUG_ENTER( "CHKexistAttribute");

  if ( attribute != NULL) {

    NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                            string);
  }

  DBUG_RETURN( attribute);
}


/** <!--**********************************************************************-->
 *
 * @fn node *CHKcorrectTypeInsertError( node *arg_node, char *string)
 *
 *******************************************************************************/
node *CHKcorrectTypeInsertError( node *arg_node, char *string)
{
  DBUG_ENTER( "CHKcorrectType");

  NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                          string);

  DBUG_RETURN( arg_node);
}

