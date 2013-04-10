/*
 * $Id: check_attribs.c 2483 2009-07-30 06:49:20Z jju $
 */

#include "check_attribs.h"

#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "check_mem.h"


/** <!--******************************************************************-->
 *
 * @fn CHKMattribString
 *
 * @brief Touch String attribute
 *
 * @param attr String node to process
 * @param arg_info arg_info structure
 *
 * @return the string
 *
 ***************************************************************************/
char *
CHKMattribString( char *attr, info * arg_info)
{
  DBUG_ENTER( "CHKMattribString");

  if( attr != NULL) {
    CHKMtouch( attr, arg_info);
  }

  DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribList
 *
 * @brief Touch List attribute
 *
 * @param attr List node to process
 * @param arg_info arg_info structure
 *
 * @return the string
 *
 ***************************************************************************/
list_t *
CHKMattribList( list_t *attr, info * arg_info)
{
  DBUG_ENTER( "CHKMattribList");

  if( attr != NULL) {
    CHKMtouch( attr, arg_info);
  }

  DBUG_RETURN (attr);
}


/** <!--******************************************************************-->
 *
 * @fn CHKMattribNode
 *
 * @brief Touch Node attribute
 *
 * @param attr Node node to process
 * @param arg_info arg_info structure
 *
 * @return the attribute
 *
 ***************************************************************************/
node *
CHKMattribNode( node * attr, info * arg_info)
{
  DBUG_ENTER( "CHKMattribNode");

  if( attr != NULL) {
    attr = TRAVdo( attr, arg_info);
  }

  DBUG_RETURN( attr);
}


/** <!--******************************************************************-->
 *
 * @fn CHKMattribLink
 *
 * @brief Touch Link attribute
 *
 * @param attr Link node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
node *
CHKMattribLink( node * attr, info * arg_info)
{
  DBUG_ENTER( "CHKMattribLink");

  /*
   * NEVER do anything with this kind of attribute
   * as you cannot make sure the node you reference
   * here really exists!
   */

  DBUG_RETURN( attr);
}


/** <!--******************************************************************-->
 *
 * @fn CHKMattribNodePointer
 *
 * @brief Touch NodePointer attribute
 *
 * @param attr NodePointer node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
node **
CHKMattribNodePointer( node ** attr, info * arg_info)
{
  DBUG_ENTER( "CHKMattribNodePointer");

  /* TODO: implement node pointer free function */

  DBUG_RETURN( attr);
}


/** <!--******************************************************************-->
 *
 * @fn CHKMattribNTypeSigPtr
 *
 * @brief Touch NTypeSigPtr attribute
 *
 * @param attr NTypeSigPtr node to process
 * @param arg_info arg_info structure
 *
 * @return the NTypeSigPtr
 *
 ***************************************************************************/

extern struct TYP_NTYPESIG * 
CHKMattribNTypeSigPtr( struct TYP_NTYPESIG *attr, info * arg_info)
{
  DBUG_ENTER( "CHKMattribNTypeSigPtr");

  if( attr != NULL) {
    CHKMtouch( attr, arg_info);
   
    /* TODO: how about the sets? */
  }

  DBUG_RETURN (attr);
}
