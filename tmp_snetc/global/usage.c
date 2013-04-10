/*
 * $Id: usage.c 3768 2013-04-05 11:57:27Z vtn.nguyen $
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "globals.h"
#include "build.h"
#include "dbug.h"
#include "usage.h"
#include "phase.h"



void USGprintUsage()
{
  DBUG_ENTER("usage");
  
  printf( "\n\n"
          "      snetc  --  The Ultimate S-Net Compiler\n"
          "    ------------------------------------------\n\n\n"
          
          "NAME:         snetc\n\n\n"

          "DESCRIPTION:\n\n"

          "    The snetc compiler transforms S-Net source code into S-Net modules.\n"
          "    suitable for further composition or for deployment.\n"
          "    \n"
          );
  

  printf( "\n\nSPECIAL OPTIONS:\n\n"
          
          "    -h              Display this helptext.\n"
          "    -help           Display this helptext.\n"
          "    -copyright      Display copyright/disclaimer.\n"
          "    -V              Display version identification.\n"
          "    -VV             Display verbose version identification.\n"
          "\n"
          "    NOTE:\n"
          "    When called with one of these options, snetc does not perform\n"
          "    any compilation steps.\n");

  
  printf( "\n\nGENERAL OPTIONS:\n\n"
          
          "    -D <var>        Set preprocessor variable <var>.\n"
          "    -D <var>=<val>  Set preprocessor variable <var> to <val>.\n"
          "    -I <path>       Specify path for preprocessor includes.\n"
          "    -L <path>       Specify path for library directories.\n"
	  "    -l <lib>        Search the library named <lib> when linking.\n"
          "\n"
          "    -C              Generate C-file only; do not invoke C compiler.\n"
          "    -c              Compile the source files, but do not link.\n"
          "    -o <file>       Place output in file \"file\".\n"
          "\n"
          "    -v <n>          Specify verbose level:\n"
          "                      0: error messages only,\n"
          "                      1: error messages and warnings,\n"
          "                      2: basic compile time information,\n"
          "                      3: full compile time information.\n"
          "                    (default: %d)\n"
          "\n"
          "    -u              Print unformatted error and warning messages.\n"
          "\n"
          "    -m <var>        Include metadata file.\n"
          "\n"
          "    -distrib <name> Specify the distribution implementation:\n"
          "                      nodist: Distribution functionality disabled (0)\n"
          "                      mpi: MPI distribution implementation (1)\n"
          "                      scc: Intel SCC distribution implementation (2)\n"
          "                    (default: %d)\n"
          "\n"
          "    -runtime <name> Specify the runtime implementation:\n"
          "                      stream: Stream based runtime implementation (0)\n"
          "                    (default: %d)\n"
          "\n"
          "    -threading <name> Specify the runtime implementation:\n"
          "                        pthread: Use pthread based threading (0)\n"
          "                        lpel: Use decentralised LPEL based threading (1)\n"
          "                        lpel_hrc: Use hierarchical LPEL based threading (2)\n"
          "                      (default: %d)\n"
          "\n",
          global.verbose_level, global.distribution_lib, global.runtime_lib,
          global.threading_lib);


  printf( "\n\nBREAK OPTIONS:\n\n"

          "    Break options allow you to stop the compilation process\n"
          "    after a particular phase.\n"
          );

  {
    compiler_phase_t tmp;
    
#define PHASEelement(e) tmp=PH_##e; printf("\n    -b%3d           Break after: %s\n", PH_##e
#define PHASEtext(t) ,t);                       \

#define SUBPHASEspec(s) printf("    -b%3d:%-7s     Break after: %s\n", tmp, s,
#define SUBPHASEtext(t) t);                     \
  
#include "phase.mac"

#undef SUBPHASEtext
#undef SUBPHASEspec
#undef PHASEtext
#undef PHASEelement
  }                                             \

  printf( "\n\nGENERAL DEBUG OPTIONS:\n\n"
          
          "    -d nocleanup    Do not remove temporary files and directories.\n"
/* 
 * This is currently not supported:
 *
 *        "    -d cccall       Generate shell script \".snetc\" that contains C compiler\n"
 *        "                    invocation.\n"
 *        "                    This implies option \"-d nocleanup\".\n"
 */
          "    -d syscall      Show all system calls during compilation.\n"
          );

  
  printf( "\n\nINTERNAL DEBUG OPTIONS:\n\n"
	  "    -d treecheck    Check syntax tree for consistency with xml specification. \n"
          "    -d memcheck     Check syntax tree for memory consistency.\n"
          "    -d efence       Link executable with ElectricFence (malloc debugger).\n"
          "\n"
          "    -# t            Display trace information generated by Fred Fish DBUG\n"
          "                    package.\n"
          "                    Each function entry and exit during program execution is\n"
          "                    printed on the screen.\n"
          "\n"
          "    -# d            Display debug output information generated by Fred Fish\n"
          "                    DBUG package.\n"
          "                    Each DBUG_PRINT macro in the code will be executed.\n"
          "                    Each DBUG_EXECUTE macro in the code will be executed.\n"
          "\n"
          "    -# d,<str>      Restrict \"-# d\" option to DBUG_PRINT / DBUG_EXECUTE macros\n"
          "                    which are tagged with the string <str> (no quotes).\n"
          "\n"
          "    -# <f>/<t>/<o>  Restrict the effect of any Fred Fish DBUG package option <o>\n"
          "                    to the range <f> to <t> of snetc compiler phases.\n"
          "                      (default: <f> = first compiler phase,\n"
          "                                <t> = last compiler phase.)\n"
          );

  
  printf( "\n\nC-COMPILER OPTIONS:\n\n"

          "    -g              Include debug information into object code.\n"
          "\n"
          "    -O <n>          Specify  the C compiler level of optimization.\n"
          "                      0: no C compiler optimizations.\n"
          "                      1: minor C compiler optimizations.\n"
          "                      2: medium C compiler optimizations.\n"
          "                      3: full C compiler optimizations.\n"
          "                    (default: %d)\n"
          "\n"
          "                    NOTE:\n"
          "                    The actual effects of these options are specific to the\n"
          "                    C compiler used for code generation. Both the choice of\n"
          "                    a C compiler as well as the mapping of these generic\n"
          "                    options to compiler-specific optimization options are\n"
          "                    are determined via the snetcrc configuration file.\n"
          "                    For details concerning snetcrc files see below under\n"
          "                    \"customization\".\n",
          global.cc_optimize);

  
  printf( "\n\nENVIRONMENT VARIABLES:\n\n"

          "SNET_LIBS           Library installation directory.\n"
          "SNET_INCLUDES       Header file installation directory.\n"
          );


  printf( "\n\nAUTHORS:\n\n"

          "    The following people contributed their time and mind to create snetc\n"
          "    (roughly in order of entering the project):\n"
          "\n"
          "      Clemens Grelck\n"
          "      Frank Penczek\n"
          "      Kari Keinanen\n"
          "      Jukka Julku\n"
          "      Haoxan Cai (alias Max Troy)\n"
          );
  


  printf( "\n\nCONTACT:\n\n"

          "    WWW:    http://www.snet-home.org/\n"
          "    E-Mail: info@snet-home.org\n");


  printf( "\n\nBUGS:\n\n"

          "    Bugs??  We????\n"
          "\n"
          "    snetc is a research compiler!\n"
          "\n"
          "    It is intended as a platform for scientific research rather than a\n"
          "    \"product\" for end users. Although we try to do our very best,\n"
          "    you may well run into a compiler bug. So, we are happy to receive\n"
          "    your bug reports (Well, not really \"happy\", but ...)\n"
          );

  printf( "\n\n");

  DBUG_VOID_RETURN;
}


void USGprintVersion()
{
  DBUG_ENTER("USGprintVersion");
   
  printf("snetc %s rev %s %s  (%s %s)\n",
         (global.version_id[0] == '\0') ? "???" : global.version_id, 
         (build_rev[0] == '\0') ? "???" : build_rev,
         (global.target_platform[0] == '\0') ? "???" : global.target_platform,
         (build_date[0] == '\0') ? "???" : build_date, 
         (build_user[0] == '\0') ? "???" : build_user);
  
  DBUG_VOID_RETURN;
}
         
  
void USGprintVersionVerbose()
{
  DBUG_ENTER("USGprintVerboseVersion");
   
  printf("\n"
         "      snetc  --  The Ultimate S-Net Compiler\n"
         "    ------------------------------------------\n\n\n"
                 
         "NAME:          snetc\n"
         "VERSION:       %s\n"
         "PLATFORM:      %s\n"
         "\n"

         "BUILD:         %s\n"
         "BY USER:       %s\n"
         "ON HOST:       %s\n"
         "FOR OS:        %s\n"
         "\n"

         "\n",
         (global.version_id[0] == '\0') ? "???" : global.version_id, 
         (global.target_platform[0] == '\0') ? "???" : global.target_platform,
         (build_date[0] == '\0') ? "???" : build_date, 
         (build_user[0] == '\0') ? "???" : build_user,
         (build_host[0] == '\0') ? "???" : build_host,
         (build_os[0] == '\0') ? "???" : build_os);
  
  printf("(c) Copyright 2006-2007 by\n\n"

         "  University of Hertfordshire\n\n"

         "  http://snet.feis.herts.ac.uk/\n"
         "  email:snet-devel@yahoogroups.com\n");
  
  DBUG_VOID_RETURN;
}


void USGprintCopyright()
{
  DBUG_ENTER("USGprintCopyright");
   
  printf("\n"
         "      snetc  --  The Ultimate S-Net Compiler\n"
         "    ------------------------------------------\n\n\n"

         "    S-NET COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER\n\n"

         "(c) Copyright 2006-2007 by\n\n"

         "  University of Hertfordshire\n\n"

         "  http://snet.feis.herts.ac.uk/\n"
         "  email:snet-devel@yahoogroups.com\n");

  printf("The S-Net compiler, the S-Net deployer, the S-Net runtime system,"
         "and all accompanying software and documentation (in the following"
         "named this software) is developed by the University of Hertfordshire"
         "(in the following named the developer), which reserves all rights"
         "on this software.\n"
         "\n"
         "Permission to use this software is hereby granted free of charge\n"
         "for any non-profit purpose in a non-commercial environment, i.e. for\n"
         "educational or research purposes in a non-profit institute or for\n"
         "personal, non-commercial use. For this kind of use it is allowed to\n"
         "copy or redistribute this software under the condition that the\n"
         "complete distribution for a certain platform is copied or\n"
         "redistributed and this copyright notice, license agreement, and\n"
         "warranty disclaimer appears in each copy. ANY use of this software with\n"
         "a commercial purpose or in a commercial environment is not granted by\n"
         "this license.\n"
         "\n"
         "The developer disclaims all warranties with regard to this software,\n"
         "including all implied warranties of merchantability and fitness.  In no\n"
         "event shall the developer be liable for any special, indirect or\n"
         "consequential damages or any damages whatsoever resulting from loss of\n"
         "use, data, or profits, whether in an action of contract, negligence, or\n"
         "other tortuous action, arising out of or in connection with the use or\n"
         "performance of this software. The entire risk as to the quality and\n"
         "performance of this software is with you. Should this software prove\n"
         "defective, you assume the cost of all servicing, repair, or correction.\n\n");

#if 0
  printf("Permission to use, copy, modify, and distribute this software and its\n"
         "documentation for any purpose and without fee is hereby granted,\n"
         "provided that the above copyright notice appear in all copies and that\n"
         "both the copyright notice and this permission notice and warranty\n"
         "disclaimer appear in supporting documentation, and that the name of\n"
         "CAU Kiel or any CAU Kiel entity not be used in advertising\n"
         "or publicity pertaining to distribution of the software without\n"
         "specific, written prior permission.\n\n"

         "CAU Kiel disclaims all warranties with regard to this software, including\n"
         "all implied warranties of merchantability and fitness.  In no event\n"
         "shall CAU Kiel be liable for any special, indirect or consequential\n"
         "damages or any damages whatsoever resulting from loss of use, data or\n"
         "profits, whether in an action of contract, negligence or other\n"
         "tortious action, arising out of or in connection with the use or\n"
         "performance of this software.\n\n");
#endif

  DBUG_VOID_RETURN;
}
  

