/*
 * $Id: options.c 3768 2013-04-05 11:57:27Z vtn.nguyen $
 */


/*****************************************************************************
 * 
 * file:   options.c
 *
 * prefix: OPT
 *
 * description:
 *  This file provides means for the analysis of sac2c command line arguments.
 *  It uses the set macro definitions from main_args.h
 *   
 *****************************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "options.h"
#include "config.h"
#include "dbug.h"
#include "str.h"
#include "getoptions.h"
#include "usage.h"
#include "globals.h"
#include "memory.h"
#include "ctinfo.h"

// used for catenating metadata files (true filename is not known yet)
#define TEMP_FILENAME  "md.temp"
#define INCLUDE_BEGIN  "//#INCLUDE_BEGIN %s\n"
#define INCLUDE_END    "//#INCLUDE_END\n"
#define TEMP_FILETYPE  ".temp" 

#define STR_OR_EMPTY( str)  STR_OR_NULL( (str), "")
#define STR_OR_NULL( str, null_str) (((str) != NULL) ? (str) : (null_str))


#undef ARGS_ERROR
#define ARGS_ERROR( msg) 


/******************************************************************************
 *
 * function:
 *   void OPTcheckPreSetupOptions( int argc, char *argv[])
 *
 * description:
 *   
 *   Here, we check the command line only for very special arguments that
 *   require identification before anything else.
 *
 ******************************************************************************/

void OPTcheckPreSetupOptions( int argc, char *argv[])
{
  DBUG_ENTER("OPTcheckPreSetupOptions");

  ARGS_BEGIN( argc, argv);

  ARGS_OPTION_BEGIN( "d")
  {
    ARG_CHOICE_BEGIN();
#ifdef SHOW_MALLOC
    ARG_CHOICE("memcheck", global.memcheck = TRUE);
#endif
    ARG_CHOICE_END();
  }
  ARGS_OPTION_END( "d");

  ARGS_END();
  
  DBUG_VOID_RETURN;
}


/******************************************************************************
 *
 * function:
 *   void CheckOptionConsistency()
 *
 * description:
 *   This function is called from main() right after command line arguments
 *   have been analysed. Errors and warnings are produced whenever the user
 *   has selected an incompatible combination of options.
 *
 ******************************************************************************/

static
void CheckOptionConsistency( void)
{
  DBUG_ENTER("CheckOptionConsistency");

  DBUG_VOID_RETURN;
}


/******************************************************************************
 *
 * function:
 *   void OPTanalyseCommandline( int argc, char *argv[])
 *
 * description:
 *   This function analyses the commandline options given to sac2c.
 *   Usually selections made are stored in global variables for later
 *   reference.
 *
 ******************************************************************************/

#undef ARGS_ERROR
#define ARGS_ERROR( msg)                                                 \
{                                                                        \
  CTIerror( CTI_ERRNO_INVALID_OPTIONS, "%s: %s %s %s",                   \
            msg, ARGS_argv[0], STR_OR_EMPTY( OPT), STR_OR_EMPTY( ARG));  \
}

void OPTanalyseCommandline( int argc, char *argv[])
{
  DBUG_ENTER("OPTanalyseCommandline");
  
  FILE *mdfp = NULL;

  ARGS_BEGIN( argc, argv);


  /*
   * Options starting with aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
   */


  /*
   * Options starting with bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
   */

  ARGS_OPTION_BEGIN( "b") 
  {
    ARG = strtok(ARG, ":");
    ARG_RANGE(global.break_after, 1, 25);
    
    ARG = strtok(NULL, ":");
    if (ARG!=NULL) {
      global.break_specifier = STRcpy( ARG);
    }
  }
  ARGS_OPTION_END("b");

  
  /*
   * Options starting with ccccccccccccccccccccccccccccccccccccccccccc
   */

  ARGS_FLAG( "copyright", USGprintCopyright(); exit(0));

  ARGS_FLAG( "C", global.break_after = PH_codegen);

  ARGS_FLAG( "c", global.is_deployed = FALSE);

  /*
   * Options starting with ddddddddddddddddddddddddddddddddddddddddddd
   */
  ARGS_OPTION_BEGIN( "distrib")
  {
    ARG_CHOICE_BEGIN();
    ARG_CHOICE("scc",    global.distribution_lib = scc);
    ARG_CHOICE("mpi",    global.distribution_lib = mpi);
    ARG_CHOICE("nodist", global.distribution_lib = nodist);
    ARG_CHOICE_END();
  }
  ARGS_OPTION_END( "distrib");

  ARGS_OPTION_BEGIN( "d")
  {
    /*
     * CAUTION:
     * Due to -d memcheck the -d options is also identified in
     * presetup options and is repeated here only for technical
     * reasons. Any change in this option MUST be reflected in
     * OPTcheckPreSetupOptions().
     */
    ARG_CHOICE_BEGIN();
    ARG_CHOICE("treecheck",    global.treecheck = TRUE);
#ifdef SHOW_MALLOC
    ARG_CHOICE("memcheck",     global.memcheck = TRUE);
#endif
    ARG_CHOICE("efence",       global.use_efence = TRUE);
    ARG_CHOICE("nocleanup",    global.cleanup = FALSE);
    ARG_CHOICE("syscall",      global.show_syscall = TRUE);
    ARG_CHOICE_END();
  }
  ARGS_OPTION_END( "d");
  
  ARGS_OPTION_BEGIN( "D")
  { 
    if(global.cpp_call == NULL) {
      global.cpp_call = STRcat("-D", ARG);
    }
    else {
      char *tmp = STRcatn(3, global.cpp_call, " -D", ARG);
      MEMfree(global.cpp_call);
      global.cpp_call = tmp;
    }
  }
  ARGS_OPTION_END("D");
  


  /*
   * Options starting with eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
   */


  /*
   * Options starting with ggggggggggggggggggggggggggggggggggggggggggg
   */
   
  ARGS_FLAG( "g", global.cc_debug = TRUE);

 
  /*
   * Options starting with hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
   */

  ARGS_FLAG( "h", USGprintUsage(); exit(0));
  ARGS_FLAG( "help", USGprintUsage(); exit(0));


  /*
   * Options starting with iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
   */

  ARGS_OPTION_BEGIN( "I")
  {
    if(global.include_dirs == NULL) {
      global.include_dirs = STRcat("-I", ARG);
    } else {
      char *tmp = STRcatn(3, global.include_dirs, " -I", ARG);
      MEMfree( global.include_dirs);
      global.include_dirs = tmp;
    }
  }
  ARGS_OPTION_END("I");


  ARGS_FLAG( "ignoredist", global.enable_dist_snet = FALSE);


  /*
   * Options starting with kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
   */


  /*
   * Options starting with lllllllllllllllllllllllllllllllllllllllllll
   */
  ARGS_OPTION_BEGIN( "L")
  {
    if(global.clib_dirs == NULL) {
      global.clib_dirs = STRcat("-L", ARG);
    } else {
      char *tmp = STRcatn(3, global.clib_dirs, " -L", ARG);
      MEMfree( global.clib_dirs);
      global.clib_dirs = tmp;
    }
  }
  ARGS_OPTION_END("L");

  ARGS_OPTION_BEGIN( "l")
  {
    if(global.clibs == NULL) {
      global.clibs = STRcat("-l", ARG);
    } else {
      char *tmp = STRcatn(3, global.clibs, " -l", ARG);
      MEMfree( global.clibs);
      global.clibs = tmp;
    }
  }
  ARGS_OPTION_END("l");


  /*
   * Options starting with mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   */
  ARGS_OPTION_BEGIN( "m")
  { 
    FILE *fp = NULL;
    if((fp = fopen(ARG, "r")) != NULL) {
      if(mdfp == NULL) {
	if((mdfp = fopen(TEMP_FILENAME, "a")) == NULL) { 
	  CTIwarn(CTI_ERRNO_FILE_ACCESS_ERROR, 
		  "Could not create temporary file. Metadata ignored!");

	  // the same as ARGS_OPTION_END() 
	  ARGS_i += ARGS_shift;
	  continue;    
	}
      }

      fprintf(mdfp, "//#INCLUDE_BEGIN %s\n", ARG); 
      char c;
      
      while((c = getc(fp)) != EOF) {
	putc(c, mdfp);
      }
      
      putc('\n', mdfp);

      fprintf(mdfp, "//#INCLUDE_END\n"); 

      // Only count of metadata files is kept.
      // The actual names are needed, but they can be more easily stored later.

      global.mdfc++; 

      fclose(fp);
    } 
    else {
      CTIwarn(CTI_ERRNO_FILE_ACCESS_ERROR,
	      "Could not open metadata file: \"%s\"!", ARG); 
    }
  }
  ARGS_OPTION_END("m");

  /*
   * Options starting with nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
   */

  /*
   * Options starting with ooooooooooooooooooooooooooooooooooooooooooo
   */

  ARGS_OPTION( "O", ARG_RANGE(global.cc_optimize, 0, 3));

  ARGS_OPTION_BEGIN( "o")
    global.output_file = ARG;
  ARGS_OPTION_END("o");

  /*
   * Options starting with ppppppppppppppppppppppppppppppppppppppppppp
   */

  /*
   * Options starting with qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
   */

  /*
   * Options starting with rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
   */
  ARGS_OPTION_BEGIN( "runtime")
  {
    ARG_CHOICE_BEGIN();
    ARG_CHOICE("stream", global.runtime_lib = stream);
    ARG_CHOICE_END();
  }
  ARGS_OPTION_END( "runtime");

  /*
   * Options starting with sssssssssssssssssssssssssssssssssssssssssss
   */

  ARGS_FLAG( "static", global.is_static = TRUE);

  /*
   * Options starting with ttttttttttttttttttttttttttttttttttttttttttt
   */
  ARGS_OPTION_BEGIN( "threading")
  {
    ARG_CHOICE_BEGIN();
    ARG_CHOICE("lpel", global.threading_lib = lpel);
    ARG_CHOICE("lpel_hrc", global.threading_lib = lpel_hrc);
    ARG_CHOICE("pthread", global.threading_lib = pthread);
    ARG_CHOICE_END();
  }
  ARGS_OPTION_END( "threading");

  /*
   * Options starting with uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
   */

  ARGS_FLAG( "u", global.formatted_errors = FALSE);


  /*
   * Options starting with vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
   */

  ARGS_OPTION( "v", ARG_RANGE(global.verbose_level, 0, 3));

  ARGS_FLAG( "V", USGprintVersion(); exit(0));
   
  ARGS_FLAG( "VV", USGprintVersionVerbose(); exit(0));


  /*
   * Options starting with wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
   */
  ARGS_OPTION_BEGIN( "W")
  { 
    if(global.rpath == NULL) {
      global.rpath = STRcat("-W", ARG);
    }
    else {
      char *tmp = STRcatn(3, global.rpath, " -W", ARG);
      MEMfree( global.rpath);
      global.rpath = tmp;
    }
  }
  ARGS_OPTION_END("W");



  /*
   * Options starting with xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
   */

  /*
   * Options starting with yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
   */

  /*
   * Options starting with zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz
   */


  /*
   * Options starting with ###########################################
   */
  ARGS_OPTION_BEGIN( "#")
  {
    if (NULL == strchr( ARG, '/')) {
      global.my_dbug_str = STRcpy( ARG);
      global.my_dbug = 1;
      global.my_dbug_from = PH_initial;
      global.my_dbug_to = PH_final;
    }
    else {
      char *s;
      global.my_dbug_from = (compiler_phase_t) strtol( ARG, &s, 10);
      if (*s=='/') {
        s++;
      }
      else {
        ARGS_ERROR( "Invalid dbug phase specification");
      }
      
      global.my_dbug_to = (compiler_phase_t) strtol( s, &s, 10);
      if (*s=='/') {
        s++;
      }
      else {
        ARGS_ERROR( "Invalid dbug phase specification");
      }

      global.my_dbug_str = STRcpy( s);
      global.my_dbug = 1;
    }
  }
  ARGS_OPTION_END( "#");


  ARGS_ARGUMENT(
  {
    int i = 0;

    //Test if the code file given is valid
    if(access(ARG, R_OK) != 0) {
      CTIabort(CTI_ERRNO_FILE_ACCESS_ERROR,
	       "Could not access file: \"%s\"!", ARG);
    }

    global.mdfc++;
    global.mdfv = MEMmalloc(sizeof(char **) * global.mdfc);
    for(i = 0; i < global.mdfc; i++) {
      global.mdfv[i] = NULL;
    }

    if (global.pathname == NULL) {

      if(mdfp != NULL) {
	FILE *codefp = NULL;
	char *c = NULL;
	char *temp = NULL;

	global.pathname = STRcpy(ARG);

	// remove old file type identifier
	if((c = strstr(global.pathname, ".")) != NULL) {
	  for(; strlen(c) != 0; c++) {
	    *c = '\0';
	  }
	} 

	// temp file is done in the current directory, not in the source directory
	if((c = strrchr(global.pathname, '/')) != NULL) {
	  c++;
	}
	else {
	  c = global.pathname;
	}

	temp = global.pathname;

	global.pathname = STRcat(c, TEMP_FILETYPE);

	MEMfree(temp);
	c = NULL;
	temp = NULL;

	// copy code from code file to temp file (code file must not be modified!)
	if((codefp = fopen(ARG, "r")) != NULL) {
	  char c;
	  fprintf(mdfp, INCLUDE_BEGIN, ARG); 
	  while((c = getc(codefp)) != EOF) {	    
	    putc(c, mdfp);
	  }
	  fprintf(mdfp, INCLUDE_END); 
	  fclose(codefp);
	}
	else {
	  CTIabort(CTI_ERRNO_FILE_ACCESS_ERROR,
		   "Could not access file: \"%s\"!", ARG);
	}
	
	fclose(mdfp);
	
	// rename file
	if(rename (TEMP_FILENAME, global.pathname) == -1) {
	  CTIabort(CTI_ERRNO_FILE_ACCESS_ERROR,
		   "Could not create temporary file!");
	}
      }
      else {	
	global.pathname = STRcpy( ARG);
	global.mdfv[0] = STRcpy( ARG);
      }

      global.filename = strrchr(global.pathname, '/');
    
      if (global.filename == NULL) {
        global.filename = global.pathname;
      }
      else {
        global.filename += 1;
      }

      global.filebase = STRcpy(global.filename);
      char *ptr = strstr(global.filebase, ".");
      if(ptr) {
        *ptr = 0;
      }

    }
    else {
        ARGS_ERROR( "Too many source files specified");
    }
  });
  
  ARGS_UNKNOWN( ARGS_ERROR( "Invalid command line entry"));

  ARGS_END();

  CheckOptionConsistency();
  
  DBUG_VOID_RETURN;
}



