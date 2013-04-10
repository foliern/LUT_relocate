/*
 * $Id: free_info.h 1723 2007-11-12 15:09:56Z cg $
 */

/**
 * @file free_info.h
 *
 * Definition of INFO structure and macros for the free traversal.
 *
 * As the free traversal is split up into three files, the INFO
 * structure and macros have to be known to all three. The 
 * MakeInfo and FreeInfo functions still reside in free.c, as the
 * traversal is always started from within free.c
 * 
 */

/*
 * INFO structure
 */
struct INFO {
  node   *flag;
  node   *assign;
};

/*
 * INFO macros
 */
#define INFO_FREE_FLAG(n)                 (n->flag)
#define INFO_FREE_ASSIGN(n)               (n->assign)

