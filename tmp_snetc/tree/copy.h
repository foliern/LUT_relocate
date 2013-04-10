/******************************************************************************
 * $Id: copy.h 1723 2007-11-12 15:09:56Z cg $
 *
 * Author: Jukka Julku, VTT Technical Research Centre of Finland
 *
 * Date:   06.06.2007
 *
 * Description:
 *
 * Top-level copy function definition.
 *
 *****************************************************************************/

#ifndef _COPY_H_
#define _COPY_H_

#include "types.h"

/*
 * Top-level copy function.
 *
 * Returns copy of the argument tree or NULL if the copy 
 * procesess failed.
 *
 * NOTICE: NodePtr -attributes are NOT deep copied and so 
 *         all copied NodePtrs WILL point to original nodes.
 *
 */

extern node *COPYdoCopyTree( node *arg_node);


/*
 * traversal functions
 */

#include "copy_node.h"


#endif /* _COPY_H_ */
