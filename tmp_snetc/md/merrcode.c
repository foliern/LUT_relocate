/*******************************************************************************
 *
 * $Id: merrcode.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   06.08.2009
 * -----
 *
 * Propagates user-defined error codes to all nodes.
 *
 *******************************************************************************/

#include "merrcode.h"
#include "traverse.h"
#include "dbug.h"
#include "tree_basic.h"
#include "memory.h"
#include "str.h"
#include "mutil.h"

struct INFO {
  const char *err_code;
};

#define INFO_ERRCODE(n)  (n->err_code)

static void infoClear(info *inf)
{
  INFO_ERRCODE(inf)= NULL;
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


/** <!--******************************************************************-->
 *
 * @fn MERRCODEat
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node At node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEat( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEat");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(AT_LEFT( arg_node)!= NULL){
    AT_LEFT( arg_node)= TRAVdo( AT_LEFT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEbtaginit
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BTagInit node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEbtaginit( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEbtaginit");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(BTAGINIT_BTAG( arg_node)!= NULL){
    BTAGINIT_BTAG( arg_node)= TRAVdo( BTAGINIT_BTAG( arg_node), arg_info);
  }
  if(BTAGINIT_INIT( arg_node)!= NULL){
    BTAGINIT_INIT( arg_node)= TRAVdo( BTAGINIT_INIT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEbtagref
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BTagRef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEbtagref( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEbtagref");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEbtags
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BTags node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEbtags( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEbtags");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(BTAGS_NEXT( arg_node)!= NULL){
    BTAGS_NEXT( arg_node)= TRAVdo( BTAGS_NEXT( arg_node), arg_info);
  }


  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEboxbody
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BoxBody node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEboxbody( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEboxbody");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEboxdef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BoxDef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEboxdef( node *arg_node, info *arg_info ){
  const char *temp;

  DBUG_ENTER("MERRCODEboxdef");

  /* Get new error code */
  temp = INFO_ERRCODE(arg_info);
  INFO_ERRCODE(arg_info)= MDUtilMetadataGetKey(BOXDEF_METADATA( arg_node), METADATA_KEY_ERR_CODE);

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));
  /* Visit sons */
  if(BOXDEF_SIGN( arg_node)!= NULL){
    BOXDEF_SIGN( arg_node)= TRAVdo( BOXDEF_SIGN( arg_node), arg_info);
  }
  if(BOXDEF_BODY( arg_node)!= NULL){
    BOXDEF_BODY( arg_node)= TRAVdo( BOXDEF_BODY( arg_node), arg_info);
  }
  if(BOXDEF_METADATA( arg_node)!= NULL){
    BOXDEF_METADATA( arg_node)= TRAVdo( BOXDEF_METADATA( arg_node), arg_info);
  }
  /* Reset error code */ 
  INFO_ERRCODE(arg_info)= temp;

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEboxref
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BoxRef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEboxref( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEboxref");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(NODE_ERRCODE(BOXREF_BOX(arg_node)));

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEboxsign
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BoxSign node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEboxsign( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEboxsign");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(BOXSIGN_INTYPE( arg_node)!= NULL){
    BOXSIGN_INTYPE( arg_node)= TRAVdo( BOXSIGN_INTYPE( arg_node), arg_info);
  }
  if(BOXSIGN_OUTTYPES( arg_node)!= NULL){
    BOXSIGN_OUTTYPES( arg_node)= TRAVdo( BOXSIGN_OUTTYPES( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEboxtypes
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BoxTypes node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEboxtypes( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEboxtypes");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(BOXTYPES_ENTRIES( arg_node)!= NULL){
    BOXTYPES_ENTRIES( arg_node)= TRAVdo( BOXTYPES_ENTRIES( arg_node), arg_info);
  }
  if(BOXTYPES_NEXT( arg_node)!= NULL){
    BOXTYPES_NEXT( arg_node)= TRAVdo( BOXTYPES_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}
/** <!--******************************************************************-->
 *
 * @fn MERRCODEbranchlist
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node BranchList node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEbranchlist( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEbranchlist");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(BRANCHLIST_ATTRACTS( arg_node)!= NULL){
    BRANCHLIST_ATTRACTS( arg_node)= TRAVdo( BRANCHLIST_ATTRACTS( arg_node), arg_info);
  }
  if(BRANCHLIST_BRANCH( arg_node)!= NULL){
    BRANCHLIST_BRANCH( arg_node)= TRAVdo( BRANCHLIST_BRANCH( arg_node), arg_info);
  }
  if(BRANCHLIST_NEXT( arg_node)!= NULL){
    BRANCHLIST_NEXT( arg_node)= TRAVdo( BRANCHLIST_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}
/** <!--******************************************************************-->
 *
 * @fn MERRCODEchoice
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Choice node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEchoice( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEchoice");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(CHOICE_BRANCHLIST( arg_node)!= NULL){
    CHOICE_BRANCHLIST( arg_node)= TRAVdo( CHOICE_BRANCHLIST( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEdefs
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Defs node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEdefs( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEdefs");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(DEFS_DEF( arg_node)!= NULL){
    DEFS_DEF( arg_node)= TRAVdo( DEFS_DEF( arg_node), arg_info);
  }
  if(DEFS_NEXT( arg_node)!= NULL){
    DEFS_NEXT( arg_node)= TRAVdo( DEFS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}
/** <!--******************************************************************-->
 *
 * @fn MERRCODEerror
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Error node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEerror( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEerror");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(ERROR_NEXT( arg_node)!= NULL){
    ERROR_NEXT( arg_node)= TRAVdo( ERROR_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEfieldinit
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node FieldInit node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEfieldinit( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEfieldinit");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(FIELDINIT_FIELD( arg_node)!= NULL){
    FIELDINIT_FIELD( arg_node)= TRAVdo( FIELDINIT_FIELD( arg_node), arg_info);
  }
  if(FIELDINIT_INIT( arg_node)!= NULL){
    FIELDINIT_INIT( arg_node)= TRAVdo( FIELDINIT_INIT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}
/** <!--******************************************************************-->
 *
 * @fn MERRCODEfieldref
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node FieldRef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEfieldref( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEfieldref");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEfields
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Fields node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEfields( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEfields");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(FIELDS_NEXT( arg_node)!= NULL){
    FIELDS_NEXT( arg_node)= TRAVdo( FIELDS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEfilt
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Filt node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEfilt( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEfilt");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(FILT_PATTERN( arg_node)!= NULL){
    FILT_PATTERN( arg_node)= TRAVdo( FILT_PATTERN( arg_node), arg_info);
  }
  if(FILT_GUARDACTIONS( arg_node)!= NULL){
    FILT_GUARDACTIONS( arg_node)= TRAVdo( FILT_GUARDACTIONS( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEguardactions
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node GuardActions node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEguardactions( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEguardactions");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(GUARDACTIONS_TAGEXPR( arg_node)!= NULL){
    GUARDACTIONS_TAGEXPR( arg_node)= TRAVdo( GUARDACTIONS_TAGEXPR( arg_node), arg_info);
  }
  if(GUARDACTIONS_ACTION( arg_node)!= NULL){
    GUARDACTIONS_ACTION( arg_node)= TRAVdo( GUARDACTIONS_ACTION( arg_node), arg_info);
  }
  if(GUARDACTIONS_NEXT( arg_node)!= NULL){
    GUARDACTIONS_NEXT( arg_node)= TRAVdo( GUARDACTIONS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEguardpatterns
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node GuardPatterns node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEguardpatterns( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEguardpatterns");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(GUARDPATTERNS_ENTRIES( arg_node)!= NULL){
    GUARDPATTERNS_ENTRIES( arg_node)= TRAVdo( GUARDPATTERNS_ENTRIES( arg_node), arg_info);
  }
  if(GUARDPATTERNS_CONDITION( arg_node)!= NULL){
    GUARDPATTERNS_CONDITION( arg_node)= TRAVdo( GUARDPATTERNS_CONDITION( arg_node), arg_info);
  }
  if(GUARDPATTERNS_NEXT( arg_node)!= NULL){
    GUARDPATTERNS_NEXT( arg_node)= TRAVdo( GUARDPATTERNS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODElanguageinterfaces
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node LanguageInterfaces node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODElanguageinterfaces( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODElanguageinterfaces");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(LANGUAGEINTERFACES_NEXT( arg_node)!= NULL){
    LANGUAGEINTERFACES_NEXT( arg_node)= TRAVdo( LANGUAGEINTERFACES_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEmetadataboxdef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node MetadataBoxDef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEmetadataboxdef( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEmetadataboxdef");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(METADATABOXDEF_KEYS( arg_node)!= NULL){
    METADATABOXDEF_KEYS( arg_node)= TRAVdo( METADATABOXDEF_KEYS( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEmetadatadefaultdef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node MetadataDefaultDef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEmetadatadefaultdef( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEmetadatadefaultdef");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(METADATADEFAULTDEF_KEYS( arg_node)!= NULL){
    METADATADEFAULTDEF_KEYS( arg_node)= TRAVdo( METADATADEFAULTDEF_KEYS( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEmetadatadefs
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node MetadataDefs node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEmetadatadefs( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEmetadatadefs");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(METADATADEFS_DEF( arg_node)!= NULL){
    METADATADEFS_DEF( arg_node)= TRAVdo( METADATADEFS_DEF( arg_node), arg_info);
  }
  if(METADATADEFS_NEXT( arg_node)!= NULL){
    METADATADEFS_NEXT( arg_node)= TRAVdo( METADATADEFS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEmetadatakeylist
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node MetadataKeyList node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEmetadatakeylist( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEmetadatakeylist");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(METADATAKEYLIST_NEXT( arg_node)!= NULL){
    METADATAKEYLIST_NEXT( arg_node)= TRAVdo( METADATAKEYLIST_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEmetadatalist
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node MetadataList node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEmetadatalist( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEmetadatalist");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(METADATALIST_ENTRY( arg_node)!= NULL){
    METADATALIST_ENTRY( arg_node)= TRAVdo( METADATALIST_ENTRY( arg_node), arg_info);
  }
  if(METADATALIST_NEXT( arg_node)!= NULL){
    METADATALIST_NEXT( arg_node)= TRAVdo( METADATALIST_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEmetadatanetdef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node MetadataNetDef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEmetadatanetdef( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEmetadatanetdef");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(METADATANETDEF_KEYS( arg_node)!= NULL){
    METADATANETDEF_KEYS( arg_node)= TRAVdo( METADATANETDEF_KEYS( arg_node), arg_info);
  }
  if(METADATANETDEF_CHILDS( arg_node)!= NULL){
    METADATANETDEF_CHILDS( arg_node)= TRAVdo( METADATANETDEF_CHILDS( arg_node), arg_info);
  }
  if(METADATANETDEF_DEFINITIONS( arg_node)!= NULL){
    METADATANETDEF_DEFINITIONS( arg_node)= TRAVdo( METADATANETDEF_DEFINITIONS( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEmodule
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Module node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEmodule( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEmodule");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(MODULE_DEFS( arg_node)!= NULL){
    MODULE_DEFS( arg_node)= TRAVdo( MODULE_DEFS( arg_node), arg_info);
  }
  if(MODULE_FIELDS( arg_node)!= NULL){
    MODULE_FIELDS( arg_node)= TRAVdo( MODULE_FIELDS( arg_node), arg_info);
  }
  if(MODULE_STAGS( arg_node)!= NULL){
    MODULE_STAGS( arg_node)= TRAVdo( MODULE_STAGS( arg_node), arg_info);
  }
  if(MODULE_BTAGS( arg_node)!= NULL){
    MODULE_BTAGS( arg_node)= TRAVdo( MODULE_BTAGS( arg_node), arg_info);
  }
  if(MODULE_INTERFACES( arg_node)!= NULL){
    MODULE_INTERFACES( arg_node)= TRAVdo( MODULE_INTERFACES( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEnetbody
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node NetBody node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEnetbody( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEnetbody");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(NETBODY_DEFS( arg_node)!= NULL){
    NETBODY_DEFS( arg_node)= TRAVdo( NETBODY_DEFS( arg_node), arg_info);
  }
  if(NETBODY_CONNECT( arg_node)!= NULL){
    NETBODY_CONNECT( arg_node)= TRAVdo( NETBODY_CONNECT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEnetdef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node NetDef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEnetdef( node *arg_node, info *arg_info ){
  const char *temp;

  DBUG_ENTER("MERRCODEnetdef");

  /* Get new error code */
  temp = INFO_ERRCODE(arg_info);
  INFO_ERRCODE(arg_info)= MDUtilMetadataGetKey(NETDEF_METADATA( arg_node), METADATA_KEY_ERR_CODE);

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(NETDEF_SIGN( arg_node)!= NULL){
    NETDEF_SIGN( arg_node)= TRAVdo( NETDEF_SIGN( arg_node), arg_info);
  }
  if(NETDEF_BODY( arg_node)!= NULL){
    NETDEF_BODY( arg_node)= TRAVdo( NETDEF_BODY( arg_node), arg_info);
  }
  if(NETDEF_METADATA( arg_node)!= NULL){
    NETDEF_METADATA( arg_node)= TRAVdo( NETDEF_METADATA( arg_node), arg_info);
  }
  /* Reset error code */ 
  INFO_ERRCODE(arg_info)= temp;

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEnetrefs
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node NetRefs node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEnetrefs( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEnetrefs");
  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(NODE_ERRCODE(NETREFS_NET(arg_node)));

  /* Visit sons */
  if(NETREFS_NEXT( arg_node)!= NULL){
    NETREFS_NEXT( arg_node)= TRAVdo( NETREFS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEoutputfields
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node OutputFields node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEoutputfields( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEoutputfields");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(OUTPUTFIELDS_LEFTFIELD( arg_node)!= NULL){
    OUTPUTFIELDS_LEFTFIELD( arg_node)= TRAVdo( OUTPUTFIELDS_LEFTFIELD( arg_node), arg_info);
  }
  if(OUTPUTFIELDS_RIGHTFIELD( arg_node)!= NULL){
    OUTPUTFIELDS_RIGHTFIELD( arg_node)= TRAVdo( OUTPUTFIELDS_RIGHTFIELD( arg_node), arg_info);
  }
  if(OUTPUTFIELDS_STAG( arg_node)!= NULL){
    OUTPUTFIELDS_STAG( arg_node)= TRAVdo( OUTPUTFIELDS_STAG( arg_node), arg_info);
  }
  if(OUTPUTFIELDS_BTAG( arg_node)!= NULL){
    OUTPUTFIELDS_BTAG( arg_node)= TRAVdo( OUTPUTFIELDS_BTAG( arg_node), arg_info);
  }
  if(OUTPUTFIELDS_TAGEXPR( arg_node)!= NULL){
    OUTPUTFIELDS_TAGEXPR( arg_node)= TRAVdo( OUTPUTFIELDS_TAGEXPR( arg_node), arg_info);
  }
  if(OUTPUTFIELDS_NEXT( arg_node)!= NULL){
    OUTPUTFIELDS_NEXT( arg_node)= TRAVdo( OUTPUTFIELDS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODErange
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Range node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODErange( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODErange");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(RANGE_STAGSTART( arg_node)!= NULL){
    RANGE_STAGSTART( arg_node)= TRAVdo( RANGE_STAGSTART( arg_node), arg_info);
  }
  if(RANGE_BTAGSTART( arg_node)!= NULL){
    RANGE_BTAGSTART( arg_node)= TRAVdo( RANGE_BTAGSTART( arg_node), arg_info);
  }
  if(RANGE_STAGSTOP( arg_node)!= NULL){
    RANGE_STAGSTOP( arg_node)= TRAVdo( RANGE_STAGSTOP( arg_node), arg_info);
  }
  if(RANGE_BTAGSTOP( arg_node)!= NULL){
    RANGE_BTAGSTOP( arg_node)= TRAVdo( RANGE_BTAGSTOP( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODErecentries
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node RecEntries node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODErecentries( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODErecentries");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(RECENTRIES_FIELD( arg_node)!= NULL){
    RECENTRIES_FIELD( arg_node)= TRAVdo( RECENTRIES_FIELD( arg_node), arg_info);
  }
  if(RECENTRIES_STAG( arg_node)!= NULL){
    RECENTRIES_STAG( arg_node)= TRAVdo( RECENTRIES_STAG( arg_node), arg_info);
  }
  if(RECENTRIES_BTAG( arg_node)!= NULL){
    RECENTRIES_BTAG( arg_node)= TRAVdo( RECENTRIES_BTAG( arg_node), arg_info);
  }
  if(RECENTRIES_NEXT( arg_node)!= NULL){
    RECENTRIES_NEXT( arg_node)= TRAVdo( RECENTRIES_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODErecouts
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node RecOuts node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODErecouts( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODErecouts");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));
  /* Visit sons */

  if(RECOUTS_FIELDS( arg_node)!= NULL){
    RECOUTS_FIELDS( arg_node)= TRAVdo( RECOUTS_FIELDS( arg_node), arg_info);
  }
  if(RECOUTS_NEXT( arg_node)!= NULL){
    RECOUTS_NEXT( arg_node)= TRAVdo( RECOUTS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODErectype
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node RecType node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODErectype( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODErectype");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(RECTYPE_ENTRIES( arg_node)!= NULL){
    RECTYPE_ENTRIES( arg_node)= TRAVdo( RECTYPE_ENTRIES( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEstaginit
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node STagInit node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEstaginit( node *arg_node, info *arg_info ){

  DBUG_ENTER("MERRCODEstaginit");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(STAGINIT_STAG( arg_node)!= NULL){
    STAGINIT_STAG( arg_node)= TRAVdo( STAGINIT_STAG( arg_node), arg_info);
  }
  if(STAGINIT_INIT( arg_node)!= NULL){
    STAGINIT_INIT( arg_node)= TRAVdo( STAGINIT_INIT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEstagref
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node STagRef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEstagref( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEstagref");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEstags
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node STags node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEstags( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEstags");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(STAGS_NEXT( arg_node)!= NULL){
    STAGS_NEXT( arg_node)= TRAVdo( STAGS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEserial
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Serial node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEserial( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEserial");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(SERIAL_LEFT( arg_node)!= NULL){
    SERIAL_LEFT( arg_node)= TRAVdo( SERIAL_LEFT( arg_node), arg_info);
  }
  if(SERIAL_RIGHT( arg_node)!= NULL){
    SERIAL_RIGHT( arg_node)= TRAVdo( SERIAL_RIGHT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEsplit
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Split node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEsplit( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEsplit");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(SPLIT_LEFT( arg_node)!= NULL){
    SPLIT_LEFT( arg_node)= TRAVdo( SPLIT_LEFT( arg_node), arg_info);
  }
  if(SPLIT_RANGE( arg_node)!= NULL){
    SPLIT_RANGE( arg_node)= TRAVdo( SPLIT_RANGE( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}
/** <!--******************************************************************-->
 *
 * @fn MERRCODEstar
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Star node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEstar( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEstar");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(STAR_LEFT( arg_node)!= NULL){
    STAR_LEFT( arg_node)= TRAVdo( STAR_LEFT( arg_node), arg_info);
  }
  if(STAR_TERM( arg_node)!= NULL){
    STAR_TERM( arg_node)= TRAVdo( STAR_TERM( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn MERRCODEfeedback
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Feedback node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEfeedback( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEfeedback");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(FEEDBACK_LEFT( arg_node)!= NULL){
    FEEDBACK_LEFT( arg_node)= TRAVdo( FEEDBACK_LEFT( arg_node), arg_info);
  }
  if(FEEDBACK_BACK( arg_node)!= NULL){
    FEEDBACK_BACK( arg_node)= TRAVdo( FEEDBACK_BACK( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}



/** <!--******************************************************************-->
 *
 * @fn MERRCODEsync
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Sync node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEsync( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEsync");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(SYNC_MAINPATTERN( arg_node)!= NULL){
    SYNC_MAINPATTERN( arg_node)= TRAVdo( SYNC_MAINPATTERN( arg_node), arg_info);
  }
  if(SYNC_AUXPATTERNS( arg_node)!= NULL){
    SYNC_AUXPATTERNS( arg_node)= TRAVdo( SYNC_AUXPATTERNS( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtagexpr
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node TagExpr node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtagexpr( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtagexpr");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(TAGEXPR_STAG( arg_node)!= NULL){
    TAGEXPR_STAG( arg_node)= TRAVdo( TAGEXPR_STAG( arg_node), arg_info);
  }
  if(TAGEXPR_BTAG( arg_node)!= NULL){
    TAGEXPR_BTAG( arg_node)= TRAVdo( TAGEXPR_BTAG( arg_node), arg_info);
  }
  if(TAGEXPR_CONDITION( arg_node)!= NULL){
    TAGEXPR_CONDITION( arg_node)= TRAVdo( TAGEXPR_CONDITION( arg_node), arg_info);
  }
  if(TAGEXPR_LEFT( arg_node)!= NULL){
    TAGEXPR_LEFT( arg_node)= TRAVdo( TAGEXPR_LEFT( arg_node), arg_info);
  }
  if(TAGEXPR_RIGHT( arg_node)!= NULL){
    TAGEXPR_RIGHT( arg_node)= TRAVdo( TAGEXPR_RIGHT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtrans
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Trans node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtrans( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtrans");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(TRANS_LEFT( arg_node)!= NULL){
    TRANS_LEFT( arg_node)= TRAVdo( TRANS_LEFT( arg_node), arg_info);
  }
  if(TRANS_RIGHT( arg_node)!= NULL){
    TRANS_RIGHT( arg_node)= TRAVdo( TRANS_RIGHT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtypedef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node TypeDef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtypedef( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtypedef");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(TYPEDEF_TYPE( arg_node)!= NULL){
    TYPEDEF_TYPE( arg_node)= TRAVdo( TYPEDEF_TYPE( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtypemap
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node TypeMap node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtypemap( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtypemap");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(TYPEMAP_INTYPE( arg_node)!= NULL){
    TYPEMAP_INTYPE( arg_node)= TRAVdo( TYPEMAP_INTYPE( arg_node), arg_info);
  }
  if(TYPEMAP_OUTTYPE( arg_node)!= NULL){
    TYPEMAP_OUTTYPE( arg_node)= TRAVdo( TYPEMAP_OUTTYPE( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtyperef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node TypeRef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtyperef( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtyperef");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtypesigdef
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node TypeSigDef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtypesigdef( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtypesigdef");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(TYPESIGDEF_TYPESIGN( arg_node)!= NULL){
    TYPESIGDEF_TYPESIGN( arg_node)= TRAVdo( TYPESIGDEF_TYPESIGN( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtypesigref
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node TypeSigRef node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtypesigref( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtypesigref");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtypesigns
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node TypeSigns node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtypesigns( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtypesigns");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(TYPESIGNS_TYPESIG( arg_node)!= NULL){
    TYPESIGNS_TYPESIG( arg_node)= TRAVdo( TYPESIGNS_TYPESIG( arg_node), arg_info);
  }
  if(TYPESIGNS_NEXT( arg_node)!= NULL){
    TYPESIGNS_NEXT( arg_node)= TRAVdo( TYPESIGNS_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MERRCODEtypes
 *
 * @brief Sets error code for the node and its sons
 *
 * @param arg_node Types node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *MERRCODEtypes( node *arg_node, info *arg_info ){
  DBUG_ENTER("MERRCODEtypes");

  /* Set error code */
  NODE_ERRCODE(arg_node)= STRcpy(INFO_ERRCODE(arg_info));

  /* Visit sons */
  if(TYPES_TYPE( arg_node)!= NULL){
    TYPES_TYPE( arg_node)= TRAVdo( TYPES_TYPE( arg_node), arg_info);
  }
  if(TYPES_NEXT( arg_node)!= NULL){
    TYPES_NEXT( arg_node)= TRAVdo( TYPES_NEXT( arg_node), arg_info);
  }

  /* Return value */
  DBUG_RETURN(arg_node);
}

node *MERRCODEdoPropagate(node *syntax_tree)
{
  info *inf;
  DBUG_ENTER("MERRCODEdoPropagate");
  DBUG_ASSERT((syntax_tree != NULL), " MERRCODEdoPropagate with empty syntaxtree");
  TRAVpush(TR_merrcode);
  inf = infoMake();
  syntax_tree = TRAVdo(syntax_tree, inf);
  inf = infoFree(inf);
  TRAVpop();
  DBUG_RETURN(syntax_tree);
}
