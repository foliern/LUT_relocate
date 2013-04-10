#ifndef _SNETC_MAPPING_H
#define _SNETC_MAPPING_H

#include "types.h"


/* ************************ Top level function: ************************  */

extern node *NETIDdoLookup(node *syntax_tree);

/* ********************************************************************** */

extern node *NETIDstar( node *arg_node, info *arg_info );
extern node *NETIDsplit( node *arg_node, info *arg_info );

#endif /* _SNETC_MAPPING_H */

