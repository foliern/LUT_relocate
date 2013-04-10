/*******************************************************************************
 *
 * $Id: cpreproc.c 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   29.03.2007
 * -----
 *
 *******************************************************************************/

#include <stdio.h>

#include "cpreproc.h"
#include "dbug.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "str.h"

/*
 * file handle for parsing
 */
FILE *yyin;

node *CPPdoPreprocRun(node *syntax_tree)
{
  DBUG_ENTER("CPPdoPreprocRun");

  if(global.pathname == NULL) {
    CTIabort( CTI_ERRNO_FILE_ACCESS_ERROR,
	      "SNet-source file path is missing");
  }
  else {
    /* -E, run only preprocessor
       -C, do not discard comments
       -P, do not include #line directives
       -x c, specify explicitly that file is C-source-file (instead of using file suffix)
     */
    char *cmd = STRcatn(6, "gcc ", global.include_dirs, " ", global.cpp_call, " -E -C -x c ", global.pathname);
    yyin = popen(cmd, "r");
  
    if(yyin == NULL) {
      CTIabort( CTI_ERRNO_FILE_ACCESS_ERROR,
		"Unable to open file \"%s\"", global.pathname);
    }
    MEMfree(cmd);
  }
  DBUG_RETURN(syntax_tree);
}
