/**
 * $Id: ctinfo.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * @file
 *
 * This file provides the interface for producing any kind of output during
 * compilation. It fully replaces the macro based legacy implementation
 * in files Error.[ch]
 *
 * We have 4 levels of verbosity controlled by the command line option -v
 * and the global variable verbose_level.
 *
 * Verbose level 0:
 *
 * Only error messages are printed.
 *
 * Verbose level 1:
 *
 * Error messages and warnings are printed.
 *
 * Verbose level 2:
 *
 * Error messages, warnings and basic compile time information, e.g. compiler
 * phases,  are printed.
 *
 * Verbose level 3:
 *
 * Error messages, warnings and full compile time information are printed.
 *
 *
 * Default values are 1 for the product version and 3 for the developer version.
 *
 */



#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "ctinfo.h"

#include "dbug.h"
#include "str.h"
#include "system.h"
#include "memory.h"
#include "build.h"
#include "print.h"
#include "globals.h"
#include "free.h"
#include "phase.h"
#include "check_mem.h"

#include "merrcode.h"

static char *message_buffer=NULL;
static int message_buffer_size=0;
static int message_line_length=76;

static char *abort_message_header = "ABORT: ";
static char *error_message_header = "ERROR: ";
static char *warn_message_header = "WARNING: ";
static char *state_message_header = "";
static char *note_message_header = "  ";


static int errors=0;
static int warnings=0;

/* Human readable error message formats*/
static const char *error_format_humanreadable_at_line = "%sat %s:%d\n\n";
static const char *error_format_humanreadable_at_location_usercode = "%sat %s:%d.%d (err_code=\"%s\")\n";
static const char *error_format_humanreadable_at_location = "%sat %s:%d.%d\n";

/* Normal error message formats*/
static const char *error_format_type_nomessage = "%s(%3d) - %s\n";
static const char *error_format_nomessage = "%s(%3d)\n";
static const char *error_format_type_message = "%s(%3d) - %s: ";
static const char *error_format_message = "%s(%3d) - ";
static const char *error_format_at_line_type_message = "%s(%3d) at %s:%d - %s: ";
static const char *error_format_at_line_message = "%s(%3d) at %s:%d - ";
static const char *error_format_at_location_usercode_type_message = "%s(%3d) at %s:%d.%d (err_code=\"%s\") - %s: ";
static const char *error_format_at_location_usercode_message = "%s(%3d) at %s:%d.%d (err_code=\"%s\") - ";
static const char *error_format_at_location_type_message = "%s(%3d) at %s:%d.%d - %s: ";
static const char *error_format_at_location_message = "%s(%3d) at %s:%d.%d - ";

/* Generic error type messages */
static const char *cti_errno_strings [] = {
  NULL,                    // CTI_ERRNO_OUT_OF_MEMORY_ERROR
  NULL,                    // CTI_ERRNO_INTERNAL_ERROR
  "File access error",     // CTI_ERRNO_FILE_ACCESS_ERROR
  NULL,                    // CTI_ERRNO_INVALID_OPTIONS
  NULL,                    // CTI_ERRNO_SYSTEM_CALL_ERROR
  "Parse error",           // CTI_ERRNO_PARSING_ERROR
  "Interface error",       // CTI_ERRNO_INTERFACE_ERROR
  NULL,                    // CTI_ERRNO_DEAD_CODE
  "Type check error",      // CTI_ERRNO_TYPE_CHECK_ERROR
  "Signature error",       // CTI_ERRNO_SIGNATURE_ERROR
  NULL,                    // CTI_ERRNO_TYPE_INFERENCE_ERROR
  NULL                     // CTI_ERRNO_PLUGIN_ERROR
};


#define MAX_ITEM_NAME_LENGTH 255
#define TEMP_FILETYPE  ".temp"


/******************************************************************************
 *
 * function:
 *   char *CVintBytes2String( unsigned int bytes)
 *
 * description:
 *   This function yields a pointer to a static memory area that contains
 *   a "dotted" version of the integer number given. It is primarily used
 *   for printing memory usages.
 *
 ******************************************************************************/

static
char *IntBytes2String( unsigned int bytes)
{
  static char res[32];
  char *tmp = &res[0];
  int factor = 1000000000;
  int num;

  DBUG_ENTER( "CVintBytes2String");

  while( (bytes/factor == 0) && (factor >= 1000) ) {
    factor /= 1000;
    tmp += sprintf( tmp, "    " );
  }
  tmp += sprintf( tmp, "%3u", (bytes/factor) );
  while( factor >= 1000) {
    bytes = bytes % factor;
    factor /= 1000;
    num = bytes/factor;
    if( num <10) {
      tmp += sprintf( tmp, ".00%1u", num);
    } else if( num<100) {
      tmp += sprintf( tmp, ".0%2u", num);
    } else {
      tmp += sprintf( tmp, ".%3u", num);
    }
  }

  DBUG_RETURN(res);
}

/** <!--********************************************************************-->
 *
 * @fn void ProcessMessage( char *buffer, int line_length)
 *
 *   @brief  formats message according to line length
 *
 *           '@' characters are inserted into the buffer to represent line
 *           breaks.
 *
 *   @param buffer  message buffer
 *   @param line_length maximum line length
 *
 ******************************************************************************/

static
void ProcessMessage( char *buffer, int line_length)
{
  int index, column, last_space;

  DBUG_ENTER("ProcessMessage");

  index=0;
  last_space=0;
  column=0;

  while (buffer[index]!='\0') {
    if (buffer[index]=='\t') {
      buffer[index]=' ';
    }

    if (buffer[index]==' ') {
      last_space=index;
    }

    if (buffer[index]=='\n') {
      buffer[index]='@';
      column=0;
    }
    else {
      if (column==line_length) {
        if (buffer[last_space]==' ') {
          buffer[last_space]='@';
          column=index-last_space;
        }
        else {
          break;
        }
      }
    }

    index++;
    column++;
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void Format2Buffer( const char *format, va_list arg_p)
 *
 *   @brief The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It is taken care of buffer overflows.
 *
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static
void Format2Buffer( const char *format, va_list arg_p)
{
  int len;
  va_list arg_p_copy;

  DBUG_ENTER("Format2Buffer");

  va_copy( arg_p_copy, arg_p);
  len = vsnprintf( message_buffer, message_buffer_size, format, arg_p_copy);
  va_end( arg_p_copy);

  if (len < 0) {
    DBUG_ASSERT((message_buffer_size == 0), "message buffer corruption");
    /*
     * Output error due to non-existing message buffer
     */

    len = 120;

    message_buffer = (char*) MEMmalloc( len+2);
    CHKMdoNotReport( message_buffer);
    message_buffer_size = len+2;

    va_copy( arg_p_copy, arg_p);
    len = vsnprintf( message_buffer, message_buffer_size, format, arg_p_copy);
    va_end( arg_p_copy);
    DBUG_ASSERT((len >= 0), "message buffer corruption");
  }

  if (len >= message_buffer_size) {
    /* buffer too small  */

    MEMfree( message_buffer);
    message_buffer = (char*) MEMmalloc( len+2);
    CHKMdoNotReport( message_buffer);
    message_buffer_size = len+2;

    va_copy( arg_p_copy, arg_p);
    len = vsnprintf( message_buffer, message_buffer_size, format, arg_p_copy);
    va_end( arg_p_copy);

    DBUG_ASSERT((len < message_buffer_size), "message buffer corruption");
  }

  DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn char *CTIgetErrorMessageVA( int line, const char *format, va_list arg_p)
 *
 *   @brief generates error message string
 *
 *          The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It is taken care of buffer overflows. Afterwards, the message
 *          is formatted to fit a certain line length and is printed to
 *          stderr.
 *
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

char *CTIgetErrorMessageVA( int line, const char *format, va_list arg_p)
{
  char *first_line, *res;

  DBUG_ENTER( "CTIgetErrorMessageVA");
  Format2Buffer( format, arg_p);
  ProcessMessage( message_buffer,
                  message_line_length - strlen( error_message_header));

  first_line = (char *)MEMmalloc( 32 * sizeof( char));
  sprintf( first_line, "line %d @", line);
  res = STRcat( first_line, message_buffer);
  first_line = MEMfree( first_line);

  DBUG_RETURN( res);
}



/** <!--********************************************************************-->
 *
 * @fn void PrintMessage( const char *header, const char *format, va_list arg_p)
 *
 *   @brief prints message
 *
 *          The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It is taken care of buffer overflows. Afterwards, the message
 *          is formatted to fit a certain line length and is printed to
 *          stderr.
 *
 *   @param header  string which precedes each line of the message, e.g.
                    ERROR or WARNING.
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static
void PrintMessage( const char *header, const char *format, va_list arg_p)
{
  char *line;

  DBUG_ENTER("PrintMessage");

  Format2Buffer( format, arg_p);

  ProcessMessage( message_buffer, message_line_length - strlen( header));

  line = strtok( message_buffer, "@");

  while (line != NULL) {
    fprintf( stderr, "%s%s\n", header, line);
    line = strtok( NULL, "@");
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn static void CleanUp()
 *
 *   @brief  does some clean up upon termination
 *
 *
 ******************************************************************************/

static
void CleanUp()
{
  DBUG_ENTER("CleanUp");
  // removes temp-file
  if(global.pathname != NULL) {
    if(strstr(global.pathname, ".") != NULL){
      if(strcmp(strstr(global.pathname, "."), TEMP_FILETYPE) == 0){
  if(remove(global.pathname) != 0){
    CTIwarn(CTI_ERRNO_FILE_ACCESS_ERROR,
	    "Could not remove temp file %s.", global.pathname);
  }
      }
    }
  }

  if(global.mdfv != NULL) {
    int i = 0;
    while(i < global.mdfc) {

      if(global.mdfv[i] != NULL) {
  MEMfree(global.mdfv[i]);
  global.mdfv[i] = NULL;
      }
      i++;
    }
    MEMfree(global.mdfv);
    global.mdfv = NULL;
    global.mdfc = 0;
  }

  if (global.cleanup) {
    global.cleanup = FALSE;
  }

  if(global.filebase != NULL){
    MEMfree(global.filebase);
    global.filebase = NULL;
  }
  if(global.pathname != NULL){
    MEMfree(global.pathname);
    global.pathname = NULL;
  }
  if(global.cpp_call != NULL){
    MEMfree(global.cpp_call);
    global.cpp_call = NULL;
  }
  if(global.include_dirs != NULL){
    MEMfree(global.include_dirs);
    global.include_dirs = NULL;
  }
  if(global.clib_dirs != NULL){
    MEMfree(global.clib_dirs);
    global.clib_dirs = NULL;
  }
  if(global.clibs != NULL){
    MEMfree(global.clibs);
    global.clibs = NULL;
  }
  if(global.my_dbug_str != NULL){
    MEMfree(global.my_dbug_str);
    global.my_dbug_str = NULL;
  }
  if(global.break_specifier != NULL
     && strlen(global.break_specifier) > 0){
    MEMfree(global.break_specifier);
    global.break_specifier = NULL;
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void AbortCompilation()
 *
 *   @brief  terminates the compilation process with a suitable error message.
 *
 ******************************************************************************/

static
void AbortCompilation()
{
  DBUG_ENTER("AbortCompilation");

  fprintf( stderr, "\n*** Compilation failed ***\n");
  fprintf( stderr, "*** Exit code %d (%s)\n",
           global.compiler_phase,
           PHphaseName( global.compiler_phase));
  fprintf( stderr, "*** %d Error(s), %d Warning(s)\n\n",
           errors, warnings);

  CleanUp();

  exit( (int) global.compiler_phase);

  DBUG_VOID_RETURN;
}



/** <!--********************************************************************-->
 *
 * @fn void InternalCompilerErrorBreak( int sig)
 *
 *   @brief  interrupt handler for segmentation faults and bus errors
 *
 *           An error message is produced and a bug report is created which
 *           may be sent via email to an appropriate address.
 *           Temporary files are deleted and the compilation process
 *           terminated.
 *
 *           DBUG_ENTER/RETURN are omitted on purpose to reduce risk of
 *           creating more errors during error handling.
 *
 *   @param sig  signal causing interrupt
 *
 ******************************************************************************/

static
void InternalCompilerErrorBreak( int sig)
{
  FILE *error_file;
  int i;

  fprintf( stderr,
           "\n\n"
           "OOOPS your program crashed the compiler 8-((\n"
           "Please send a bug report to bugs@snet-home.org.\n\n");


  error_file = fopen( "SNETbugreport", "w");

  if (error_file != NULL) {
    fprintf( error_file, "/*\n"
                         " * SNET - bug report\n"
                         " * ================\n"
                         " *\n"
                         " * automatically generated on ");
    fclose( error_file);
    SYScallNoErr( "date >> SNETbugreport");
    error_file = fopen( "SNETbugreport", "a");

    fprintf( error_file, " *\n");
    fprintf( error_file, " * using snetc %s rev %s for %s\n",
             global.version_id,
             build_rev,
             global.target_platform);
    fprintf( error_file, " * built %s.\n", build_date);
    fprintf( error_file, " * by user %s on host %s for %s.\n",
                         build_user, build_host, build_os);
    fprintf( error_file, " *\n");

    fprintf( error_file, " * The compiler was called by\n");
    fprintf( error_file, " *  %s", global.argv[0]);
    for (i=1; i<global.argc; i++) {
      fprintf( error_file, " %s", global.argv[i]);
    }
    fprintf( error_file, "\n");
    fprintf( error_file, " *\n");

    if( global.pathname != NULL) {
      fprintf( error_file, " * The contents of %s is:\n", global.pathname);
      fprintf( error_file, " */\n\n");
      fclose( error_file);
      SYScallNoErr( "cat %s >> SNETbugreport", global.pathname);
      error_file = fopen( "SNETbugreport", "a");
    }
    else{
      fprintf( error_file,
               " * Compiler crashed before S-NET file name could be "
               "determined!\n");
      fprintf( error_file, " */\n\n");
    }

    fclose( error_file);

    fprintf( stderr,
             "For your convenience, the compiler has pre-fabricated a bug report in\n"
             "the file \"SNETbugreport\" which was created in the current directory!\n"
             "Besides some infos concerning the compiler version and its\n"
             "usage it contains the specified source file.\n"
             "If you want to send that bug report to us you may simply use\n\n"
             "  mail snet-bugs@snet-home.org < SNETbugreport\n\n");
  }
  else {
    fprintf( stderr, "Sorry, snetc is unable to create a bug report file.\n");
  }

  CleanUp();

  exit(1);
}


/** <!--********************************************************************-->
 *
 * @fn void UserForcedBreak( int sig)
 *
 *   @brief  interrupt handler for user-forced breaks like CTRL-C
 *
 *           Temporary files are deleted and the compilation process
 *           terminated.
 *
 *           DBUG_ENTER/RETURN are omitted on purpose to reduce risk of
 *           creating more errors during error handling.
 *
 *   @param sig  signal causing interrupt
 *
 ******************************************************************************/

static
void UserForcedBreak( int sig)
{
  CleanUp();

  exit(0);
}


/** <!--********************************************************************-->
 *
 * @fn void CTIinstallInterruptHandlers()
 *
 *   @brief  installs interrupt handlers
 *
 ******************************************************************************/

void CTIinstallInterruptHandlers()
{
  DBUG_ENTER("CTIinstallInterruptHandlers");

  signal( SIGSEGV, InternalCompilerErrorBreak); /* Segmentation Fault */
  signal( SIGBUS, InternalCompilerErrorBreak);  /* Bus Error */
  signal( SIGINT, UserForcedBreak);     /* Interrupt (Control-C) */

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn static void PrintUnformattedEmpty(const char* message_header, int errno)
 *
 *   @brief         Prints unformatted error message with no error description
 *
 *   @param header  string which precedes each line of the message, e.g.
 *                  ERROR or WARNING.
 *   @param errno   Error code
 *
 ******************************************************************************/

static void PrintUnformattedEmpty(const char* message_header, cti_errno_t errno)
{
  if(cti_errno_strings[CTI_ERRNO_OUT_OF_MEMORY_ERROR] != NULL) {
    fprintf( stderr, error_format_type_nomessage, abort_message_header, CTI_ERRNO_OUT_OF_MEMORY_ERROR,
	     cti_errno_strings[CTI_ERRNO_OUT_OF_MEMORY_ERROR]);
  } else {
    fprintf( stderr, error_format_nomessage, abort_message_header, CTI_ERRNO_OUT_OF_MEMORY_ERROR);
  }
}


/** <!--********************************************************************-->
 *
 * @fn static void PrintUnformatted(const char* message_header, int errno, const char *format, va_list arg_p)
 *
 *   @brief         Prints unformatted error message with error description
 *
 *   @param header  string which precedes each line of the message, e.g.
 *                  ERROR or WARNING.
 *   @param errno   Error code
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static void PrintUnformatted(const char* message_header, cti_errno_t errno, const char *format, va_list arg_p)
{
  if(cti_errno_strings[errno] != NULL) {
    fprintf(stderr, error_format_type_message, message_header, errno, cti_errno_strings[errno]);
  } else {
    fprintf(stderr, error_format_message, message_header, errno);
  }
  vfprintf(stderr, format, arg_p);
  fprintf( stderr, "\n");
}


/** <!--********************************************************************-->
 *
 * @fn static void PrintUnformattedLine(const char* message_header, int errno, int line, const char *format, va_list arg_p)
 *
 *   @brief         Prints unformatted error message with line number
 *
 *   @param header  string which precedes each line of the message, e.g.
 *                  ERROR or WARNING.
 *   @param errno   Error code
 *   @param line    Line number
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static void PrintUnformattedLine(const char* message_header, cti_errno_t errno, int line, const char *format, va_list arg_p)
{
  if(cti_errno_strings[errno] != NULL) {
    fprintf(stderr, error_format_at_line_type_message,
	    message_header, errno, global.filename, line, cti_errno_strings[errno]);
  } else {
    fprintf(stderr, error_format_at_line_message,
	    message_header, errno, global.filename, line);
  }

  vfprintf(stderr, format, arg_p);
  fprintf( stderr, "\n");
}


/** <!--********************************************************************-->
 *
 * @fn static void PrintUnformattedLine(const char* message_header, int errno, int line, const char *format, va_list arg_p)
 *
 *   @brief         Prints unformatted error message with node information
 *
 *   @param header  string which precedes each line of the message, e.g.
 *                  ERROR or WARNING.
 *   @param errno   Error code
 *   @param n       Node
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static void PrintUnformattedNode(const char* message_header, cti_errno_t errno, node *n, const char *format, va_list arg_p)
{
  if(NODE_ERRCODE(n) != NULL) {
    if(cti_errno_strings[errno] != NULL) {
      fprintf( stderr, error_format_at_location_usercode_type_message,
	       message_header, errno, NODE_FILE(n), NODE_LINE(n), NODE_COL(n), NODE_ERRCODE(n),
	       cti_errno_strings[errno]);
    } else {
      fprintf( stderr, error_format_at_location_usercode_message,
	       message_header, errno, NODE_FILE(n), NODE_LINE(n), NODE_COL(n), NODE_ERRCODE(n));
    }
  } else {
    if(cti_errno_strings[errno] != NULL) {
      fprintf( stderr,  error_format_at_location_type_message,
	       message_header, errno, NODE_FILE(n), NODE_LINE(n), NODE_COL(n),
	       cti_errno_strings[errno]);
    } else {
      fprintf( stderr, error_format_at_location_message,
	       message_header, errno, NODE_FILE(n), NODE_LINE(n), NODE_COL(n));
    }
  }

  vfprintf(stderr, format, arg_p);
  fprintf( stderr, "\n");
}




/** <!--********************************************************************-->
 *
 * @fn void CTIerror( const char *format, ...)
 *
 *   @brief  produces an error message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIerror(cti_errno_t errno, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIerror");

  va_start( arg_p, format);

  if(global.formatted_errors) {
    PrintMessage( error_message_header, format, arg_p);
  } else {

    PrintUnformatted(error_message_header, errno, format, arg_p);
  }

  va_end(arg_p);

  errors++;

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CTIerrorLine( int line, const char *format, ...)
 *
 *   @brief  produces an error message preceded by file name and line number.
 *
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIerrorLine(cti_errno_t errno, int line, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIerrorLine");

  va_start( arg_p, format);

  if(global.formatted_errors) {
    fprintf( stderr, error_format_humanreadable_at_line,
	     error_message_header, global.filename, line);
    PrintMessage( error_message_header, format, arg_p);

  } else {

    PrintUnformattedLine(error_message_header, errno, line, format, arg_p);
  }

  va_end(arg_p);

  errors++;

  DBUG_VOID_RETURN;
}

void CTIerrorNode(cti_errno_t errno, node *n, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIerrorNode");

  va_start( arg_p, format);

  if(global.formatted_errors) {
    if(NODE_ERRCODE(n) != NULL) {
      fprintf( stderr, error_format_humanreadable_at_location_usercode,
	       error_message_header, NODE_FILE(n), NODE_LINE(n), NODE_COL(n), NODE_ERRCODE(n));
    } else {
      fprintf( stderr, error_format_humanreadable_at_location,
	       error_message_header, NODE_FILE(n), NODE_LINE(n), NODE_COL(n));
    }

    PrintMessage( error_message_header, format, arg_p);
  } else {

    PrintUnformattedNode(error_message_header, errno, n, format, arg_p);
  }

  va_end(arg_p);

  errors++;

  DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorLineVA( int line, const char *format, va_list arg_p)
 *
 *   @brief  produces an error message preceded by file name and line number.
 *
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIerrorLineVA( int line, const char *format, va_list arg_p)
{
  DBUG_ENTER("CTIerrorLineVA");

  fprintf( stderr, "\n");
  fprintf( stderr, "%sline %d  file: %s\n",
           error_message_header, line, global.filename);
  PrintMessage( error_message_header, format, arg_p);

  errors++;

  DBUG_VOID_RETURN;
}




/** <!--********************************************************************-->
 *
 * @fn void CTIerrorContinued( const char *format, ...)
 *
 *   @brief  continues an error message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIerrorContinued( const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIerrorContinued");

  va_start( arg_p, format);

  PrintMessage( error_message_header, format, arg_p);

  va_end(arg_p);

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn int CTIgetErrorMessageLineLength( )
 *
 *   @brief  yields useful line length for error messages
 *
 *   @return line length
 *
 ******************************************************************************/

int CTIgetErrorMessageLineLength( )
{
  DBUG_ENTER("CTIgetErrorMessageLineLength");

  DBUG_RETURN( message_line_length - strlen( error_message_header));
}



/** <!--********************************************************************-->
 *
 * @fn void CTIabort( const char *format, ...)
 *
 *   @brief   produces an error message without file name and line number
 *            and terminates the compilation process.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIabort(cti_errno_t errno, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIabort");

  va_start( arg_p, format);

  if(global.formatted_errors) {
    PrintMessage( abort_message_header, format, arg_p);

  } else {

    PrintUnformatted(abort_message_header, errno, format, arg_p);

  }

  va_end(arg_p);

  errors++;

  AbortCompilation();

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CTIabortLine( int line, const char *format, ...)
 *
 *   @brief   produces an error message preceded by file name and line number
 *            and terminates the compilation process.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIabortLine(cti_errno_t errno, int line, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIabortLine");

  va_start( arg_p, format);

  if(global.formatted_errors) {
    fprintf( stderr, error_format_humanreadable_at_line,
	     abort_message_header, global.filename, line);
    PrintMessage( abort_message_header, format, arg_p);
  } else {

    PrintUnformattedLine(abort_message_header, errno, line, format, arg_p);

  }

  va_end(arg_p);

  errors++;

  AbortCompilation();

  DBUG_VOID_RETURN;
}

void CTIabortNode(cti_errno_t errno, node *n, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIabortLine");

  va_start( arg_p, format);

  if(global.formatted_errors) {
    if(NODE_ERRCODE(n) != NULL) {
      fprintf( stderr, error_format_humanreadable_at_location_usercode,
	       abort_message_header, NODE_FILE(n), NODE_LINE(n), NODE_COL(n), NODE_ERRCODE(n));
    } else {
      fprintf( stderr, error_format_humanreadable_at_location,
	       abort_message_header, NODE_FILE(n), NODE_LINE(n), NODE_COL(n));
    }

    PrintMessage( abort_message_header, format, arg_p);
  } else {

    PrintUnformattedNode(abort_message_header, errno, n, format, arg_p);

  }
  va_end(arg_p);

  errors++;

  AbortCompilation();

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CTIabortOutOfMemory( unsigned int request)
 *
 *   @brief   produces a specific "out of memory" error message
 *            without file name and line number and terminates the
 *            compilation process.
 *
 *            This very special function is needed because the normal
 *            procedure of formatting a message may require further
 *            allocation of memory, which in this very case generates
 *            a vicious circle of error messages instead of terminating
 *            compilation properly.
 *
 *   @param request size of requested memory
 *
 ******************************************************************************/

void CTIabortOutOfMemory( unsigned int request)
{
  DBUG_ENTER("CTIabortOutOfMemory");

  if(global.formatted_errors) {
    fprintf( stderr,
	     "\n"
	     "%sOut of memory:\n"
	     "%s %u bytes requested\n",
	     abort_message_header,
	     abort_message_header, request);

#ifdef SHOW_MALLOC
    fprintf( stderr, "%s %u bytes already allocated\n",
	     abort_message_header, global.current_allocated_mem);
#endif
  } else {

    PrintUnformattedEmpty(abort_message_header, CTI_ERRNO_OUT_OF_MEMORY_ERROR);

  }

  errors++;

  AbortCompilation();

  DBUG_VOID_RETURN;
}




/** <!--********************************************************************-->
 *
 * @fn void CTIabortOnError()
 *
 *   @brief  terminates compilation process if errors have occurred.
 *
 ******************************************************************************/

void CTIabortOnError()
{
  DBUG_ENTER("CTIabortOnError");

  if (errors > 0) {
    AbortCompilation();
  }

  DBUG_VOID_RETURN;
}



/** <!--********************************************************************-->
 *
 * @fn void CTIwarnLine( int line, const char *format, ...)
 *
 *   @brief   produces a warning message preceded by file name and line number.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIwarnLine(cti_errno_t errno, int line, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIwarnLine");

  if (global.verbose_level >= 1) {
    va_start( arg_p, format);

    if(global.formatted_errors) {
      fprintf(stderr, error_format_humanreadable_at_line,
	      warn_message_header, global.filename, line);
      PrintMessage( warn_message_header, format, arg_p);
    } else {

      PrintUnformattedLine(warn_message_header, errno, line, format, arg_p);

    }

    va_end(arg_p);

    warnings++;
  }

  DBUG_VOID_RETURN;
}

void CTIwarnNode(cti_errno_t errno, node *n, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIwarnLine");

  if (global.verbose_level >= 1) {

    va_start( arg_p, format);

    if(global.formatted_errors) {
      if(NODE_ERRCODE(n) != NULL) {
	fprintf( stderr, error_format_humanreadable_at_location_usercode,
		 warn_message_header, NODE_FILE(n), NODE_LINE(n), NODE_COL(n), NODE_ERRCODE(n));
      } else {
	fprintf( stderr, error_format_humanreadable_at_location,
		 warn_message_header, NODE_FILE(n), NODE_LINE(n), NODE_COL(n));
      }

      PrintMessage( warn_message_header, format, arg_p);
    } else {

      PrintUnformattedNode(warn_message_header, errno, n, format, arg_p);

    }
    va_end(arg_p);

    warnings++;
  }

  DBUG_VOID_RETURN;
}




/** <!--********************************************************************-->
 *
 * @fn void CTIwarn( const char *format, ...)
 *
 *   @brief   produces a warning message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/



void CTIwarn(cti_errno_t errno, const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIwarn");

  if (global.verbose_level >= 1) {
    va_start( arg_p, format);

    if(global.formatted_errors) {
      PrintMessage( warn_message_header, format, arg_p);
    } else {

      PrintUnformatted(warn_message_header, errno, format, arg_p);

    }

    va_end(arg_p);

    warnings++;
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CTIwarnContinued( const char *format, ...)
 *
 *   @brief  continues a warning message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIwarnContinued( const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIwarnContinued");

  va_start( arg_p, format);

  PrintMessage( warn_message_header, format, arg_p);

  va_end(arg_p);

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn int CTIgetWarnMessageLineLength( )
 *
 *   @brief  yields useful line length for warning messages
 *
 *   @return line length
 *
 ******************************************************************************/

int CTIgetWarnMessageLineLength( )
{
  DBUG_ENTER("CTIgetWarnMessageLineLength");

  DBUG_RETURN( message_line_length - strlen( warn_message_header));
}



/** <!--********************************************************************-->
 *
 * @fn void CTIstate( const char *format, ...)
 *
 *   @brief  produces basic compile time information output (verbose level 2)
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTIstate( const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTIstate");

  if (global.verbose_level >= 2) {
    va_start( arg_p, format);

    PrintMessage( state_message_header, format, arg_p);

    va_end(arg_p);
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CTInote( const char *format, ...)
 *
 *   @brief  produces full compile time information output (verbose level 3)
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void CTInote( const char *format, ...)
{
  va_list arg_p;

  DBUG_ENTER("CTInote");

  if (global.verbose_level >= 3) {
    va_start( arg_p, format);

    PrintMessage( note_message_header, format, arg_p);

    va_end(arg_p);
  }

  DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIterminateCompilation()
 *
 *   @brief  terminates successful compilation process
 *
 ******************************************************************************/

void CTIterminateCompilation( compiler_phase_t phase,
                              char *break_specifier,
                              node *syntax_tree)
{

   DBUG_ENTER("CTIterminateCompilation");
  /*
   * Upon premature termination of compilation process show compiler resources
   * or syntax tree if available.
   */

  if (phase < PH_parse) {
    /* RSCshowResources(); */
  }
  else {
    if (phase <= PH_codegen) {
      syntax_tree = PRTdoPrint( syntax_tree);
    }
  }

  /*
   *  Finally, we do some clean up.
   */

  if (syntax_tree != NULL) {
    syntax_tree = FREEdoFreeTree( syntax_tree);
  }

  /*
   *  At last, we display a success message.
   */

  CTIstate( " ");
  CTIstate( "*** Compilation successful ***");

  if (phase < PH_final) {
    CTIstate( "*** BREAK after: %s", PHphaseName(phase));
    if (break_specifier != NULL) {
      CTIstate( "*** BREAK specifier: '%s`", break_specifier);
    }
  }

  CleanUp();

#ifdef SHOW_MALLOC
  CHKMdeinitialize();

  CTIstate( "*** Maximum allocated memory (bytes):   %s",
            IntBytes2String( global.max_allocated_mem));
  CTIstate( "*** Currently allocated memory (bytes): %s",
            IntBytes2String( global.current_allocated_mem - message_buffer_size ));
#endif

  CTIstate( "*** Exit code 0");
  CTIstate( "*** 0 error(s), %d warning(s)", warnings);
  CTIstate( " ");

  /*
   * Free message_buffer
   */

  if(message_buffer != NULL) {
    MEMfree(message_buffer);
    message_buffer = NULL;
  }

  exit( 0);


  DBUG_VOID_RETURN;
}


