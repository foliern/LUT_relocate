/*
 * $Id: tree_basic.h 2501 2009-08-03 10:10:09Z jju $
 */

#ifndef _SAC_TREE_BASIC_H_
#define _SAC_TREE_BASIC_H_

#include "types.h"


/*
 *  Global Access-Macros
 */

#define NODE_TYPE(n) ((n)->nodetype)
#define NODE_FILE(n) ((n)->file)
#define NODE_LINE(n) ((n)->line)
#define NODE_COL(n)  ((n)->col)
#define NODE_ERROR(n)((n)->error)

#define NODE_TEXT(n) TBnodeText(NODE_TYPE(n))

#define NODE_ERRCODE(n) ((n)->err_code)



/*
 *  Function declarations
 */

extern const char *TBnodeText( nodetype nt);


/*
 *  Definition of node data structure
 */

#include "sons.h"
#include "attribs.h"

struct NODE {
  nodetype             nodetype;       /* type of node */
  char*                file;
  int                  line;
  int                  col;            /* location data: file;line;col */
  node*                error;          /* error node */
#ifdef CLEANMEM
  struct SONUNION      sons;           /* the sons */
  struct ATTRIBUNION   attribs;        /* the nodes attributes */
#else
  union SONUNION       sons;           /* the sons */
  union ATTRIBUNION    attribs;        /* the nodes attributes */
#endif
  char*          err_code;       /* user defiable (metadata) error code */
};

#include "node_basic.h"

#endif /* _SAC_TREE_BASIC_H_ */
