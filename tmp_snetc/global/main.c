/*
 *
 * $Id: main.c 3768 2013-04-05 11:57:27Z vtn.nguyen $
 *
 */


/*
 *  this file contains the main function of the S-NET compiler!
 */

#include <locale.h>

#include "phase.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "types.h"
#include "options.h"
#include "ctinfo.h"
#include "globals.h"

static void SetupLibraries()
{
  char *tmp;

  switch (global.runtime_lib) {
    case stream:
      tmp = global.clibs;
      global.clibs = STRcat(global.clibs, " -lruntimestream");
      MEMfree(tmp);
      break;
  }

  switch (global.distribution_lib) {
    case nodist:
      tmp = global.clibs;
      global.clibs = STRcat(global.clibs, " -ldistribnodist");
      MEMfree(tmp);
      break;

    case mpi:
      if (!global.enable_dist_snet) {
        CTIwarn(CTI_ERRNO_DISTRIB_WARN, "Compiling MPI version with "
                "distribution disabled!\n");
      }
      tmp = global.clibs;
      global.clibs = STRcat(global.clibs, " -ldistribmpi");
      MEMfree(tmp);
      break;

    case scc:
      if (!global.enable_dist_snet) {
        CTIwarn(CTI_ERRNO_DISTRIB_WARN, "Compiling SCC version with "
                "distribution disabled!\n");
        //Warning
      }
      tmp = global.clibs;
      global.clibs = STRcat(global.clibs, " -ldistribscc -lRCCE_smallflags_nongory_nopwrmgmt");
      MEMfree(tmp);
      break;
  }

  switch (global.threading_lib) {
    case pthread:
      tmp = global.clibs;
      global.clibs = STRcat(global.clibs, " -ltbpthread");
      MEMfree(tmp);
      break;

    case lpel:
      tmp = global.clibs;
      global.clibs = STRcat(global.clibs, " -ltblpel");
      MEMfree(tmp);
      break;
    case lpel_hrc:
      tmp = global.clibs;
      global.clibs = STRcat(global.clibs, " -ltblpel_hrc");
      MEMfree(tmp);
      break;
  }
}

static
node *SetupCompiler( int argc, char *argv[])
{
  node *syntax_tree;

  DBUG_ENTER("SetupCompiler");

  setlocale( LC_ALL, "C");
  CTIinstallInterruptHandlers();
  OPTcheckPreSetupOptions( argc, argv);

  GLOBinitializeGlobal( argc, argv);
  OPTanalyseCommandline( argc, argv);

  SetupLibraries();

  global.compiler_phase = PH_parse;
  syntax_tree = NULL;

  DBUG_RETURN( syntax_tree);
}



/*
 *  And now, the main function which triggers the whole compilation.
 */

int main( int argc, char *argv[])
{
  node *syntax_tree;

  DBUG_ENTER("main");

  /*
   * We must set up the compiler infrastructure first.
   */

  syntax_tree = SetupCompiler( argc, argv);

  /*
   * The sequence of compiler phases is derived from phase_info.mac.
   */

#define PHASEelement(it_element) \
  syntax_tree = PHrunCompilerPhase( PH_##it_element, syntax_tree);

#include "phase.mac"

#undef SUBPHASEelement


  /*
   * Now, we are done.
   */

  CTIterminateCompilation( PH_final, "", syntax_tree);

  DBUG_RETURN( 0);
}
