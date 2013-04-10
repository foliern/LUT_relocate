/*******************************************************************************
 *
 * $Id: buffer.c 1723 2007-11-12 15:09:56Z cg $
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

#include "buffer.h"

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
#include "str.h"

struct PTR_BUF {
  void **buf;
  int pos;
  int size;
} ;


ptr_buf *BUFcreate(int size)
{
  ptr_buf *res;

  DBUG_ENTER("BUFcreate");

  res = (ptr_buf *)MEMmalloc( sizeof( ptr_buf));
  res->buf = (void **)MEMmalloc( size * sizeof( void *));
  res->pos = 0;
  res->size = size;

  DBUG_PRINT( "PTRBUF", ("allocating buffer size %d : %p", size, res));

  DBUG_RETURN( res);
}

ptr_buf *BUFadd(ptr_buf *s, void *ptr)
{
  int new_size;
  void **new_buf;
  int i;

  DBUG_ENTER( "BUFadd");

  if( s->pos == s->size) {
    new_size = 2*s->size;
    DBUG_PRINT( "PTRBUF", ("increasing buffer %p from size %d to size %d",
                           s, s->size, new_size));

    new_buf = (void **)MEMmalloc( new_size * sizeof( void *));
    for( i=0; i<s->pos; i++) {
      new_buf[i] = s->buf[i];
    }
    s->buf = MEMfree( s->buf);
    s->buf = new_buf;
    s->size = new_size;
  }
  s->buf[s->pos] = ptr;
  s->pos++;
  DBUG_PRINT( "PTRBUF", ("%p added to buffer %p", ptr, s));
  DBUG_PRINT( "PTRBUF", ("pos of buffer %p now is %d", s, s->pos));

  DBUG_RETURN( s);
}


int BUFsize(ptr_buf *s)
{
  DBUG_ENTER( "BUFsize");
  DBUG_RETURN( s->size);
}


void *BUFptr(ptr_buf *s, int pos)
{
  void *res;

  DBUG_ENTER( "BUFptr");
  if( pos < s->pos) {
    res = s->buf[ pos];
  } else {
    res = NULL;
  }
  DBUG_RETURN( res);
}


void BUFflush(ptr_buf *s)
{
  DBUG_ENTER( "BUFflush");

  s->pos = 0;
  DBUG_PRINT( "PTRBUF", ("pos of buffer %p reset to %d", s, s->pos));

  DBUG_VOID_RETURN;
}

void *BUFfree(ptr_buf *s)
{
  DBUG_ENTER( "BUFfree");

  DBUG_PRINT( "PTRBUF", ("freeing buffer %p", s));
  s->buf = MEMfree( s->buf);
  s = MEMfree( s);

  DBUG_RETURN( s);
}

struct STR_BUF {
  char *buf;
  int pos;
  int size;
} ;

str_buf *BUFcreateS(int size)
{
  str_buf *res;

  DBUG_ENTER("BUFcreateS");

  res = (str_buf *)MEMmalloc( sizeof( str_buf));
  res->buf = (char *)MEMmalloc( size * sizeof( char));
  res->buf[0] = '\0';
  res->pos = 0;
  res->size = size;

  DBUG_PRINT( "STRBUF", ("allocating buffer size %d : %p", size, res));

  DBUG_RETURN( res);
}

static
str_buf *EnsureStrBufSpace(str_buf *s, int len)
{
  int new_size;
  char *new_buf;

  DBUG_ENTER("EnsureStrBufSpace");

  if( (len + 1) > (s->size - s->pos) ) {

    new_size = ( len >= s->size ? s->size + 2*len : 2*s->size);

    DBUG_PRINT( "STRBUF", ("increasing buffer %p from size %d to size %d",
                           s, s->size, new_size));

    new_buf = (char *)MEMmalloc( new_size * sizeof( char));
    memcpy( new_buf, s->buf, s->pos+1);
    s->buf = MEMfree( s->buf);
    s->buf = new_buf;
    s->size = new_size;
  }

  DBUG_RETURN( s);

}

str_buf *BUFprintS(str_buf *s, const char *string)
{
  int len;

  DBUG_ENTER("BUFprintS");
 
  len = strlen( string);

  s = EnsureStrBufSpace( s, len);

  s->pos += sprintf( &s->buf[s->pos], "%s", string);
  DBUG_PRINT( "STRBUF", ("pos of buffer %p now is %d", s, s->pos));

  DBUG_RETURN( s);
}

str_buf *BUFprintfS(str_buf *s, const char *format, ...)
{
  va_list arg_p;
  int len, rem;
  bool ok;

  DBUG_ENTER("BUFprintfS");
  
  ok = FALSE;

  while (!ok) {
    rem = s->size - s->pos;

    va_start( arg_p, format);
    len = vsnprintf( &s->buf[s->pos], rem, format, arg_p);
    va_end( arg_p);

    if ((len >= 0) && (len < rem)) {
      ok = TRUE;
    } else {
      if (len < 0) {
	len = 2*(s->size+10);
      }
      s = EnsureStrBufSpace( s, len);
    }
  }
    
  s->pos += len;

  DBUG_RETURN( s);
}

char *BUF2str(str_buf *s)
{
  DBUG_ENTER( "BUF2str");

  DBUG_RETURN( STRcpy( s->buf));
}

void BUFflushS(str_buf *s)
{
  DBUG_ENTER( "BUFflushS");

  s->pos = 0;
  DBUG_PRINT( "STRBUF", ("pos of buffer %p reset to %d", s, s->pos));

  DBUG_VOID_RETURN;
}

bool BUFemptyS(str_buf *s)
{
  DBUG_ENTER( "BUFemptyS");

  DBUG_RETURN( s->pos == 0 );
}

void *BUFfreeS(str_buf *s)
{
  DBUG_ENTER( "BUFfreeS");

  s->buf = MEMfree( s->buf);
  s = MEMfree( s);

  DBUG_RETURN( s);
}
