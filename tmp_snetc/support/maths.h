/*******************************************************************************
 *
 * $Id: maths.h 1723 2007-11-12 15:09:56Z cg $
 *
 * Descipription: 
 * --------------
 *
 * Math functions of compiler tool-kit.
 *
 * Author: ?
 * -------
 *
 * Modified: Kari Keinanen 15.01.2007, VTT Technical Research Centre of Finland
 * ---------
 *           - functions renamed, ILIB -> MATH
 *           - documentation added
 *
 *******************************************************************************/

#ifndef _MATHS_H_
#define _MATHS_H_

#include "types.h"


/*******************************************************************************
 *
 * Description: Count number of digits of number.
 *
 * Parameters: - number, number which digits are count
 *
 * Return: - number of digits, e.g 4 for 1000
 *
 *******************************************************************************/
extern int MATHdigitCount(int number);


/*******************************************************************************
 *
 * Description: Get lowest common multiple of x, y.
 *
 * Parameters: - x
 *             - y
 *
 * Return: - lowest common multiple
 *
 *******************************************************************************/
extern int MATHlcm(int x, int y);


#endif /* _MATHS_H_ */
