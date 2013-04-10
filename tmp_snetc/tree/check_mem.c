/*
 * $Id: check_mem.c 2507 2009-08-05 12:02:11Z jju $
 */

/******************************************************************************
 *
 * PREFIX: CHKM
 *
 * description:
 *   the check mechanism for memory leaks
 *
 ******************************************************************************/

#ifdef SHOW_MALLOC

#include <stdlib.h>
#include <string.h>

#include "check_mem.h"
#include "dbug.h"

#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "check_lib.h"
#include "phase.h"
#include "check_node.h"
#include "check_attribs.h"
#include "globals.h"
#include "ctinfo.h"


/*
 * These types are only used to compute malloc_align_step.
 *
 * CAUTION:
 *
 * We need malloc_align_step in check_mem.c as well. Rather than using a single
 * global variable we use two static global variables, which are initialised
 * exactly in the same way, and of course need to be.
 */

typedef union {
  long int l;
  double d;
}
malloc_align_type;

typedef struct {
  int size;
  malloc_align_type align;
}
malloc_header_type;


const static int malloc_align_step
  = sizeof( malloc_header_type) - sizeof( malloc_align_type);


/**
 * INFO structure
 */
struct INFO {
  node  *error;
};

/**
 * INFO macros 
 */
#define INFO_ERROR(n)      (n->error)

/**
 * INFO functions
 */
static info *MakeInfo()
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = MEMmalloc( sizeof( info));
  
  INFO_ERROR( result)    = NULL;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER( "FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}


typedef struct MEMOBJ {
  int size;
  void *ptr;
  nodetype nodetype;
  char *file;
  int line;
  compiler_subphase_t subphase;
  const char* traversal;
  struct
  {
    unsigned int IsUsed:1;
    unsigned int IsShared:1;
    unsigned int IsReported:1;
  } flags;
} memobj;

#define MEMOBJ_SIZE(n) ((n)->size)
#define MEMOBJ_PTR(n) ((n)->ptr)
#define MEMOBJ_NODETYPE(n) ((n)->nodetype)
#define MEMOBJ_FILE(n) ((n)->file)
#define MEMOBJ_LINE(n) ((n)->line)
#define MEMOBJ_SUBPHASE(n) ((n)->subphase)
#define MEMOBJ_TRAVERSAL(n) ((n)->traversal)
#define MEMOBJ_USEDBIT(n) ((n)->flags.IsUsed)
#define MEMOBJ_SHAREDBIT(n) ((n)->flags.IsShared)
#define MEMOBJ_REPORTED(n) ((n)->flags.IsReported)


#define SHIFT2ORIG(n) (( memobj **) (((char *)(n)) - malloc_align_step))
#define SHIFT2MEMOBJ(n) ( *( SHIFT2ORIG(n)))

#define ORIG2SHIFT(n) ((( char *)(n)) + malloc_align_step)
#define ORIG2MEMOBJ(n) (*(memobj **) (n))

static void CHKManalyzeMemtab( memobj*, int);

static char *MemobjToErrorMessage( char*, memobj*);

static memobj *memtab = NULL;
static int memfreeslots = 0;
static int memindex = 0;
static int memtabsize = 0;

static int all_cnt_sharedmem = 0;
static int all_cnt_node_spaceleaks = 0;
static int all_cnt_non_node_spaceleaks = 0;
static int control_value = 0;




/** <!--********************************************************************-->
 *
 * @fn memobj *AllocateMemtab( int memtabsize) 
 *
 * 
 *
 *****************************************************************************/

static memobj *AllocateMemtab( int memtabsize)
{
  memobj *memtab;
  int size;

  DBUG_ENTER( "AllocateMemtab");

  size = memtabsize * sizeof (memobj);

  if (global.current_allocated_mem + size < global.current_allocated_mem){
    DBUG_ASSERT( (0), "counter for allocated memory: overflow detected");
  }

  global.current_allocated_mem += size;

  if (global.max_allocated_mem < global.current_allocated_mem) {
    global.max_allocated_mem = global.current_allocated_mem;
  }

  memtab = ( memobj*)malloc( size);

  DBUG_PRINT( "MEM_ALLOC", ("Alloc memory: %d Bytes at adress: " F_PTR,
                            size, memtab ));
  
  DBUG_PRINT( "MEM_TOTAL", ("Currently allocated memory: %u",
                            global.current_allocated_mem));

  DBUG_RETURN( memtab);
}

/** <!--********************************************************************-->
 *
 * @fn memobj *FreeMemtab( memobj *memtab, int memtabsize) 
 *
 * 
 *
 *****************************************************************************/
static void  *FreeMemtab( memobj *memtab, int memtabsize) 
{
  int size;

  DBUG_ENTER( "FreeMemtab");

  size = memtabsize * sizeof (memobj);

  DBUG_PRINT( "MEM_ALLOC", ("Free memory: %d Bytes at adress: " F_PTR,
                            size, memtab));
  
  if (global.current_allocated_mem < global.current_allocated_mem - size) {
    DBUG_ASSERT( (0), "counter for allocated memory: overflow detected");
  }

  global.current_allocated_mem -= size;
 
  DBUG_PRINT( "MEM_TOTAL", ("Currently allocated memory: %u",
                            global.current_allocated_mem));
  free(memtab);

  DBUG_RETURN ( (void *) NULL);
}

/** <!--********************************************************************-->
 *
 * @fn void *CHKMdeinitialize()
 *
 * the initial function
 *
 *****************************************************************************/
void CHKMdeinitialize()
{
  DBUG_ENTER( "CHKMdeinitialize");

  if ( global.memcheck) {
    global.current_allocated_mem -= memtabsize * sizeof( memobj);
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn node *CHKMdoMemCheck( node *syntax_tree)
 *
 * the traversal start function
 *
 *****************************************************************************/
node *CHKMdoMemCheck( node *syntax_tree) {

  info *info;

  DBUG_ENTER( "CHKMdoMemCheck");

  DBUG_PRINT( "CHKM", ("Traversing syntax tree..."));

  info = MakeInfo();

  TRAVpush( TR_chkm);
  syntax_tree = TRAVdo( syntax_tree, info);
  TRAVpop();

  info = FreeInfo( info);

  DBUG_PRINT( "CHKM", ("Syntax tree traversal complete"));

  DBUG_PRINT( "CHKM", ("Analyzing memory table..."));

  CHKManalyzeMemtab( memtab, memindex); 

  DBUG_PRINT( "CHKM", ("Analysis of memory table complete."));

  DBUG_RETURN( syntax_tree);
}


/** <!--********************************************************************-->
 *
 * @fn void *CHKMregisterMem( int size, void *orig_ptr)
 *
 *****************************************************************************/ 
void *CHKMregisterMem( int size, void *orig_ptr) 
{
  void *shifted_ptr = NULL;
  memobj* ptr_to_memobj;
  int i;
  
  DBUG_ENTER( "CHKMregisterMem");

  if( global.memcheck) {

    shifted_ptr = ORIG2SHIFT( orig_ptr);
    
    /* 
     * change the memtabsize if the memtab is full or empty 
     */
    if ( memindex == memtabsize) {
      
      /* 
       * at beginning the memtab ist empty so: 
       */
      if ( memtabsize == 0) {
        
        DBUG_PRINT( "CHKM", ("Allocating memtab for 1000 memory objects"
                             " size: %d bytes", 1000 * sizeof( memobj)));
        
        memtab = AllocateMemtab( 1000);
        memtabsize = 1000;
        memfreeslots = 1000;
      }
      
      /* 
       * expand or reduce mechanismn for the memtab 
       */
      else {
        int newtabsize = ( memtabsize - memfreeslots) * 2;
        int newindex = 0;
        memobj *newtab;
        
        DBUG_PRINT( "CHKM", ("Allocating memtab for %d memory objects"
                             " size: %d bytes", newtabsize, 
                             newtabsize * sizeof( memobj)));
        
        newtab = AllocateMemtab( newtabsize);
        
        /* 
         * copy the old memtab to new (smaller or bigger) memtab. All gaps
         * will ignore 
         */
        for ( i = 0; i < memtabsize; i++) {
          
          if( memtab[i].ptr != NULL) {
            memobj **tmpptr;
            
            newtab[newindex] = memtab[i];
            tmpptr = ( memobj**) newtab[newindex].ptr;
            *tmpptr = newtab + newindex;
            newindex ++;
          }
        }
        memtab = FreeMemtab( memtab, memtabsize);
        memtab = newtab;
        memindex = newindex;
        memfreeslots = newtabsize - newindex;
        memtabsize = newtabsize;
      }
    }

    /* 
     * <<<<<<<<  default entry for the memtab  >>>>>>>>> 
     */

    ptr_to_memobj = memtab + memindex;
    
    MEMOBJ_PTR( ptr_to_memobj)  = orig_ptr;
    MEMOBJ_SIZE( ptr_to_memobj) = size;
    MEMOBJ_NODETYPE( ptr_to_memobj) = N_undefined;
    MEMOBJ_SUBPHASE( ptr_to_memobj) = global.compiler_subphase;
    MEMOBJ_TRAVERSAL( ptr_to_memobj) = TRAVgetName();
    MEMOBJ_USEDBIT( ptr_to_memobj) = FALSE;
    MEMOBJ_SHAREDBIT( ptr_to_memobj) = FALSE;
    
    if ( global.compiler_subphase == SUBPH_initial) {
 
     MEMOBJ_REPORTED( ptr_to_memobj) = TRUE;
    } 
    else {
      MEMOBJ_REPORTED( ptr_to_memobj) = FALSE;
    } 

    *( memobj**) orig_ptr = ptr_to_memobj;
    
    memfreeslots = memfreeslots - 1;
    memindex = memindex + 1;
  }
  else {

    * (int*)orig_ptr = size;
    
    shifted_ptr = ORIG2SHIFT( orig_ptr);
  } 

  DBUG_RETURN( shifted_ptr);
}


/** <!--********************************************************************-->
 *
 * @fn void *CHKMunregisterMem( void *shiftet_ptr)
 *
 *   @brief  
 *
 *   @param *shifted_ptr
 *   @return (void *) orig_ptr
 *
 ******************************************************************************/
void *CHKMunregisterMem( void *shifted_ptr) 
{
  memobj *ptr_to_memobj;
  void *orig_ptr;
  
  DBUG_ENTER( "CHKMunregisterMEM");
  
  orig_ptr = SHIFT2ORIG( shifted_ptr);
  
  if( global.memcheck) {
    ptr_to_memobj = SHIFT2MEMOBJ( shifted_ptr);
    
    if ((MEMOBJ_SIZE( ptr_to_memobj) == 0) && 
        (MEMOBJ_PTR( ptr_to_memobj) == NULL)) {
      printf( "MEMORY WARNING: %s", "double free"); /* miss where */
    }
    
    MEMOBJ_SIZE( ptr_to_memobj) = 0;
    MEMOBJ_PTR( ptr_to_memobj) = NULL;
    memfreeslots = memfreeslots + 1;
  }

  DBUG_RETURN(( void *) orig_ptr);
}


/** <!--********************************************************************-->
 *
 * @fn node *CHKMtouch( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
void CHKMtouch( void *shifted_ptr, info *arg_info) 
{
  memobj *ptr_to_memobj;
  
  DBUG_ENTER( "CHKMprefun");

  if (shifted_ptr != NULL) {

    ptr_to_memobj = SHIFT2MEMOBJ( shifted_ptr);
    
    if (( memtab <= ptr_to_memobj) && 
        ( ptr_to_memobj < memtab + memtabsize) && 
        ( MEMOBJ_PTR( ptr_to_memobj) == SHIFT2ORIG( shifted_ptr))) {
      
      if ( MEMOBJ_USEDBIT( ptr_to_memobj)) {
        MEMOBJ_SHAREDBIT( ptr_to_memobj) = TRUE;
      }
      else {
        MEMOBJ_USEDBIT( ptr_to_memobj) = TRUE;
      }
    }
    else {
      /*
       * this is a Zombie --> dangling pointer 
       */
      if (INFO_ERROR( arg_info) == NULL) {
        INFO_ERROR( arg_info) = CHKinsertError( INFO_ERROR( arg_info), 
                                                "Dangling Pointer");
      }
    }
  }
  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn node *CHKMappendErrorNodes() node *arg_node, info *arg_info)
 *
 * This function is the true postfun of the CHKM Traversal. Called in 
 * CHKMpostfun.
 *
 * If a node is a spaceleak, the error nodes of his sons and attributes will 
 * be transfer to the next legal upper node.
 * 
 *****************************************************************************/
node *CHKMappendErrorNodes( node *arg_node, info *arg_info) 
{
  memobj *ptr_to_memobj;
  
  DBUG_ENTER( "CHKMappendErrorNodes");

  ptr_to_memobj = SHIFT2MEMOBJ( arg_node);
  
  if (( memtab <= ptr_to_memobj) && 
      ( ptr_to_memobj < memtab + memtabsize) && 
      ( MEMOBJ_PTR( ptr_to_memobj) == SHIFT2ORIG( arg_node)) &&
      ( INFO_ERROR( arg_info) != NULL)) {
    
    NODE_ERROR( arg_node) = TCappendError( NODE_ERROR( arg_node),
                                           INFO_ERROR( arg_info));
    INFO_ERROR( arg_info) = NULL; 
  }

  DBUG_RETURN( arg_node);
}


/** <!--********************************************************************-->
 *
 * @fn static void *CHKManalyzeMemtab( memobj *arg_memtab, int memindex)
 * @brief 
 *
 * @param *arg_memtab
 * @param arg_memindex
 *
 * @return void
 *
 *****************************************************************************/
static 
void CHKManalyzeMemtab( memobj *arg_memtab, int arg_memindex) 
{
  int index;
  memobj *ptr_to_memobj; 
  memobj *copy_memtab;

  int copy_index;
  node *arg_node;
  char *memtab_info;

  int cnt_sharedmem;
  int cnt_node_spaceleaks;
  int cnt_non_node_spaceleaks;


  DBUG_ENTER( "CHKManalyzeMemtab");
  
  /*
   *  must copy the memtab(!) to freeze, because Node_Error expand again the 
   *  memtab and so can possibly change the back pointers
   *
   */
  copy_memtab = AllocateMemtab( arg_memindex);

  for ( index = 0; index < arg_memindex; index++) {
    copy_memtab[index] = arg_memtab[index];
    MEMOBJ_USEDBIT( arg_memtab+index) = FALSE;
    MEMOBJ_SHAREDBIT( arg_memtab+index) = FALSE;
  }

  copy_index = arg_memindex;
  cnt_sharedmem = 0;
  cnt_node_spaceleaks = 0;
  cnt_non_node_spaceleaks = 0;

  /*
   *  traverse the copy memtab an the check any entry
   *
   * - 1. query: is it a legal entry?
   * - 2. query: is the entry of type Node or not
   * - 3. query: the object, where the entry point, 
   *             will be apperent not (longer) used, but isn't freed
   * - 4. query: the entry (error message) is reported yet
   *
   */ 
  for ( index = 0; index < copy_index; index++) {

    ptr_to_memobj = copy_memtab + index;

    if ( MEMOBJ_PTR( ptr_to_memobj) != NULL) {

       if ( MEMOBJ_NODETYPE( ptr_to_memobj) != N_undefined) {

        arg_node = (node *) ORIG2SHIFT( MEMOBJ_PTR( ptr_to_memobj));

         if ( !MEMOBJ_USEDBIT( ptr_to_memobj)) {
          
          if ( !MEMOBJ_REPORTED( ptr_to_memobj)) {
           
            memtab_info = MemobjToErrorMessage( "NODE SPACELEAK:", 
                                                ptr_to_memobj);
            
            /*
             * CTIwarn internaly frees memory that was allocated before the 
             * memtab has been copied in CHKMdoAnalyzeMemtab. Therefore it must
             * not be used to print the error string here.
             */
            fprintf( stderr, "MEMORY LEAK: %s\n", memtab_info);

            memtab_info = MEMfree( memtab_info);

            MEMOBJ_REPORTED( ORIG2MEMOBJ( MEMOBJ_PTR( ptr_to_memobj))) = TRUE;
            cnt_node_spaceleaks++;
          }
        }
         /*
          * - 3. query: the object, where the entry point,
          *             will shared 
          * - 4. query: the entry (error message) is reported yet
          */
        else {
          if ( MEMOBJ_SHAREDBIT( ptr_to_memobj)) {

            if ( !MEMOBJ_REPORTED( ptr_to_memobj)) {
            
              memtab_info = MemobjToErrorMessage( "SHARED MEMORY:", 
                                                  ptr_to_memobj);

              NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                                      memtab_info);
              memtab_info = MEMfree( memtab_info);

              MEMOBJ_REPORTED( ORIG2MEMOBJ( MEMOBJ_PTR( ptr_to_memobj))) = TRUE;
              cnt_sharedmem++;
            }
          }
        }
      } 
      else {

        /* 
         *  all entries of non node type:
         *
         * - 3. query: the object, where the entry point, 
         *             will be apperent not (longer) used, but isn't freed
         * - 4. query: the entry (error message) is reported yet

         */
        
        if ( !MEMOBJ_USEDBIT( ptr_to_memobj)) {

          if ( !MEMOBJ_REPORTED( ptr_to_memobj)) {

            memtab_info = MemobjToErrorMessage( "NON-NODE SPACELEAK:", 
                                                ptr_to_memobj);
       
            /*
             * CTIwarn internaly frees memory that was allocated before the 
             * memtab has been copied in CHKMdoAnalyzeMemtab. Therefore it must
             * not be used to print the error string here.
             */
            fprintf( stderr, "MEMORY LEAK: %s\n", memtab_info);
            
            memtab_info = MEMfree( memtab_info);

            MEMOBJ_REPORTED( ORIG2MEMOBJ( MEMOBJ_PTR( ptr_to_memobj))) = TRUE;
            cnt_non_node_spaceleaks++;
          }
        }

      }
    }
  }

  copy_memtab = FreeMemtab( copy_memtab, copy_index);

  if ((( cnt_node_spaceleaks != 0) || 
       ( cnt_sharedmem != 0) || 
       ( cnt_non_node_spaceleaks)) &&
      ((cnt_node_spaceleaks + cnt_sharedmem + cnt_non_node_spaceleaks) 
       != control_value)) {

    all_cnt_node_spaceleaks+=cnt_node_spaceleaks;
    all_cnt_non_node_spaceleaks+=cnt_non_node_spaceleaks;
    all_cnt_sharedmem+=cnt_sharedmem;
    
    CTIwarn( CTI_ERRNO_INTERNAL_ERROR,
             ">>>>> node spaceleaks:        %d\n"
             ">>>>> non-node spaceleaks:    %d\n"
             ">>>>> illegal memory sharing: %d\n",
             cnt_node_spaceleaks, cnt_non_node_spaceleaks, cnt_sharedmem);

    CTIwarn( CTI_ERRNO_INTERNAL_ERROR,
             ">>>>> node spaceleaks        (overall memory statistics): %d\n"
             ">>>>> non-node spaceleaks    (overall memory statistics): %d\n"
             ">>>>> illegal memory sharing (overall memory statistics): %d\n",
             all_cnt_node_spaceleaks, 
             all_cnt_non_node_spaceleaks, 
             all_cnt_sharedmem);

    
    control_value = 0;
    control_value = 
      cnt_node_spaceleaks + cnt_sharedmem + cnt_non_node_spaceleaks;
  }

 DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CHKMdoNotReport( void *shifted_ptr)
 *
 *****************************************************************************/
void CHKMdoNotReport( void *shifted_ptr) 
{
  memobj *ptr_to_memobj;

  DBUG_ENTER( "CHKMdoNotReport");

  if( global.memcheck) {
    ptr_to_memobj = SHIFT2MEMOBJ( shifted_ptr);
    
    MEMOBJ_REPORTED( ptr_to_memobj) = TRUE;
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CHKMsetNodeType( node *shifted_ptr, nodetype newnodetype)
 *
 *****************************************************************************/
void CHKMsetNodeType( node *shifted_ptr, nodetype newnodetype) 
{
  memobj *ptr_to_memobj;

  DBUG_ENTER( "CHKMsetNodeType");


  if( global.memcheck) {

    ptr_to_memobj = SHIFT2MEMOBJ( shifted_ptr);
    
    MEMOBJ_NODETYPE( ptr_to_memobj) = newnodetype;
  }

  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn void CHKMsetLocation( node *shiftet_ptr, char *file, int line)
 *
 *****************************************************************************/
void CHKMsetLocation( node *shifted_ptr, char *file, int line) 
{
  memobj *ptr_to_memobj;
  
  DBUG_ENTER( "CHKMsetLocation");

  if( global.memcheck) {

    ptr_to_memobj = SHIFT2MEMOBJ( shifted_ptr);
    
    MEMOBJ_FILE( ptr_to_memobj) = file;
    
    MEMOBJ_LINE( ptr_to_memobj) = line;
  }
  
  DBUG_VOID_RETURN;
}


/** <!--********************************************************************-->
 *
 * @fn int CHKMgetSize( node *shiftet_ptr)
 *
 *****************************************************************************/
int CHKMgetSize( node *shifted_ptr) 
{
  memobj *ptr_to_memobj;
  int *ptr_to_size;
  int size;

  DBUG_ENTER( "CHKMgetSize");

  if (global.memcheck) {
    
    ptr_to_memobj = SHIFT2MEMOBJ( shifted_ptr);
    
    size = MEMOBJ_SIZE( ptr_to_memobj);
  }
  else {

    ptr_to_size = (int *)SHIFT2ORIG (shifted_ptr);

    size = *ptr_to_size;

  }

  DBUG_RETURN( size);
}


static
char *MemobjToErrorMessage( char *kind_of_error, memobj *ptr_to_memobj)
{
  char *str;
  int test = 0;

  DBUG_ENTER( "CHKMtoString");

  str = (char *) MEMmalloc( sizeof( char) * 1024);
  
  test = snprintf( str, 1024, "%s Address: %p, Nodetype: %s,\n"
                   "             allocated at: %s:%d, \n"
                   "             Traversal: %s, Subphase: %s",
                   kind_of_error,
                   MEMOBJ_PTR( ptr_to_memobj),
                   TBnodeText( MEMOBJ_NODETYPE( ptr_to_memobj)),
                   MEMOBJ_FILE( ptr_to_memobj),
                   MEMOBJ_LINE( ptr_to_memobj),
                   MEMOBJ_TRAVERSAL( ptr_to_memobj),
                   PHsubPhaseName( MEMOBJ_SUBPHASE( ptr_to_memobj)));
  
  DBUG_ASSERT( test < 1024, "buffer is too small");

  DBUG_RETURN( str);
};

#endif /* SHOW_MALLOC */
