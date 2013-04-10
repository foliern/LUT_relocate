/*
 * $Id: memory.h 1723 2007-11-12 15:09:56Z cg $
 */

#ifndef _SAC_MEMORY_H_
#define _SAC_MEMORY_H_

#include <stdio.h>

/*********************************
 *
 * Memory
 *
 * Prefix: MEM 
 *
 *********************************/

extern void    *MEMfree  ( void *address);

/* special check_mem malloc-call to get more informations */

#ifdef SHOW_MALLOC        
extern void *MEMmallocAt( int size, char *file, int line);
#define MEMmalloc( size) MEMmallocAt( size, __FILE__, __LINE__)
#else
   extern void *MEMmalloc( int size);
#endif /* SHOW_MALLOC */

extern void *ILIBmemCopy( int size, void *mem);


/*********************************
 * macro definitions
 *********************************/

/* format string for pointers */
#ifdef NEED_PTR_PREFIX
 #define F_PTR "0x%p"
#else
 #define F_PTR "%p"
#endif



#endif /* _SAC_MEMORY_H_ */


