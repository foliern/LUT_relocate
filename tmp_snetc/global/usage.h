/*
 *
 * $Log$
 * Revision 3.4  2004/11/22 21:19:32  ktr
 * prefix USAGE --> USG
 *
 * Revision 3.3  2004/11/22 15:42:55  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 3.2  2003/07/28 15:35:06  cg
 * Added short version identification option (-V).
 * Full version information is now available with -VV
 * (verbose version).
 *
 * Revision 3.1  2000/11/20 17:59:41  sacbase
 * new release made
 *
 * Revision 2.2  1999/05/12 14:28:54  cg
 * command line options streamlined.
 *
 * Revision 2.1  1999/02/23 12:40:17  sacbase
 * new release made
 *
 * Revision 1.4  1999/02/15 13:34:09  sbs
 * added -noDLAW opt_dlaw;
 *
 * Revision 1.3  1998/07/10 15:20:04  cg
 * included option -i to display copyright/disclaimer
 *
 * Revision 1.2  1994/11/10 15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _SAC_USAGE_H_
#define _SAC_USAGE_H_

/******************************************************************************
 *
 * Usage
 *
 * Prefix: USG
 *
 *****************************************************************************/
extern void USGprintUsage();
extern void USGprintCopyright();
extern void USGprintVersion();
extern void USGprintVersionVerbose();

#endif /* _SAC_USAGE_H_ */

