#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(RusHub, 2.3.7[beta], [setuper@gmail.com])
AM_INIT_AUTOMAKE(RusHub, 2.3.7[beta])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CXX

AC_ARG_ENABLE(caps,
             [AC_HELP_STRING([--enable-caps],
                             [Enable posix capabilities for RusHub])],, enable_caps=no)
AC_ARG_ENABLE(sharedtinyxml,
             [AC_HELP_STRING([--disable-sharedtinyxml],
                             [Enable posix capabilities for RusHub])],, enable_sharedtinyxml=yes)
# Checks for libraries.
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main],, exit)
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main],, exit)

if test "$enable_sharedtinyxml" = "yes"; then
	AC_CHECK_LIB([tinyxml], [main],, exit)
fi
if test "$enable_caps" = "yes"; then
   AC_CHECK_LIB([cap], [main],, exit) 
fi

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h dlfcn.h fcntl.h memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h],, exit)

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
AC_CHECK_FUNCS([floor gettimeofday inet_ntoa memmove memset mkdir select setlocale socket strchr strdup strerror strrchr strstr strtoul uname dlerror dlopen dlclose dlsym],, exit)

if test "$enable_sharedtinyxml" = "yes"; then
	makestr="src/Makefile"
else
	makestr="src/tinyxml/Makefile src/Makefile"
fi
AM_CONDITIONAL([COND_OPT], [test "$enable_sharedtinyxml" = no])
AC_CONFIG_FILES([Makefile $makestr])
AC_OUTPUT
