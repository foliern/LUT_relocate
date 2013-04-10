dnl
dnl $Id$
dnl

dnl parameters: package name, version, where to send bugreports, tarball-name
AC_INIT([SNet], [dev], [info@snet-home.org], [snet])
AC_PREREQ([2.68])

dnl set source directory and scripting directory
AC_CONFIG_SRCDIR([.])
AC_CONFIG_AUX_DIR([build-aux])

AC_CONFIG_FILES([makefiles/config.mkf])
AC_CONFIG_HEADER([config.h])

dnl check for host type
AC_CANONICAL_TARGET
dnl set OS and ARCH flags
AC_DEFINE_UNQUOTED([OS], ["$target_os"], [set to the os type])
AC_SUBST([OS], [$target_os])
AC_DEFINE_UNQUOTED([ARCH], ["$target_cpu"], [set to the architecture])
AC_SUBST([ARCH], [$target_cpu])

dnl check for ancillary programs
AC_PROG_RANLIB
AC_PROG_YACC
AC_PROG_LEX

AC_CHECK_PROGS([XSLT], [xsltproc sabcmd])
if test x"$XSLT" = xno; then
  AC_MSG_ERROR([Cannot find a usable xslt processor.])
fi

dnl we use C as our compiler language
AC_LANG([C])

AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS

AC_PROG_CPP

AC_SEARCH_LIBS([sqrt], [m])
AC_SEARCH_LIBS([dlopen], [dl])

AC_MSG_CHECKING([for 0x prefix when printing pointers])
need_ptr_prefix=no
AC_RUN_IFELSE([AC_LANG_PROGRAM([@%:@include <stdio.h>], [dnl
         char buffer[128];
         snprintf(buffer, 128, "%p", (void*)42);
         if (buffer[0] != 0 || buffer[1] != 'x') return 1;
         return 0;
        ])], [], [need_ptr_prefix=yes])
AC_MSG_RESULT([$need_ptr_prefix])
if test $need_ptr_prefix = yes; then
   AC_DEFINE([NEED_PTR_PREFIX], [1], [set to 1 if printf does not print a leading 0x in front of pointers])
fi

dnl check if stderr is a constant (needed for dbug.c)
AC_MSG_CHECKING([whether stderr is a constant])
AC_TRY_COMPILE([
@%:@include <stdio.h>
@%:@include <unistd.h>
@%:@include <stdlib.h>
FILE *_db_fp_ = stderr;
               ], [], [stderr_is_constant=yes], [stderr_is_constant=no])
AC_MSG_RESULT([$stderr_is_constant])
if test $stderr_is_constant = yes; then
  AC_DEFINE([STDERR_IS_CONSTANT], [1], [set to 1 if stderr is a constant])
fi

dnl check whether its suns cc
AC_MSG_CHECKING([whether cc is a Sun Pro C compiler])
AC_TRY_COMPILE([],
[@%:@ifndef __SUNPRO_C
  choke me
@%:@endif],
[AC_MSG_RESULT(yes)
SUNC=yes],
[AC_MSG_RESULT(no)
SUNC=no])

dnl check whether its compaqs (or dec) cc
AC_MSG_CHECKING([whether cc is a Compaq/DEC C compiler])
AC_TRY_COMPILE([],
[@%:@ifndef __DECC
  choke me
@%:@endif],
[AC_MSG_RESULT(yes)
DECC=yes],
[AC_MSG_RESULT(no)
DECC=no])

dnl check whether its apples cc
AC_MSG_CHECKING(whether cc is a Apple compiler)
AC_TRY_COMPILE(,
[@%:@ifndef __APPLE__
  choke me
@%:@endif],
  AC_MSG_RESULT(yes)
  MACC=yes
  GCC=no,
  AC_MSG_RESULT(no)
  MACC=no
)
  
dnl check for pthread flag on gcc
if test "$GCC" = yes; then
  AC_MSG_CHECKING([existence of pthread flag])
  echo "int main(){return 0;}" > .snet_configure_temp.c
  if test "`$CC -o .snet_configure_temp -pthread .snet_configure_temp.c 2>&1 | grep unrecognized `" = ""; then
    AC_MSG_RESULT([supported])
    GCC_PTHREADS="-pthread"
  else
    AC_MSG_RESULT([unsupported])
    GCC_PTHREADS="-lpthread"
  fi
  rm -f .snet_configure_temp.c .snet_configure_temp
fi

dnl test for compiler flags
dnl for sac2crc
AC_MSG_CHECKING([compiler flags])
if [ test "$GCC" = yes ]; then
  AC_SUBST(SOFLAGS, ["-shared"])
  AC_SUBST(MKCCFLAGS, ["-Wall -g -std=c99"])
  AC_SUBST(PDCCFLAGS, ["-Wall -O3 -std=c99"])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["$CC -E -C -x c"])
  AC_MSG_RESULT([gcc settings])
elif [ test "$SUNC" = yes ]; then
  AC_SUBST(SOFLAGS, ["-shared"])
  AC_SUBST(MKCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xc99=all"])
  AC_SUBST(PDCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -xO4 -xc99=all -KPIC"])
  AC_SUBST(DEPSFLAG, ["-xM"])
  AC_SUBST(CPPFILE, ["$CC -E -C -x c"])
  AC_MSG_RESULT([Sun cc settings])
elif [ test "$DECC" = yes ]; then
  AC_SUBST(SOFLAGS, ["-shared"])
  AC_SUBST(MKCCFLAGS, ["-g"])
  AC_SUBST(PDCCFLAGS, ["-g3"])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["$CC -E -C -x c"])
  AC_MSG_RESULT([Compaq/DEC cc settings])
elif [ test "$MACC" = yes ]; then
  AC_SUBST(SOFLAGS, ["-dynamiclib"])
  AC_SUBST(MKCCFLAGS, ["-Wall -std=c99 -g"])
  AC_SUBST(PDCCFLAGS, ["-std=c99"])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["$CC -E -C -x c"])
  AC_MSG_RESULT([Apple cc settings])
else
  AC_SUBST(SOFLAGS, ["-shared"])
  AC_SUBST(MKCCFLAGS, [""])
  AC_SUBST(PDCCFLAGS, [""])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["$CPP -C"])
  AC_MSG_RESULT([none found])
fi

dnl generate specific flags for known os
AC_MSG_CHECKING([sac2crc system flags])
OSFLAGS=
EXTRA_YYFLAGS=
case "$target_os" in
  solaris*)     
    OSFLAGS=["-D__EXTENSIONS__ -DMUST_INIT_YY -DPIC"]
    LD_PATH=-R
    if test "$target_os" = solaris2.8; then
       EXTRA_YYFLAGS=-DYYDEBUG=1
    fi
    AC_MSG_RESULT([solaris settings])
    ;;
  *linux*)      
    OSFLAGS=["-fPIC -DPIC -D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE -Dlint -D_POSIX_C_SOURCE=200112L"]
    LD_PATH=-Wl,-rpath,
    AC_MSG_RESULT([linux settings])
    ;;
  *osf*)	
    OSFLAGS=["-D_OSF_SOURCE"]
    LD_PATH=
    AC_MSG_RESULT([OSF settings])
    ;;
  *darwin*)      
    dnl darwin needs some extra flags
    case "$target_os" in
      *darwin10\.*|*darwin11\.*)
        #OSFLAGS=["-no-cpp-precomp"]
        LD_PATH=-Wl,-rpath,
        AC_MSG_RESULT([darwin 10.x settings])
        ;;
      *darwin9\.*)
        #OSFLAGS=["-no-cpp-precomp -Wno-long-double"]
        LD_PATH=-Wl,-rpath,
        AC_MSG_RESULT([darwin 9.x settings])
        ;;
      [*darwin[6-8]\.*)]
        #OSFLAGS=["-no-cpp-precomp -Wno-long-double"]
        LD_PATH=-L
        AC_MSG_RESULT([darwin 7.x/8.x settings])
      ;;
      *)
        AC_MSG_RESULT([unknown darwin system])
    esac
    ;;
  *bsd*)
    OSFLAGS=["-fpic"]
    LD_PATH=-Wl,-rpath,
    AC_MSG_RESULT([BSD settings])
    ;;
  *)
    AC_MSG_RESULT([unknown])
    ;;
esac
AC_SUBST([OSFLAGS])
AC_SUBST([EXTRA_YYFLAGS])

dnl check that the environment variables are properly set
AC_MSG_CHECKING([for S-Net run-time headers])
if test x"$SNET_INCLUDES" != x -a -r "$SNET_INCLUDES/snettypes.h"; then
  AC_MSG_RESULT([yes, -I\$SNET_INCLUDES])
  SNET_RT_INCLUDES=-I$SNET_INCLUDES
else
  AC_MSG_RESULT([no])
  AC_MSG_WARN([Please set the environment variable SNET_INCLUDES to the run-time include directory,])
  AC_MSG_WARN([otherwise snetc will not be able to compile programs.])
  SNET_RT_INCLUDES=
fi
AC_SUBST([SNET_RT_INCLUDES])

AC_MSG_CHECKING([for S-Net run-time libraries])
if test x"$SNET_LIBS" != x -a -r "$SNET_LIBS/libruntimestream.la"; then
  AC_MSG_RESULT([yes, -L\$SNET_LIBS])
  SNET_RT_RPATH=$LD_PATH$SNET_LIBS
else
  AC_MSG_RESULT([no])
  AC_MSG_WARN([Please set the environment variable SNET_LIBS to the run-time library directory,])
  AC_MSG_WARN([otherwise snetc will not be able to link programs.])
  AC_MSG_WARN([Hint: compile the S-Net run-time system *before* snetc!])
  SNET_RT_RPATH=
fi
AC_SUBST([SNET_RT_RPATH])

dnl generate configure dependent files
AC_OUTPUT
