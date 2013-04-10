#ifndef _SNETC_MDUTIL_H
#define _SNETC_MDUTIL_H

/** Box related metadata **/

/* Key for language interface name of a box*/
#define METADATA_KEY_BOX_LANGUAGE_INTERFACE "interface"

/* Key for real name of a box (overrides the S-Net name) */
#define METADATA_KEY_BOX_REAL_NAME "boxfun"


/** Observer keys and values **/

#define METADATA_KEY_OBSERVER_DATA_LEVEL "observer_data"
#define METADATA_VALUE_OBSERVER_DATA_LEVEL_ALLVALUES "allvalues"
#define METADATA_VALUE_OBSERVER_DATA_LEVEL_TAGVALUES "tagvalues"
#define METADATA_VALUE_OBSERVER_DATA_LEVEL_LABELS "labels"

#define METADATA_KEY_OBSERVER "observer"

#define METADATA_KEY_OBSERVER_TYPE "observer_type"
#define METADATA_VALUE_OBSERVER_TYPE_BOTH "both"
#define METADATA_VALUE_OBSERVER_TYPE_BEFORE "before"
#define METADATA_VALUE_OBSERVER_TYPE_AFTER "after"

#define METADATA_KEY_OBSERVER_FILE "observer_file"

#define METADATA_KEY_OBSERVER_ADDRESS "observer_address"

#define METADATA_KEY_OBSERVER_CODE "observer_code"

#define METADATA_KEY_OBSERVER_INTERACTIVE "observer_interactive"

#define METADATA_KEY_OBSERVER_PORT "observer_port"
#define METADATA_VALUE_OBSERVER_DEFAULT_PORT "6555"

#define METADATA_KEY_ERR_CODE "err_code"

/** Compiler's internal metadata **/

#define METADATA_KEY_SNET_ORIGINAL_NAME "snet_originalname"



#include "types.h"
#include <metadata.h>

/* Encoding from the syntax tree (metadata of box or net).
 * NOTICE: Not a deep copy!
 */

snet_meta_data_enc_t *MDUtilMetadataCreate(node *keylist);
const char *MDUtilMetadataGet(snet_meta_data_enc_t *md, const char *key);


/* Destroy encoding. */

void MDUtilMetadataDestroy(snet_meta_data_enc_t *md);

/* Get value by key from syntax tree (metadata of box or net). */
const char *MDUtilMetadataGetKey(node *keylist, const char *key);

/* Set value for key from syntax tree (metadata of box or net). */
node *MDUtilMetadataSetKey(node *keylist, const char *key, const char *value);

#endif /* _SNETC_MDUTIL_H */

