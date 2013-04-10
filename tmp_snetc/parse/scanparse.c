/*
 * $Id: scanparse.c 2507 2009-08-05 12:02:11Z jju $
 */

#include <stdio.h>

#include "scanparse.h"
#include "dbug.h"
#include "globals.h"
#include "ctinfo.h"

/*
 * external parser function from fun.y
 */
extern node *YYparseTree();
extern void yylex_destroy();

node *SPdoScanParse( node *syntax_tree)
{
  DBUG_ENTER("SPdoScanParse");

  if(yyin != NULL) {
    syntax_tree = YYparseTree();

    /* Close pipe of C-preprocessor call */
    pclose(yyin);
    yyin = NULL;
  }
  else {
    /* Parsing external network source */
    yyin = fopen(global.pathname, "r");

    if(yyin == NULL) {
      CTIabort(CTI_ERRNO_FILE_ACCESS_ERROR,
	       "Unable to open file '%s'", global.pathname);
    }
    syntax_tree = YYparseTree();
    fclose(yyin);
    yyin = NULL;
  }

  yylex_destroy();

  DBUG_RETURN( syntax_tree);
}
