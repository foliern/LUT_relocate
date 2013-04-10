/**
 * $Id: ctinfo.h 3378 2012-03-11 11:36:57Z vnn $
 *
 */

/**
 *
 * @file
 *
 * header file for ctinfo.c
 *
 */


#ifndef _SNET_CTINFO_H_
#define _SNET_CTINFO_H_

#include "types.h"
#include "tree_basic.h"
#include <stdarg.h>

typedef enum {
  CTI_ERRNO_OUT_OF_MEMORY_ERROR,
  CTI_ERRNO_INTERNAL_ERROR,
  CTI_ERRNO_FILE_ACCESS_ERROR,
  CTI_ERRNO_INVALID_OPTIONS,
  CTI_ERRNO_SYSTEM_CALL_ERROR,
  CTI_ERRNO_PARSING_ERROR,
  CTI_ERRNO_INTERFACE_ERROR,
  CTI_ERRNO_DEAD_CODE,
  CTI_ERRNO_TYPE_CHECK_ERROR,
  CTI_ERRNO_SIGNATURE_ERROR,
  CTI_ERRNO_TYPE_INFERENCE_ERROR,
  CTI_ERRNO_PLUGIN_ERROR,
  CTI_ERRNO_DISTRIB_WARN,
  CTI_ERRNO_TOPLEVEL_NET_INITIALIZER_ERROR,
  CTI_ERRNO_OPERATION_INITIALIZER_ERROR
} cti_errno_t;



extern void CTIinstallInterruptHandlers();
extern char *CTIgetErrorMessageVA( int line, const char *format, va_list arg_p);
extern void CTIerror(cti_errno_t, const char *format, ...);
extern void CTIerrorLine(cti_errno_t, int line, const char *format, ...);
extern void CTIerrorNode(cti_errno_t, node *n, const char *format, ...);
extern void CTIerrorContinued( const char *format, ...);
extern int CTIgetErrorMessageLineLength( );
extern void CTIabort(cti_errno_t, const char *format, ...);
extern void CTIabortLine(cti_errno_t, int line, const char *format, ...);
extern void CTIabortNode(cti_errno_t, node *n, const char *format, ...);
extern void CTIabortOutOfMemory( unsigned int request);
extern void CTIabortOnError();
extern void CTIwarn(cti_errno_t, const char *format, ...);
extern void CTIwarnLine(cti_errno_t, int line, const char *format, ...);
extern void CTIwarnNode(cti_errno_t, node *n, const char *format, ...);
extern void CTIwarnContinued( const char *format, ...);
extern int CTIgetWarnMessageLineLength( );
extern void CTIstate( const char *format, ...);
extern void CTInote( const char *format, ...);
extern void CTIterminateCompilation(compiler_phase_t phase,
                                    char *break_specifier,
                                    node *syntax_tree);
extern const char *CTIitemName(node *item);


#endif   /* _SNET_CTINFO_H_ */
