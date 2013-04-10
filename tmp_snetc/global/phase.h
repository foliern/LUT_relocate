/*
 *
 * $Id: phase.h 1723 2007-11-12 15:09:56Z cg $
 *
 */


#ifndef _SAC_PHASE_H_
#define _SAC_PHASE_H_

#include "types.h"

extern node *PHdummy( node *syntax_tree);
extern node *PHidentity( node *syntax_tree);

extern const char *PHphaseName( compiler_phase_t phase);
extern node *PHrunCompilerPhase( compiler_phase_t phase, node *syntax_tree);

extern const char *PHsubPhaseName( compiler_subphase_t phase);
extern node *PHrunCompilerSubPhase( compiler_subphase_t phase, 
                                    node *syntax_tree);

#endif  /* _SAC_PHASE_H_ */
