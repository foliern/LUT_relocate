/*
 * $Id: copy_attribs.c 2483 2009-07-30 06:49:20Z jju $
 */

#include "dbug.h"
#include "copy_attribs.h"
#include "typing.h"
#include "tree_basic.h"
#include "memory.h"
#include "copy.h"
#include "str.h"


/** <!--******************************************************************-->
 *
 * @fn COPYattribString
 *
 * @brief Copies String attribute
 *
 * @param attr String node to process
 * @param parent parent node
 *
 * @return result of Copy call
 *
 ***************************************************************************/

char *
COPYattribString (char *attr, node * parent)
{
  DBUG_ENTER ("COPYattribString");

  if (attr != NULL) {
    DBUG_PRINT ("COPY", ("Copying string '%s' at " F_PTR, attr, attr));
    attr = STRcpy(attr);
  }

  DBUG_RETURN (attr);
}


/** <!--******************************************************************-->
 *
 * @fn COPYattribNode
 *
 * @brief Copies Node attribute
 *
 * @param attr Node node to process
 * @param parent parent node
 *
 * @return result of Copy call
 *
 ***************************************************************************/

node *
COPYattribNode (node * attr, node * parent)
{
  DBUG_ENTER ("COPYattribNode");

  if (attr != NULL) {
    DBUG_PRINT ("COPY", ("Starting to Copy %s node attribute at " F_PTR,
                         NODE_TEXT (attr), attr));
    attr = COPYdoCopyTree (attr);
  }

  DBUG_RETURN (attr);
}


/** <!--******************************************************************-->
 *
 * @fn COPYattribLink
 *
 * @brief Copies Link attribute
 *
 * @param attr Link node to process
 * @param parent parent node
 *
 * @return result of Copy call
 *
 ***************************************************************************/

node *
COPYattribLink (node * attr, node * parent)
{
  DBUG_ENTER ("COPYattribLink");

  /*
   * NEVER do anything with this kind of attribute
   * as you cannot make sure the node you reference
   * here really exists!
   */

  DBUG_RETURN ((node *) NULL);
}

/** <!--******************************************************************-->
 *
 * @fn COPYattribNTypeSigPtr
 *
 * @brief Copies struct TYP_NTYPESIG attribute
 *
 * @param attr struct TYP_NTYPESIG node to process
 * @param parent parent node
 *
 * @return result of Copy call
 *
 ***************************************************************************/

struct TYP_NTYPESIG * 
COPYattribNTypeSigPtr( struct TYP_NTYPESIG *attr, node *parent )
{
  if(attr != NULL){
    attr = TYPcopyNtypesig(attr);
  }

  return attr;
}
