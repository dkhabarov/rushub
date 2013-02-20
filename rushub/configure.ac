#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define([package], [RusHub])
m4_define([version], [2.3.10])

AC_PREREQ([2.50])
AC_INIT([package], [version], [setuper@gmail.com])
AM_INIT_AUTOMAKE([package], [version])
AC_CONFIG_SRCDIR([src/config.h.in])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

export PATH=".:$am_aux_dir:$PATH"
dnl Get the OS type
AC_CANONICAL_HOST
case $host in
 *-*-linux*)
   SYSTEM=LINUX
   AC_DEFINE(HAVE_LINUX, 1, Linux environment)
   ;;
 *-*-cygwin*)
   SYSTEM=CYGWIN
   AC_DEFINE(HAVE_CYGWIN, 1, Cygwin environment)
   ;;
 *-*-solaris*)
   SYSTEM=SOLARIS
   AC_DEFINE(HAVE_SOLARIS, 1, Solaris environment)
   ;;
 *-*-freebsd*)
   SYSTEM=FREEBSD
   AC_DEFINE(HAVE_FREEBSD, 1, Freebsd environment)
   ;;
 *-*-hp*)
   SYSTEM=HP
   AC_DEFINE(HAVE_HP, 1, Hp environment)
   ;;
 *)
   SYSTEM=UNKNOWN
   ;;
esac
echo "Your OS is $host"

AC_ARG_ENABLE(caps,
    [AC_HELP_STRING([--enable-caps],
    [Enable posix capabilities for RusHub])],, enable_caps=no)
AC_ARG_ENABLE(sharedtinyxml,
    [AC_HELP_STRING([--enable-sharedtinyxml],
    [Enable shared tinyxml for RusHub])],, enable_sharedtinyxml=yes)
AC_ARG_ENABLE(sharedzlib,
    [AC_HELP_STRING([--enable-sharedzlib],
    [Enable shared zlib for RusHub])],, enable_sharedzlib=yes)
# Checks for libraries.

AC_CHECK_LIB([dl], [dlopen], [AC_SUBST(DLOPEN_LIBS, "-ldl")
    echo "dlopen is in libdl"
    ],[echo "dlopen is not defined in ldl"])
AC_CHECK_LIB([c], [dlopen], [AC_SUBST(DLOPEN_LIBS, "-lc")
    echo "dlopen is in libc"
    ],[echo "dlopen is not defined in libc"])

AC_CHECK_LIB([pthread], [pthread_create], [AC_SUBST(PTHREAD_LIBS, "-lpthread")
    AC_DEFINE(HAVE_PTHREAD, 1, Define to 1 if the system has pthread lib)
    ],[echo "pthread is not found"])

# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main], AC_SUBST(RT_LIBS, "-lrt"), exit)

if test "$enable_sharedzlib" = yes; then
    AC_CHECK_LIB([z], [main], [AC_SUBST(ZLIB_LIBS, "-lz")
        AC_DEFINE(HAVE_LIBZLIB, 1, Compile with shared zlib)], exit)
fi
if test "$enable_sharedtinyxml" = yes; then
    AC_CHECK_LIB([tinyxml], [main], [AC_SUBST(TINYXML_LIBS, "-ltinyxml")
        AC_DEFINE(HAVE_LIBTINYXML, 1, Compile with shared tinyxml)
        ],[enable_sharedtinyxml=no
        echo "shared tinyxml is not found"
        echo "use static tinyxml"
        ])
fi
if test "$enable_caps" = yes; then
    AC_CHECK_LIB([cap], [main], [AC_SUBST(CAPS_LIBS, "-lcap")
        AC_DEFINE(HAVE_LIBCAP, 1, Compile with posix capabilities)], exit) 
fi

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h dlfcn.h fcntl.h memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h stdint.h],, exit)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
#AC_CHECK_FUNCS([floor gettimeofday inet_ntoa memmove memset mkdir select setlocale socket strchr strdup strerror strrchr strstr strtoul uname dlerror dlopen dlclose dlsym],, exit)

RUSHUB_LIBS="$CAPS_LIBS $RT_LIBS $DLOPEN_LIBS $ZLIB_LIBS $PTHREAD_LIBS $TINYXML_LIBS"
AC_SUBST(RUSHUB_LIBS)
AC_SUBST([AM_LDFLAGS])
AC_SUBST([LIBS])

AM_CONDITIONAL([STATIC_TINYXML], [test "$enable_sharedtinyxml" = no])
AC_CONFIG_FILES([Makefile
    src/tinyxml/Makefile
		src/Makefile
		])

AC_OUTPUT
