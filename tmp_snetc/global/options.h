/*
 * $Id: options.h 1723 2007-11-12 15:09:56Z cg $
 */

/******************************************************************************
 *
 * Options
 *
 * Prefix: OPT
 *
 * Description:
 *
 * This file provides external declarations for symbols defined in options.c.
 *
 *****************************************************************************/

#ifndef _SAC_OPTIONS_H_
#define _SAC_OPTIONS_H_

extern void OPTanalyseCommandline( int argc, char *argv[]);
extern void OPTcheckPreSetupOptions( int argc, char *argv[]);

#endif /* _SAC_OPTIONS_H_ */

