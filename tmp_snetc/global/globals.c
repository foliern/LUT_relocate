/*
 *
 * $Id: globals.c 3371 2012-02-13 15:32:29Z mvn $
 *
 */

/*
 * File : globals.c
 *
 * This file should contain the definitions of all global variables
 * used in the implementation of the snetc compiler which are not restricted
 * to the use within one particular file or within the files of one
 * particular subdirectory. While the former ones have to be declared static,
 * the latter ones should be defined in a file called globals_xyz.c specific
 * to the respective subdirectory.
 *
 * However, the usage of global variables should be as extensive as possible
 * since a functional programming style is preferred in the SAC project. The
 * major application of global variables therefore is the storage of such
 * global information as determined by the command line arguments of a snetc
 * compiler call.
 *
 */

#include "globals.h"
#include "dbug.h"
#include "str.h"
#include "config.h"
#include "check_mem.h"

#include <limits.h>


/*
 * Initialize global variables from globals.mac
 */

global_t global;

void GLOBinitializeGlobal( int argc, char *argv[])
{
  DBUG_ENTER("GLOBinitializeGlobal");

#define GLOBALname( name) global.name = 
#define GLOBALinit( init) init ;
#include "globals.mac"

  global.argc = argc;
  global.argv = argv;

  DBUG_VOID_RETURN;
}
