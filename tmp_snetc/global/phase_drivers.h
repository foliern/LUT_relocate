/*
 *
 * $Id: phase_drivers.h 1723 2007-11-12 15:09:56Z cg $
 *
 */



#ifndef _SAC_PHASE_DRIVERS_H_
#define _SAC_PHASE_DRIVERS_H_

#include "types.h"

/*
 * The prototypes of phase driver functions are derived from phase_info.mac.
 */

#define PHASEelement( it_element) \
extern node * PHDdrive##it_element ( node *syntax_tree);

#include "phase.mac"

#undef SUBPHASEelement


#endif  /* _SAC_PHASE_DRIVERS_H_ */
