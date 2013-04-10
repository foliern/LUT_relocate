/*******************************************************************************
 *
 * $Id: str.c 2047 2008-07-31 11:13:31Z jju $
 *
 * Author: ?
 * -------
 *
 * Modified: Kari Keinanen 12.01.2007, VTT Technical Research Centre of Finland
 * ---------
 *           - functions renamed, ILIBstring -> STR
 *
 *******************************************************************************/

#include "str.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "dbug.h"

#include "ctinfo.h"
#include "free.h"
#include "globals.h"
#include "traverse.h"
#include "memory.h"

char *STRcpy( const char *source)
{
  char *ret;
   
  DBUG_ENTER( "STRcpy");
   
  if (source != NULL) {
    ret = (char*) MEMmalloc( sizeof( char) * (strlen( source) + 1));
    strcpy( ret, source);
  }
  else {
    ret = NULL;
  }

  DBUG_RETURN(ret);
}

char *STRncpy( const char *source, int maxlen)
{
  char *ret;
  int max;
   
  DBUG_ENTER( "STRncpy");
   
  if (source != NULL) {
    max = strlen( source);
    if (max > maxlen) {
      max = maxlen;
    }

    ret = (char*) MEMmalloc( sizeof( char) * ( max + 1));
    strncpy( ret, source, max);

    /* make sure string ends with 0 */
    ret[max] = '\0';
  }
  else {
    ret = NULL;
  }

  DBUG_RETURN(ret);
}

char *STRcat( const char *first, const char* second)
{
  char *result;

  DBUG_ENTER( "STRcat");

  result = STRcatn(2, first, second);
  
  DBUG_RETURN( result);
}

char *STRcatn(int n, ...)
{
  int i;
  int length = 0;
  char *result;
  const char *ptr;
  va_list argList;

  DBUG_ENTER("STRcatn");

  va_start(argList, n);
  for(i = 0; i < n; ++i) {
    ptr = va_arg(argList, const char *);
    if(ptr != NULL) {
      length += strlen(ptr);
    }
  }
  va_end(argList);

  result = (char *) MEMmalloc(length + 1);
  result[0] = 0;

  va_start(argList, n);
  for(i = 0; i < n; ++i) {
    ptr = va_arg(argList, const char *);
    if(ptr != NULL) {
      if(result[0] == 0) {
        strcpy(result, ptr);
      }
      else {
        strcat(result, ptr);
      }
    }
  }
  va_end(argList);

  DBUG_RETURN(result);
}

bool STReq( const char *first, const char *second)
{
  bool res;

  DBUG_ENTER("STReq");

  if((first != NULL) && (second != NULL)) {
    res = (0 == strcmp( first, second));
  }
  else if((first == NULL) && (second == NULL)) {
    res = TRUE;
  }
  else {
    res = FALSE;
  }

  DBUG_RETURN( res);
}

bool STReqci( const char *first, const char *second)
{
  bool res;
  int i;
  
  DBUG_ENTER("STReqci");

  if ((first == NULL) && (second == NULL)) {
    res = TRUE;
  }
  else if ((first == NULL) || (second == NULL)) {
    res = FALSE;
  }
  else {
    i = 0;
    while ((first[i] != '\0') && (second[i] != '\0') 
           && (tolower(first[i]) == tolower(second[i]))) {
      i+=1;
    }
    if ((first[i] == '\0') && (second[i] == '\0')) {
      res = TRUE;
    }
    else {
      res = FALSE;
    }
  }
  
  DBUG_RETURN( res);
}
char *STRtok( char *first, char *sep)
{
  static char *act_string = NULL;
  char *new_string = NULL;
  char *ret;

  DBUG_ENTER( "STRtok");

  if (first != NULL) {
    if (act_string != NULL) {
      act_string = MEMfree( act_string);
    }
    new_string = STRcpy( first);
    act_string = new_string;
  }
   
  ret = strtok( new_string,sep);
   
  if (ret == NULL) {
    act_string = MEMfree( act_string);
  }
   
  DBUG_RETURN( ret);
}
 
char *STRitoa( int number)
{
  char *str;
  int tmp;
  int length;
  int base = 10;
  
  DBUG_ENTER("STRitoa");

  tmp = number;
  length = 1;
  while (tmp >= base) {
    tmp /= base;
    length++;
  }

  str = (char *) MEMmalloc( sizeof(char) * length + 3);

  sprintf( str, "%d", number);
  
  DBUG_RETURN( str);
}

char *STRitoa_oct( int number)
{
  char *str;
  int tmp;
  int length;
  int base = 8;
  
  DBUG_ENTER("STRitoa_oct");

  tmp = number;
  length = 1;
  while (tmp >= base) {
    tmp /= base;
    length++;
  }

  str = (char *) MEMmalloc( sizeof(char) * length + 3);

  sprintf( str, "0%o", number);
  
  DBUG_RETURN( str);
}

char *STRitoa_hex( int number)
{
  char *str;
  int tmp;
  int length;
  int base = 16;
  
  DBUG_ENTER("STRitoa_hex");

  tmp = number;
  length = 1;
  while (tmp >= base) {
    tmp /= base;
    length++;
  }

  str = (char *) MEMmalloc( sizeof(char) * length + 3);

  sprintf( str, "0x%x", number);
  
  DBUG_RETURN( str);
}

#define HEX2DIG( x)                                        \
      ( ( ( x >= '0') && (x <= '9') ) ? ( x - '0')         \
                                      : ( 10 + x - 'A') )

unsigned char *STRhexToBytes(unsigned char *array, const char *string)
{
  int pos;
  unsigned char low;
  unsigned char high;

  DBUG_ENTER("STRhexToBytes");

  pos = 0;

  while ( string[pos*2] != 0) {
    low = HEX2DIG( string[pos*2 + 1]);
    high = HEX2DIG(string[pos*2]);

    array[pos] = high * 16 + low;
    pos++;
  }

  DBUG_RETURN( array);
}

#define DIG2HEX( x)                                         \
      ( ( x < 10) ? ( '0' + x) : ( 'A' + x - 10) )

char *STRbytesToHex(int len, unsigned char *array)
{
  int pos;
  char *result;
  unsigned char low;
  unsigned char high;

  DBUG_ENTER("STRbytesToHex");

  result = MEMmalloc( ( 1 + len * 2) * sizeof( char));

  for (pos = 0; pos < len; pos ++) {
    low = array[pos] % 16;
    high = array[pos] / 16;

    result[2 * pos] = (char) DIG2HEX( high);
    result[2 * pos + 1] = (char) DIG2HEX( low);
  }

  result[2*len] = '\0';

  DBUG_RETURN( result);
}
