/**
 * $Id$
 *
 * @file cgenwrap.c
 *
 * Generates code for box wrappers using a compiler plugin. The
 * plugin is loaded at compile time 
 *
 */


#include <string.h>
#include <assert.h>
#include <str.h>
#include <ltdl.h>

#include "cgenwrap.h"
#include "codefile.h"
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "lookup_table.h"
#include "print.h"
#include "type.h"

#include "mutil.h"

typedef enum { input, output, none} cgenwrap_mode_t;

struct INFO {
  cgenwrap_mode_t mode;
  char *interface_name;
  snetc_type_enc_t *input;
  int num_outputs;
  snetc_type_enc_t **outputs;
  int curr_output;
  int curr_entry; 
};

#define INFO_INTERFACE_NAME( n) ((n)->interface_name)
#define INFO_IN_TYPE( n)        ((n)->input)
#define INFO_OUT_TYPES( n)      ((n)->outputs)
#define INFO_NUM_OUTPUTS( n)    ((n)->num_outputs)
#define INFO_CURR_ENTRY( n)     ((n)->curr_entry)
#define INFO_MODE( n)           ((n)->mode)
#define INFO_CURR_OUTPUT( n)    ((n)->curr_output)

#define INFO_OUT_TYPE( arg, n)  INFO_OUT_TYPES( arg)[n]
#define TYPES( n)               (n)->type
#define TYPE( arg, num)         TYPES( arg)[num] 
#define NUM( n)                 (n)->num

static info *infoMake()
{
  info *new_inf;
  
  DBUG_ENTER("infoMake");
  
  new_inf = MEMmalloc( sizeof( info));
  INFO_INTERFACE_NAME( new_inf) = NULL;
  INFO_IN_TYPE( new_inf) = NULL;
  INFO_OUT_TYPES( new_inf) = NULL;
  INFO_CURR_ENTRY( new_inf) = 0;
  INFO_NUM_OUTPUTS( new_inf) = 0;
  INFO_CURR_OUTPUT( new_inf) = 0;
  INFO_MODE( new_inf) = none;
  DBUG_RETURN( new_inf);
}

static info *infoFree( info *inf)
{
#ifdef RUSSIA_GALORE
  int i;
  DBUG_ENTER("infofree");

  if( INFO_INTERFACE_NAME( inf) != NULL) {
    MEMfree( INFO_INTERFACE_NAME( inf));
  }

  if( INFO_IN_TYPE( inf) != NULL) {
    MEMfree( TYPES( INFO_IN_TYPE( inf)));
    MEMfree( INFO_IN_TYPE( inf));
  }
  if( INFO_OUT_TYPES( inf) != NULL) {
    for( i=0; i<INFO_NUM_OUTPUTS( inf); i++) {
      if( INFO_OUT_TYPE( inf, i) != NULL) {
        MEMfree( TYPES( INFO_OUT_TYPE( inf, i)));
      }
      MEMfree( INFO_OUT_TYPE( inf, i));
    }
    MEMfree( INFO_OUT_TYPES( inf));
    INFO_OUT_TYPES( inf) = NULL;
  }

  inf = MEMfree(inf);

  DBUG_RETURN(inf);
#else
  return( NULL);
#endif
}


static snet_type_t TypeOfEntry( node *arg_node) 
{
  snet_type_t res = 0;

  if( RECENTRIES_FIELD( arg_node) != NULL) {
    res = field;     
  } 
  else { 
    if( RECENTRIES_STAG( arg_node) != NULL) {
      res = tag;     
    } 
    else { 
      if( RECENTRIES_BTAG( arg_node) != NULL) {
        res = btag;     
      }
      else {
        /* error */
      CTIerror( CTI_ERRNO_INTERNAL_ERROR, 
               "Invalid type entry found while generating box wrapper");
      }
    }
  }
  return( res);
}

static void *loadWrapperPlugin( char *name_stub)
{
  int res;
  char *plugin_fqn;
  lt_dlhandle handle = NULL;

  plugin_fqn =
    MEMmalloc(
        ( strlen( "lib") +
          strlen( name_stub) +
          strlen( "c") +
          1)
        * sizeof( char));
  strcpy( plugin_fqn, "lib");
  strcat( plugin_fqn, name_stub);
  strcat( plugin_fqn, "c");

  CTInote("Loading plugin %s", plugin_fqn);

  res = lt_dlinit();
  if (res == 0) {
    lt_dlsetsearchpath(getenv("SNET_LIBS"));
    handle = lt_dlopenext( plugin_fqn );
  }

  if (handle == NULL) {
    CTIwarn(CTI_ERRNO_PLUGIN_ERROR,
        "Loading of plugin %s failed. No wrapper-code will "
        "be generated.\nError: %s\nSearch path: %s\n", plugin_fqn,
        lt_dlerror(), getenv("SNET_LIBS"));
  }

  MEMfree(plugin_fqn);

  return handle;
}

static void *loadWrapperGenerateFun( lt_dlhandle handle, char *name_stub)
{
  char *fun_fqn;
  void *sym;

  fun_fqn = 
    MEMmalloc( (strlen( name_stub) + strlen( "GenBoxWrapper") + 1)
            * sizeof( char));
  strcpy( fun_fqn, name_stub);
  strcat( fun_fqn, "GenBoxWrapper");

  CTInote("Loading wrapper generator %s", fun_fqn); 
  sym = lt_dlsym( handle, fun_fqn);
  if( sym == NULL) {
    CTIwarn(CTI_ERRNO_PLUGIN_ERROR,
	    "Loading wrapper-generator %s failed. No wrapper-code will "
      "be genrated. Error was: %s", fun_fqn, lt_dlerror());
  }
  MEMfree( fun_fqn);

  return( sym);
}

static char *generateWrapperCode( void *sym,
                                  const char *boxname,
                                  void *input_type,
                                  int num_outtypes,
                                  void *outtypes,
                                  snet_meta_data_enc_t *meta_data) 
{
  char *wrapper_code;
  char* ( *gen_fun)( const char*, void*, int, void*, void*) = sym;

  wrapper_code = gen_fun(boxname, input_type, num_outtypes, outtypes, meta_data);
  
  return( wrapper_code);
}
                                  
static int annotateWrapper( node *n, info *inf)
{
  void *plugin;
  void *wrapper_fun;
  char *the_code;
  snet_meta_data_enc_t *md = NULL;
  const char *boxname = NULL;
  int res;

  if( BOXDEF_WRAPCODE( n) == NULL) {
    plugin = loadWrapperPlugin( INFO_INTERFACE_NAME( inf));
    
    if( plugin != NULL) {
       wrapper_fun = 
         loadWrapperGenerateFun( plugin, INFO_INTERFACE_NAME( inf));
       if( wrapper_fun != NULL) {
	 
	 if(BOXDEF_METADATA( n) != NULL) {

	   md = MDUtilMetadataCreate(BOXDEF_METADATA( n));

	   if(md != NULL) {
	     boxname = MDUtilMetadataGet(md, METADATA_KEY_BOX_REAL_NAME);
	   }
	 } 

	 if(boxname == NULL) {
	   boxname = BOXDEF_REALNAME( n);
	 }

         the_code = 
           generateWrapperCode( wrapper_fun, 
                                boxname,
                                INFO_IN_TYPE( inf),
                                INFO_NUM_OUTPUTS( inf),
                                INFO_OUT_TYPES( inf),
                                md);

         BOXDEF_WRAPCODE( n) = 
           MEMmalloc( (strlen( the_code) + 1) * sizeof(char));
         strcpy( BOXDEF_WRAPCODE( n), the_code);
         free( the_code);
         
	       MDUtilMetadataDestroy(md);
       }

       lt_dlclose( plugin);
       res = lt_dlexit();
       assert(res == 0);

    }
  } 

  return( 0);
}

node *CGENWRAPrecentries( node *arg_node, info *arg_info)
{
  DBUG_ENTER("CGENWRAPrecentries");

  switch( INFO_MODE( arg_info)) {
    case input:
      NUM( INFO_IN_TYPE( arg_info)) += 1;
      if( RECENTRIES_NEXT( arg_node) != NULL) {
        TRAVdo( RECENTRIES_NEXT(arg_node), arg_info);
      } 
      else {
        TYPES( INFO_IN_TYPE( arg_info)) = 
          MEMmalloc( NUM( INFO_IN_TYPE( arg_info)) * sizeof( snet_type_t));
        INFO_CURR_ENTRY( arg_info) = NUM( INFO_IN_TYPE( arg_info));
      }
      INFO_CURR_ENTRY( arg_info) -= 1;
      TYPE( INFO_IN_TYPE( arg_info), INFO_CURR_ENTRY( arg_info)) = 
        TypeOfEntry( arg_node);
      break;
    case output:
      INFO_CURR_ENTRY( arg_info) += 1;
      if( RECENTRIES_NEXT( arg_node) != NULL) {
        TRAVdo( RECENTRIES_NEXT( arg_node), arg_info);
      }
      else {
        if( INFO_CURR_ENTRY( arg_info) > 1) {
          INFO_OUT_TYPE( arg_info, INFO_CURR_OUTPUT( arg_info)) =
            MEMmalloc( sizeof( snetc_type_enc_t));
          NUM( INFO_OUT_TYPE( arg_info, INFO_CURR_OUTPUT( arg_info))) = 
            INFO_CURR_ENTRY( arg_info);
          TYPES( INFO_OUT_TYPE( arg_info, INFO_CURR_OUTPUT( arg_info))) =
            MEMmalloc( INFO_CURR_ENTRY( arg_info) * sizeof( snet_type_t));
        }
      }
      INFO_CURR_ENTRY( arg_info) -= 1;
      if( INFO_CURR_ENTRY( arg_info) > 0) {
        TYPE( INFO_OUT_TYPE( arg_info, 
                             INFO_CURR_OUTPUT( arg_info)), 
              INFO_CURR_ENTRY( arg_info)) = TypeOfEntry( arg_node);
      }
      break;
    default:
      break; /* Do nothing */
  }

  DBUG_RETURN( arg_node);
}

node *CGENWRAPboxtypes( node *arg_node, info *arg_info) 
{
  DBUG_ENTER("CGENWRAPboxtypes");

  switch( INFO_MODE( arg_info)) {
    case input:
        if( BOXTYPES_ENTRIES( arg_node) != NULL) {
          TRAVdo( BOXTYPES_ENTRIES( arg_node), arg_info);
        }
        break;
   case output:
        if( BOXTYPES_NEXT( arg_node) != NULL) {
          INFO_NUM_OUTPUTS( arg_info) += 1;
          TRAVdo( BOXTYPES_NEXT( arg_node), arg_info);
        }
        else {
          INFO_OUT_TYPES( arg_info) = 
            MEMmalloc( INFO_NUM_OUTPUTS( arg_info) * sizeof( snetc_type_enc_t*));
          INFO_CURR_OUTPUT( arg_info) = INFO_NUM_OUTPUTS( arg_info);
        }
        
        INFO_CURR_OUTPUT( arg_info) -= 1;
        INFO_CURR_ENTRY( arg_info) = 0;
        if( BOXTYPES_ENTRIES( arg_node) != NULL) {
          TRAVdo( BOXTYPES_ENTRIES( arg_node), arg_info);
        }
          
      break;
   default:
      break; /* Do nothing */
  }
  
  DBUG_RETURN( arg_node);
}

node *CGENWRAPboxsign( node *arg_node, info *arg_info) 
{
  DBUG_ENTER("CGENWRAPboxsign");
  
  if( BOXSIGN_INTYPE( arg_node) != NULL) {
    INFO_MODE( arg_info) = input;
    TRAVdo( BOXSIGN_INTYPE( arg_node), arg_info);
    INFO_MODE( arg_info) = none;
  }

  if( BOXSIGN_OUTTYPES( arg_node) != NULL) {
    INFO_MODE( arg_info) = output;
    INFO_NUM_OUTPUTS( arg_info) = 1;
    TRAVdo( BOXSIGN_OUTTYPES( arg_node), arg_info);
    INFO_MODE( arg_info) = none;
  }

  DBUG_RETURN( arg_node);
}

node *CGENWRAPboxdef( node *arg_node, info *arg_info)
{
  const char *interface;

  DBUG_ENTER("CGENWRAPboxdef");
  
  if( BOXDEF_SIGN( arg_node) != NULL) {
    INFO_IN_TYPE( arg_info) = MEMmalloc( sizeof( snetc_type_enc_t));
    NUM( INFO_IN_TYPE( arg_info)) = 0;

    TRAVdo( BOXDEF_SIGN( arg_node), arg_info);
  }
  
  if( BOXDEF_METADATA( arg_node) != NULL) {
    interface = MDUtilMetadataGetKey(
        BOXDEF_METADATA( arg_node), 
        METADATA_KEY_BOX_LANGUAGE_INTERFACE);

    if(interface != NULL) {
      INFO_INTERFACE_NAME( arg_info) =  
        MEMmalloc( ( strlen(interface) + 1) * sizeof( char));
      strcpy( INFO_INTERFACE_NAME( arg_info), interface);
    }
  }
 
  if( (INFO_INTERFACE_NAME( arg_info) != NULL)) { 
    annotateWrapper( arg_node, arg_info);  
  }

  DBUG_RETURN( arg_node);
}

node *CGENWRAPdoCode(node *syntax_tree)
{
  info *inf;

  DBUG_ENTER("CGENWRAPdoCode");
  
  DBUG_ASSERT((syntax_tree != NULL), "CGENWRAP called with empty syntaxtree");

  inf = infoMake();

  TRAVpush(TR_cgenwrap);

  syntax_tree = TRAVdo(syntax_tree, inf);

  TRAVpop();

  infoFree(inf);

  DBUG_RETURN(syntax_tree);
}

