/*******************************************************************************
 *
 * $Id: mutil.c 2483 2009-07-30 06:49:20Z jju $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   20.05.2007
 * -----
 *
 * Preprocessing of metadata
 *
 *******************************************************************************/

#include <string.h>
#include "tree_basic.h"
#include "memory.h"
#include "str.h"

#include "mutil.h"


snet_meta_data_enc_t *MDUtilMetadataCreate(node *keylist)
{
  snet_meta_data_enc_t *md;

  md = MEMmalloc(sizeof(snet_meta_data_enc_t));  
  md->num_keys = 0;
 
  while(keylist != NULL) {

    md->keys[md->num_keys] = METADATAKEYLIST_KEY(keylist);
    md->values[md->num_keys] = METADATAKEYLIST_VALUE(keylist);

    md->num_keys++;

    keylist =  METADATAKEYLIST_NEXT(keylist);
  }

  return md;
}

void MDUtilMetadataDestroy(snet_meta_data_enc_t *md)
{
  MEMfree(md);
}

const char *MDUtilMetadataGetKey(node *keylist, const char *key) 
{
  while(keylist != NULL) {

    if(STReq(METADATAKEYLIST_KEY(keylist), key) == TRUE) {

      return METADATAKEYLIST_VALUE(keylist);
    }
    
    keylist =  METADATAKEYLIST_NEXT(keylist);
  }

 return NULL;
} 

node *MDUtilMetadataSetKey(node *keylist, const char *key, const char *value) 
{
  node *temp;

  if(keylist == NULL) {
    return TBmakeMetadatakeylist(STRcpy(key), STRcpy(value), NULL);
  }

  temp = keylist;

  if(STReq(METADATAKEYLIST_KEY(temp), key) == TRUE) {
    
    MEMfree(METADATAKEYLIST_VALUE(temp));

    METADATAKEYLIST_VALUE(temp) = STRcpy(value);

    return keylist;
  } 

  while(METADATAKEYLIST_NEXT(temp) != NULL) {
    temp = METADATAKEYLIST_NEXT(temp);

    if(STReq(METADATAKEYLIST_KEY(temp), key) == TRUE) {
    
      MEMfree(METADATAKEYLIST_VALUE(temp));
      
      METADATAKEYLIST_VALUE(temp) = STRcpy(value);

      return keylist;
    } 
  }

  METADATAKEYLIST_NEXT(temp) = TBmakeMetadatakeylist(STRcpy(key), STRcpy(value), NULL);

  return keylist;
} 
