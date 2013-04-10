/*
 * $Id: free_attribs.c 2483 2009-07-30 06:49:20Z jju $
 */

#include "dbug.h"
#include "free_attribs.h"
#include "typing.h"
#include "tree_basic.h"
#include "memory.h"
#include "free.h"


/** <!--******************************************************************-->
 *
 * @fn FREEattribString
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/

char *
FREEattribString (char *attr, node * parent)
{
  DBUG_ENTER ("FREEattribString");

  if (attr != NULL) {
    DBUG_PRINT ("FREE", ("Freeing string '%s' at " F_PTR, attr, attr));
    attr = MEMfree (attr);
  }

  DBUG_RETURN (attr);
}


/** <!--******************************************************************-->
 *
 * @fn FREEattribNode
 *
 * @brief Frees Node attribute
 *
 * @param attr Node node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/

node *
FREEattribNode (node * attr, node * parent)
{
  DBUG_ENTER ("FREEattribNode");

  if (attr != NULL) {
    DBUG_PRINT ("FREE", ("Starting to free %s node attribute at " F_PTR,
                         NODE_TEXT (attr), attr));
    attr = FREEdoFreeTree (attr);
  }

  DBUG_RETURN (attr);
}


/** <!--******************************************************************-->
 *
 * @fn FREEattribLink
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/

node *
FREEattribLink (node * attr, node * parent)
{
  DBUG_ENTER ("FREEattribLink");

  /*
   * NEVER do anything with this kind of attribute
   * as you cannot make sure the node you reference
   * here really exists!
   */

  DBUG_RETURN ((node *) NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribNTypeSigPtr
 *
 * @brief Frees struct TYP_NTYPESIG attribute
 *
 * @param attr struct TYP_NTYPESIG node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/

struct TYP_NTYPESIG * 
FREEattribNTypeSigPtr( struct TYP_NTYPESIG *attr, node *parent )
{
  if(attr != NULL){
    attr = TYPfreeNtypesig(attr);
  }

  return attr;
}
