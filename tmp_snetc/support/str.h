/*******************************************************************************
 *
 * $Id: str.h 1846 2008-04-22 14:47:40Z cg $
 *
 * Descipription: 
 * --------------
 *
 * String implementation of compiler tool-kit.
 *
 * Author: ?
 * -------
 *
 * Modified: Kari Keinanen 12.01.2007, VTT Technical Research Centre of Finland
 * ---------
 *           - functions renamed, ILIBstring -> STR
 *           - documentation added
 *           - new function STRcatn added (can be used as old ILIBstringConcat3/4)
 *
 *******************************************************************************/

#ifndef _STR_H_
#define _STR_H_

#include "types.h"


/*******************************************************************************
 *
 * Description: Copy string and allocate memory for new string.
 *
 * Parameters: - source, string to copy
 *
 * Return: - new copied string
 *
 *******************************************************************************/
extern char *STRcpy(const char *source);


/*******************************************************************************
 *
 * Description: Copy string and allocate memory for new string. 
 *              Copy only maxlen characters.
 *
 * Parameters: - source, string to copy
 *             - maxlen, number of characters to copy
 *
 * Return: - new copied string
 *
 *******************************************************************************/
extern char *STRncpy(const char *source, int maxlen);


/*******************************************************************************
 *
 * Description: Concatenate two strings and allocate memory for new string.
 *
 * Parameters: - first, first string
 *             - second, second string
 *
 * Return: - new concatenated string
 *
 *******************************************************************************/
extern char *STRcat(const char *first, const char* second);


/*******************************************************************************
 *
 * Description: Concatenate N strings and allocate memory for new string.
 *
 * Parameters: - n, number of strings
 *             - ..., n amount of "const char *"-type strings
 *
 * Return: - new concatenated string
 *
 *******************************************************************************/
extern char *STRcatn(int n, ...);


/*******************************************************************************
 *
 * Description: Tokenize string. On first call the str will be copied to internal
 *              static variable, next calls str should be NULL. With last call the 
 *              allocated memory of the copy will be freed.
 *
 * Parameters: - str, string to tokenize
 *             - tok, tokenizer
 *
 * Return: - pointer to the next token
 *         - NULL, no more tokens
 *
 *******************************************************************************/
extern char *STRtok(char *str, char *tok);


/*******************************************************************************
 *
 * Description: Compare two strings.
 *
 * Parameters: - first, first string to compare
 *             - second, second string to compare
 *
 * Return: - TRUE, string contents are equal
 *         - FALSE, string contents are not equal
 *
 *******************************************************************************/
extern bool STReq(const char *first, const char* second);


/*******************************************************************************
 *
 * Description: Compare two strings in a case insensitive way.
 *
 * Parameters: - first, first string to compare
 *             - second, second string to compare
 *
 * Return: - TRUE, string contents are equal
 *         - FALSE, string contents are not equal
 *
 *******************************************************************************/
extern bool STReqci(const char *first, const char* second);


/*******************************************************************************
 *
 * Description: Convert integer to string in decimal representation.
 *
 * Parameters: - number to convert
 *
 * Return: - new allocated string representation of number
 *
 *******************************************************************************/
extern char *STRitoa(int number);


/*******************************************************************************
 *
 * Description: Convert integer to string in octal representation.
 *
 * Parameters: - number to convert
 *
 * Return: - new allocated string representation of number
 *
 *******************************************************************************/
extern char *STRitoa_oct( int number);


/*******************************************************************************
 *
 * Description: Convert integer to string in hexadecimal representation.
 *
 * Parameters: - number to convert
 *
 * Return: - new allocated string representation of number
 *
 *******************************************************************************/
extern char *STRitoa_hex( int number);


/*******************************************************************************
 *
 * Description: Convert hex-string to byte array.
 *
 * Parameters: - array, converted byte array 
 *               memory must be allocated before calling this function
 *             - string, string to convert
 *
 * Return: - converted byte array
 *
 *******************************************************************************/
extern unsigned char *STRhexToBytes(unsigned char *array, const char *string);


/*******************************************************************************
 *
 * Description: Convert byte array to hex-string.
 *
 * Parameters: - len, length of byte array 
 *             - array, array to convert
 *
 * Return: - new alloceted string representation of byte array
 *
 *******************************************************************************/
extern char *STRbytesToHex(int len, unsigned char *array);

#endif /* _STR_H_ */
