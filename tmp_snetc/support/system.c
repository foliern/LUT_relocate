/*******************************************************************************
 *
 * $Id: system.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: ?
 * -------
 *
 * Modified: Kari Keinanen 15.01.2007, VTT Technical Research Centre of Finland
 * ---------
 *           - functions renamed, ILIB -> SYS
 *           - documentation moved from c-file to header file
 *
 *******************************************************************************/

#include "system.h"

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

#define MAX_SYSCALL 1000

/**
 * global file handle for syscall tracking
 */
static FILE *syscalltrack = NULL;

void SYSstartTracking()
{
  char *name;
  
  DBUG_ENTER("SYSstartTracking");

  DBUG_ASSERT( (syscalltrack == NULL),
      "tracking has already been enabled!");

  name = STRcat( global.pathname, ".snetc");
  
  syscalltrack = fopen(name, "w");
  
  if (syscalltrack==NULL) {
    CTIabort( CTI_ERRNO_FILE_ACCESS_ERROR,
	      "Unable to open file \"%s\" for writing", name);
  }

  MEMfree( name);
  
  fprintf( syscalltrack, "#! /bin/sh\n\n");

  DBUG_VOID_RETURN;
}

void SYSstopTracking()
{
  DBUG_ENTER("SYSstopTracking");

  DBUG_ASSERT( (syscalltrack != NULL),
      "no tracking log open!");

  fclose( syscalltrack);

  syscalltrack = NULL;

  DBUG_VOID_RETURN;
}

static
void trackSystemCall( const char *call)
{
  DBUG_ENTER("trackSystemCall");

  if (syscalltrack != NULL) {
    fprintf( syscalltrack, "%s\n\n", call);
  }

  DBUG_VOID_RETURN;
}

void SYScall( char *format, ...)
{
  va_list arg_p;
  static char syscall[MAX_SYSCALL];
  int exit_code;

  DBUG_ENTER( "SYScall");

  va_start( arg_p, format);
  vsprintf( syscall, format, arg_p);
  va_end( arg_p);

  SYScallNoFormat(syscall);
  DBUG_VOID_RETURN;
}

void SYScallNoFormat( char *syscall)
{
  int exit_code;

  DBUG_ENTER( "SYScallNoFormat");

  /* if -d syscall flag is set print all syscalls !
   * This allows for easy C-code patches.
   */
  if (global.show_syscall) {
    CTInote( "System call:\n %s", syscall);
  }

  trackSystemCall( syscall);
  exit_code = system( syscall);

  if (exit_code == -1) {
    CTIabort( CTI_ERRNO_SYSTEM_CALL_ERROR,
	      "System failure while trying to execute shell command.\n"
              "(e.g. out of memory).");
  }
  else if (WEXITSTATUS( exit_code) > 0) {
    CTIabort( CTI_ERRNO_SYSTEM_CALL_ERROR,
	      "System failed to execute shell command\n%s\n"
              "with exit code %d",
              syscall, WEXITSTATUS( exit_code));
  }
  else if (WIFSIGNALED( exit_code)) {
    if (WTERMSIG(exit_code) == SIGINT) {
      CTIabort( CTI_ERRNO_SYSTEM_CALL_ERROR,
		"Child recieved SIGINT when executing shell command \n%s\n",
		syscall);
    }
    else if (WTERMSIG( exit_code) == SIGQUIT)
    {
      CTIabort( CTI_ERRNO_SYSTEM_CALL_ERROR,
		"Child recieved SIGQUIT when executing shell command \n%s\n",
		syscall);
    }
  }
  else if (exit_code != 0) {
    CTIabort( CTI_ERRNO_SYSTEM_CALL_ERROR,
	      "Unknown failure while executing shell command \n%s\n"
              "Return value was %d",
              syscall,
              exit_code);
  }

  DBUG_VOID_RETURN;
}

int SYScallNoErr( char *format, ...)
{
  va_list arg_p;
  static char syscall[MAX_SYSCALL];

  DBUG_ENTER( "SYScallNoErr");

  va_start( arg_p, format);
  vsprintf( syscall, format, arg_p);
  va_end( arg_p);

  /* if -dnocleanup flag is set print all syscalls !
   * This allows for easy C-code patches.
   */
  if (global.show_syscall) {
    CTInote( "System call:\n%s", syscall);
  }

  trackSystemCall( syscall);

  DBUG_RETURN( system( syscall));
}

int SYStest( char *format, ...)
{
  va_list arg_p;
  static char syscall[MAX_SYSCALL];
  int exit_code;

  DBUG_ENTER( "SYStest");

  strcpy( syscall, "test ");

  va_start( arg_p, format);
  vsprintf( syscall + 5*sizeof( char), format, arg_p);
  va_end( arg_p);

  exit_code = system(syscall);

  if (exit_code == 0) {
    exit_code = 1;
  }
  else {
    exit_code = 0;
  }

  DBUG_PRINT( "SYSCALL", ("test returns %d", exit_code));

  DBUG_RETURN( exit_code);
}
