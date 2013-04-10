/*
 * $Id: globals.h 1723 2007-11-12 15:09:56Z cg $
 */


/*
 * File : globals.h
 *
 * Description:
 *
 * We alow exactly one global variable named 'global', which is a huge
 * structure containing all globally available information.
 *
 * Most of the work is done in types.h where the complicated type
 * global_t is generated from globals.mac and in globals.c where the
 * initialization code again is generated from globals.mac.
 * 
 */


#ifndef _SAC_GLOBALS_H_
#define _SAC_GLOBALS_H_

#include "types.h"
#include <stdio.h>

extern FILE *yyin;
extern global_t global;

extern void GLOBinitializeGlobal( int argc, char *argv[]);

#endif /* _SAC_GLOBALS_H_ */
