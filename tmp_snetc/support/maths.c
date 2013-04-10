/*******************************************************************************
 *
 * $Id: maths.c 1723 2007-11-12 15:09:56Z cg $
 *
 * Author: ?
 * -------
 *
 * Modified: Kari Keinanen 12.01.2007, VTT Technical Research Centre of Finland
 * ---------
 *           - functions renamed, ILIBstring -> MATH
 *
 *******************************************************************************/

#include "maths.h"
#include "dbug.h"

int MATHdigitCount(int number)
{
  int i = 1;

  DBUG_ENTER("MATHdigitCount");

  while (number / 10 >= 1) {
    number=number / 10;
    i+=1;
  }

  DBUG_RETURN( i);
}

int MATHlcm( int x, int y)
{
  int u, v;

  DBUG_ENTER( "MATHlcm");

  DBUG_ASSERT( ((x > 0) && (y > 0)), "arguments of lcm() must be >0");

  u = x;
  v = y;
  while (u != v) {
    if (u < v) {
      u += x;
    }
    else {
      v += y;
    }
  }

  DBUG_RETURN(u);
}
