/*
 *
 * $Id: phase_drivers.c 3371 2012-02-13 15:32:29Z mvn $
 *
 */


#include "types.h"
#include "phase.h"
#include "dbug.h"

#include "phase_drivers.h"



#define PHASEelement( it_element)                       \
  node * PHDdrive##it_element ( node *syntax_tree)      \
  {                                                     \
    DBUG_ENTER("PHDdrive" #it_element);

#define SUBPHASEelement( it_element)                                    \
  syntax_tree = PHrunCompilerSubPhase( SUBPH_##it_element, syntax_tree);

#define ENDPHASE( it_element)  \
  DBUG_RETURN( syntax_tree);   \
}

#include "phase.mac"

#undef SUBPHASEelement
#undef PHASEelement
#undef ENDPHASE


