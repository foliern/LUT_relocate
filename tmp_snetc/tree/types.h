#ifndef _SNETC_TYPES_H_
#define _SNETC_TYPES_H_

#include <stdio.h>

#include "types_nodetype.h"
#include "types_trav.h"
#include "type.h" /* Types common to compiler and runtime. */

typedef enum {
  stream = 0
} snet_runtime_t;

typedef enum {
  pthread = 0,
  lpel,
  lpel_hrc
} snet_threading_t;

/*
 * Label qualifiers
 */
typedef enum{LQUA_none, LQUA_disc, LQUA_pass}lblqual;

/*
 * Filter out field type
 */
typedef enum{FILT_empty, FILT_field, FILT_stag, FILT_btag, FILT_int}filttype;

/*
 * Metadata: type
 */
typedef enum{MD_all, MD_net, MD_box}mdtype;

/*
 * bool values
 */

typedef int bool;

#define FALSE 0
#define TRUE  1

/*
 * The NEW node structure of the SAC syntax tree
 * The type is abstract, as there is _no_ way to access a node other
 * than using tree_basic.h. Thus the structure is defined in
 * tree_basic.h. This as well solves dependency problems.
 */
typedef struct NODE node;

/*****************************************************************************
 * The info structure is used during traversal to store some stateful
 * information. It replaces the old N_info node. The structure is defined
 * as an abstract type here, so it can be definied by the different
 * traversals to suit the local needs. To do so, define a structure INFO
 * within the .c file of your traversal or create a specific .h/.c file
 * included by all .c files of your traversal. You as well have to create
 * a static MakeInfo/FreeInfo function.
 *****************************************************************************/

typedef struct INFO info;


/*
 * type of traversal functions
 */

typedef node *(*travfun_p) (node *, info *);


/*
 * type for lookup tables
 */

typedef struct LUT_T lut_t;


/*
 * type for list
 */

typedef struct LIST_T list_t;


/*
 * Types for compiler phase and subphase identifiers.
 */

typedef enum {
  #define PHASEelement(it_element) PH_##it_element,
  #include "phase.mac"
  #undef PHASEelement
  PH_dummy
} compiler_phase_t;

typedef enum {
  #define SUBPHASEelement(it_element) SUBPH_##it_element,
  #include "phase.mac"
  #undef SUBPHASEelement
  SUBPH_dummy
} compiler_subphase_t;


/*
 * Type for global variables
 */

typedef struct GLOBAL_T {
  #define GLOBALtype( it_type) it_type
  #define GLOBALname( it_name) it_name ;
  #include "globals.mac"
} global_t;


/*
 * Types for string and pointer buffers
 */

typedef struct STR_BUF str_buf;
typedef struct PTR_BUF ptr_buf;

/*
 * Types for unary, arithmetic, comparison, relational and logical operators
 */

typedef enum{
    OPER_NONE, // No operator, or unknown operator
    OPER_NOT, // !
    OPER_ABS, // abs
    OPER_MUL, // *
    OPER_DIV, // /
    OPER_REM, // %
    OPER_ADD, // +
    OPER_SUB, // -
    OPER_MIN, // min
    OPER_MAX, // max
    OPER_EQ,  // ==
    OPER_NEQ, // !=
    OPER_LT,  // <
    OPER_LTQ, // <=
    OPER_GT,  // >
    OPER_GTQ, // >=
    OPER_AND, // &&
    OPER_OR,  // ||
    OPER_COND // ?
 } operator_t;

#endif  /* _SNETC_TYPES_H_ */
