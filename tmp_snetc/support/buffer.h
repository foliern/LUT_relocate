/*******************************************************************************
 *
 * $Id: buffer.h 1723 2007-11-12 15:09:56Z cg $
 *
 * Descipription:
 * --------------
 *
 * Buffer implementation of compiler tool-kit.
 *
 * Author: ?
 * -------
 *
 * Modified: Kari Keinanen 15.01.2007, VTT Technical Research Centre of Finland
 * ---------
 *           - functions renamed, ILIB -> BUF
 *           - documentation moved from c-file to header file
 *
 *******************************************************************************/

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "types.h"


/*****************************************************************************
 *
 * @fn  ptr_buf *BUFcreate(int size)
 *
 *   @brief  creates an (unbound) pointer buffer
 *
 *           Similar mechanism as used for StrBuf's.
 *           Here, instead of characters, void pointers are stored.
 *   @param  size
 *   @return the pointer to the freshly allocated buffer.
 *
 ******************************************************************************/
extern ptr_buf *BUFcreate(int size);


/*****************************************************************************
 *
 * @fn  ptr_buf *BUFadd(ptr_buf *s, void *ptr)
 *
 *   @brief  adds ptr to buffer s (new last element)
 *
 *   @param  s
 *   @param  ptr
 *   @return the modified buffer
 *
 ******************************************************************************/
extern ptr_buf *BUFadd(ptr_buf *s, void *ptr);


/******************************************************************************
 *
 * @fn  int BUFsize(ptr_buf *s)
 *
 *   @brief  retrieve size of given pointer buffer
 *
 *   @param  s
 *   @return size of the buffer
 *
 ******************************************************************************/
extern int BUFsize(ptr_buf *s);


/*****************************************************************************
 *
 * @fn  void *BUFptr(ptr_buf *s, int pos)
 *
 *   @brief  get pointer entry at specified position
 *
 *   @param  s
 *   @param  pos
 *   @return entry
 *
 ******************************************************************************/
extern void *BUFptr(ptr_buf *s, int pos);


/******************************************************************************
 *
 * @fn  void BUFflush(ptr_buf *s)
 *
 *   @brief  flushes the given pointer buffer (no deallocation!)
 *
 *   @param  s
 *
 ******************************************************************************/
extern void BUFflush(ptr_buf *s);


/******************************************************************************
 *
 * @fn  void *BUFfree(  ptr_buf *s)
 *
 *   @brief  deallocates the given pointer buffer!
 *
 *   @param  s
 *
 ******************************************************************************/
extern void *BUFfree(ptr_buf *s);


/******************************************************************************
 *
 * Function:
 *   str_buf *BUFcreateS(int size);
 *
 * Description:
 *
 *
 ******************************************************************************/
extern str_buf *BUFcreateS(int size);


/******************************************************************************
 *
 * Function:
 *   str_buf *BUFprintS(  str_buf *s, const char *string);
 *
 * Description:
 *
 *
 ******************************************************************************/
extern str_buf *BUFprintS(str_buf *s, const char *string);


/******************************************************************************
 *
 * Function:
 *   str_buf *BUFprintfS(  str_buf *s, const char *format, ...);
 *
 * Description:
 *
 *
 ******************************************************************************/
extern str_buf *BUFprintfS(str_buf *s, const char *format, ...);


/******************************************************************************
 *
 * Function:
 *   char *BUF2str(  str_buf *s);
 *
 * Description:
 *
 *
 ******************************************************************************/
extern char *BUF2str(str_buf *s);


/******************************************************************************
 *
 * Function:
 *   void BUFflushS(  str_buf *s)
 *
 * Description:
 *
 *
 ******************************************************************************/
extern void BUFflushS(str_buf *s);


/******************************************************************************
 *
 * Function:
 *   bool BUFemptyS( str_buf *s)
 *
 * Description:
 *
 *
 ******************************************************************************/
extern bool BUFemptyS(str_buf *s);


/******************************************************************************
 *
 * Function:
 *   void *BUFfreeS( str_buf *s);
 *
 * Description:
 *
 ******************************************************************************/
extern void *BUFfreeS(str_buf *s);

#endif /* _BUFFER_H_ */
