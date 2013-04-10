/*
 *
 * $Log$
 * Revision 3.2  2004/11/22 15:42:55  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 3.1  2000/11/20 17:59:28  sacbase
 * new release made
 *
 * Revision 2.2  2000/03/23 14:00:28  jhs
 * Added build_os.
 *
 * Revision 2.1  1999/05/12 14:27:24  cg
 * initial revision
 *
 *
 */

/*****************************************************************************
 * 
 * file:   build.h
 *
 * prefix: 
 *
 * description:
 *
 *   This header file contains declarations of variables used for the -V
 *   command line option. The corresponding build.c file is created by 
 *   make upon each compilation attempt.
 *   
 *   
 *****************************************************************************/

#ifndef _SAC_BUILD_H_
#define _SAC_BUILD_H_

extern char build_date[];
extern char build_user[];
extern char build_host[];
extern char build_os[];
extern char build_rev[];
extern char build_ast[];

#endif  /* _SAC_BUILD_H_ */

