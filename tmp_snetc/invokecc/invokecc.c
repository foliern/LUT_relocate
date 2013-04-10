/*******************************************************************************
 *
 * $Id: invokecc.c 3768 2013-04-05 11:57:27Z vtn.nguyen $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   05.02.2007
 * -----
 *
 *******************************************************************************/

#include "invokecc.h"
#include <string.h>
#include <stdlib.h>
#include "dbug.h"
#include "globals.h"
#include "ctinfo.h"
#include "system.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"

struct INFO {
  char *cmd;
};

#define INFO_CMD(n) (n)->cmd

static info *infoMake(void)
{
  info *i = NULL;

  DBUG_ENTER("infoMake");

  i = MEMmalloc(sizeof(info));

  INFO_CMD(i) = NULL;

  DBUG_RETURN(i);
}

static info *infoFree(info *arg_info)
{
  info *i = NULL;

  DBUG_ENTER("infoFree");

  if(INFO_CMD(arg_info) != NULL){
    MEMfree(INFO_CMD(arg_info));
  }

  i = MEMfree(arg_info);

  DBUG_RETURN(i);
}

static char *catFreeFirst(char *str1, char *str2)
{
  char *result = STRcatn(3, str1, " ", str2);
  MEMfree(str1);
  return result;
}

static char *catFreeBoth(char *str1, char *str2)
{
  char *result = STRcatn(3, str1, " ", str2);
  MEMfree(str1);
  MEMfree(str2);
  return result;
}

/* Two specification types for include dirs and one for definitions:

   Environment variable include dirs:
   CPATH=inc_dir1:inc_dir2

   User command line specified include dirs:
   global.include_dirs=-Iinc_dir1 -Iinc_dir2

   CPP definitions:
   global.cpp_call=-DDEF1 -DDEF2=def2_value
*/
static void makeCompile()
{
  char *cmd, *cflags = getenv("SNET_EXTRA_CFLAGS");

  DBUG_ENTER("makeCompile");

  cmd = STRcatn(6, global.compiler, " ", global.filebase, ".c -o ", global.filebase, ".lo ");

  if (global.cpp_call != NULL) {
    cmd = catFreeFirst(cmd, global.cpp_call);
  }

  cmd = catFreeFirst(cmd, global.include_dirs);

  if (global.is_deployed == TRUE) {
    cmd = catFreeBoth(cmd, STRcatn(3, " -DSNetMain__", global.filebase, "=main "));
  }

  if (cflags != NULL) {
      cmd = catFreeBoth(cmd, STRcatn(2, " ", cflags));
  }

  CTInote("Compiling source files: %s\n", cmd);
  SYScallNoFormat(cmd);
  MEMfree(cmd);
  DBUG_VOID_RETURN;
}

/* Two specification types for library dirs:

   Environment variable:
   LIBRARY_PATH=lib_dir1:lib_dir2

   User command line specified:
   global.clib_dirs=-Llib_dir1 -Llib_dir2
*/
static void makeLink(info *arg_info)
{
  char *cmd, *ldflags = getenv("SNET_EXTRA_LDFLAGS");

  DBUG_ENTER("makeLink");

  cmd = STRcatn(2, global.linker, " *.lo ");
 
  if (global.is_deployed) {
    if(global.output_file != NULL) {
      cmd = catFreeBoth(cmd, STRcat(" -o ", global.output_file));
    }
    else {
      cmd = catFreeFirst(cmd, " -o a.out ");
    }
  } else {
    cmd = catFreeBoth(cmd, STRcatn(3, " -o lib", global.filebase, ".la "));
  }

  if (ldflags != NULL) {
      cmd = catFreeBoth(cmd, STRcatn(2, " ", ldflags));
  }

  if (global.rpath != NULL) {
    cmd = catFreeFirst(cmd, global.rpath);
  }

  if (INFO_CMD(arg_info) != NULL) {
    cmd = catFreeFirst(cmd, INFO_CMD(arg_info));
  }

  cmd = catFreeFirst(cmd, global.clib_dirs);
  cmd = catFreeFirst(cmd, global.clibs);

  /* Dirty hack to get easy static linking to work on the SCC */
  if (global.is_static) {
    char *staticFlags = " -all-static -Wl,-lruntimestream -Wl,-l";
    switch (global.threading_lib) {
      case pthread:
        staticFlags = STRcat(staticFlags, "tbpthread");
        break;
      case lpel:
        staticFlags = STRcat(staticFlags, "tblpel");
        break;
      case lpel_hrc:
        staticFlags = STRcat(staticFlags, "tblpel_hrc");
        break;
      default:
        CTIabort(CTI_ERRNO_INVALID_OPTIONS, "Unknown distribution layer.");
    }

    cmd = catFreeBoth(cmd, staticFlags);
  }

  CTInote("Linking executable: %s\n", cmd);
  SYScallNoFormat(cmd);
  MEMfree(cmd);
  DBUG_VOID_RETURN;
}

/* Collect language interface lib names */

node *CClanguageinterfaces(node *arg_node, info *arg_info)
{
  char *temp = NULL;

  DBUG_ENTER("CClanguageinterfaces");

  if (LANGUAGEINTERFACES_NAME(arg_node) != NULL){

    if (INFO_CMD(arg_info) != NULL) {
      temp = STRcatn(3, INFO_CMD(arg_info), " -l",  LANGUAGEINTERFACES_NAME(arg_node));
      MEMfree(INFO_CMD(arg_info));
    } else {
      temp = STRcatn(2, " -l",  LANGUAGEINTERFACES_NAME(arg_node));
    }

    INFO_CMD(arg_info) = temp;
  }

  DBUG_RETURN(arg_node);
}

node *CCdoCompile(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CCdoCompile");

  printf("\n\n------------------------------\n\n");

  TRAVpush(TR_cc);

  inf = infoMake();

  syntax_tree = TRAVdo(syntax_tree, inf);

  makeCompile();

  makeLink(inf);

  printf("\n\n------------------------------\n\n");

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}
