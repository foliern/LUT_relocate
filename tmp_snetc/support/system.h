/*******************************************************************************
 *
 * $Id: system.h 3371 2012-02-13 15:32:29Z mvn $
 *
 * Descipription:
 * --------------
 *
 * System calls implementation of compiler tool-kit.
 *
 * Author: ?
 * -------
 *
 * Modified: Kari Keinanen 15.01.2007, VTT Technical Research Centre of Finland
 * ---------
 *           - functions renamed, ILIB -> SYS
 *           - documentation moved from c-file to header file
 *
 *******************************************************************************/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "types.h"


/******************************************************************************
 *
 * Function:
 *   void SYScall( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
 *
 ******************************************************************************/
extern void SYScall( char *format, ...);

void SYScallNoFormat( char *syscall);

/******************************************************************************
 *
 * Function:
 *   int SYScall2( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   In contrast to SYScall() no error message is printed upon failure but
 *   the exit code is returned.
 *
 ******************************************************************************/
extern int SYScallNoErr( char *format, ...);


/******************************************************************************
 *
 * Function:
 *   int SYStest( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
 *
 ******************************************************************************/
extern int SYStest( char *format, ...);


/******************************************************************************
 *
 * Function:
 *   void SYScreateCppCallString( const char *file, char *cccallstr
 *                                 const char *cppfile)
 *
 * Description:
 *   Checks whether the given filename is empty, i.e., we are reading from
 *   stdin, and generates a system call string in the buffer provided by
 *   cccallstr.
 *
 ******************************************************************************/
extern void SYScreateCppCallString( const char *file, char *cccallstr, 
                                    const char *cppfile);


#endif /* _SYSTEM_H_ */
