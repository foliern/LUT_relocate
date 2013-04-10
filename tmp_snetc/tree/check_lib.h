/*
 * $Id: check_lib.h 1723 2007-11-12 15:09:56Z cg $
 */

#ifndef _SAC_CHECK_LIB_H_
#define _SAC_CHECK_LIB_H_

#include "types.h"

/******************************************************************************
 *
 * Check
 *
 * Prefix: CHK
 *
 *****************************************************************************/

#ifdef SHOW_MALLOC
extern node *CHKinsertError( node *arg_node, char *string);
extern node *CHKexistSon ( node *son, node *arg_node, char *string);
extern node *CHKexistAttribute( void *attribute, node *arg_node, char *string);
extern node *CHKnotExistAttribute( void *attribute, node *arg_node, char *string);
extern node *CHKnotExist( void *son_attribute, node *arg_node, char *string);
extern node *CHKcorrectTypeInsertError( node *arg_node, char *string);
#endif /* SHOW_MALLOC */ 

#endif /*_SAC_CHECK_LIB_H_ */


