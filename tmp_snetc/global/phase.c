/*
 * $Id: phase.c 3371 2012-02-13 15:32:29Z mvn $
 */


#include "types.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "str.h"
#include "check.h"
#include "check_mem.h"

#include "cpreproc.h"
#include "scanparse.h"
#include "dcr.h"
#include "tcheck.h"
#include "boxex.h"
#include "tres.h"
#include "siginfprim.h"
#include "routeinf.h"
#include "typechk.h"
#include "mod.h"
#include "flat.h"
#include "ptypcln.h"
#include "ptran.h"
#include "ploc.h"
#include "netren.h"
#include "netflat.h"
#include "codegen.h"
#include "cgenwrap.h"
#include "invokecc.h"

#include "mpreproc.h"
#include "mredist.h"
#include "munpack.h"
#include "mprogdef.h"
#include "mremdup.h"
#include "mcollect.h"
#include "merrcode.h"



#include "phase.h"
#include "phase_drivers.h"



typedef node *(*phase_fun_p) (node *);


static const char *phase_name[] = {
  #define PHASEtext(it_text) it_text,
  #include "phase.mac"
  ""
};

static const phase_fun_p phase_fun[] = {
  #define PHASEelement(it_element) PHDdrive##it_element,
  #include "phase.mac"
  PHdummy
};


static const char *subphase_name[] = {
  #define SUBPHASEtext(it_text) it_text,
  #include "phase.mac"
  ""
};

static const char *subphase_specifier[] = {
  #define SUBPHASEspec(it_spec) it_spec,
  #include "phase.mac"
  ""
};

static const phase_fun_p subphase_fun[] = {
  #define SUBPHASEfun(it_fun) it_fun,
  #include "phase.mac"
  PHdummy
};


const char *PHphaseName( compiler_phase_t phase)
{
  DBUG_ENTER("PHphaseName");

  DBUG_RETURN( phase_name[phase]);
}



const char *PHsubPhaseName( compiler_subphase_t subphase)
{
  DBUG_ENTER( "PHsubPhaseName");

  DBUG_RETURN( subphase_name[subphase]);
}


node *PHrunCompilerPhase( compiler_phase_t phase, node *syntax_tree)
{
  DBUG_ENTER("PHrunCompilerPhase");

  if ((global.compiler_phase <= phase) && (phase < PH_final)) {
    global.compiler_phase = phase;

#ifndef DBUG_OFF
    if ((global.my_dbug)
        && (! global.my_dbug_active)
        && (global.compiler_phase >= global.my_dbug_from)
        && (global.compiler_phase <= global.my_dbug_to)) {
      DBUG_PUSH(global.my_dbug_str);
      global.my_dbug_active = 1;
    }
#endif

    CTIstate(" ");
    CTIstate("** %d: %s ...", (int)phase, PHphaseName( phase));

    syntax_tree = phase_fun[phase]( syntax_tree);

    CTIabortOnError();

#ifdef SHOW_MALLOC
    if ( global.treecheck && (syntax_tree != NULL)) {
      syntax_tree = CHKdoTreeCheck( syntax_tree);
    }

    if ( global.memcheck && (syntax_tree != NULL)) {
      syntax_tree = CHKMdoMemCheck( syntax_tree);
    }
#endif

    if ((global.my_dbug)
        && (global.my_dbug_active)
        && (global.compiler_phase >= global.my_dbug_to)) {
      DBUG_POP();
      global.my_dbug_active = 0;
    }

    if (global.break_after <= phase) {
      CTIterminateCompilation( global.break_after, NULL, syntax_tree);
    }
  }

  DBUG_RETURN( syntax_tree);
}



node *PHrunCompilerSubPhase( compiler_subphase_t subphase, node *syntax_tree)
{
  DBUG_ENTER("PHrunCompilerSubPhase");

  if (global.compiler_subphase <= subphase) {
    global.compiler_subphase = subphase;

    CTIstate("**** %s ...", PHsubPhaseName( subphase));

    syntax_tree = subphase_fun[subphase]( syntax_tree);

    CTIabortOnError();

#ifdef SHOW_MALLOC

    if ( (global.treecheck) && (syntax_tree != NULL)) {
      syntax_tree = CHKdoTreeCheck( syntax_tree);
    }

    if ( global.memcheck && (syntax_tree != NULL)) {
      syntax_tree = CHKMdoMemCheck( syntax_tree);
    }
#endif

    if ( global.break_after < global.compiler_phase ||
        (global.break_after == global.compiler_phase &&
         STReq( global.break_specifier, subphase_specifier[subphase] )
        ) ) {
      CTIterminateCompilation( global.break_after, global.break_specifier,
                               syntax_tree);
    }
  }

  DBUG_RETURN( syntax_tree);
}


node *PHdummy( node *syntax_tree)
{
  DBUG_ENTER("PHdummy");

  DBUG_ASSERT( FALSE, "This function should never be called.");

  DBUG_RETURN( syntax_tree);
}


node *PHidentity( node *syntax_tree)
{
  DBUG_ENTER("PHidentity");

  DBUG_RETURN( syntax_tree);
}

