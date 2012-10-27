#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.5])
AC_INIT([abuhops], [0.0.1], [jasonlingle@yahoo.com])
AC_CONFIG_SRCDIR([src/abuhops.cxx])
AM_INIT_AUTOMAKE([1.11])
AM_SILENT_RULES(yes)
AM_CONFIG_HEADER([config.h])
# Disable Automake's braindead want to rerun ./configure if it thinks it
# needs to regenerate the entire build system
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_SEARCH_LIBS(__gmpz_powm, [gmp mpir], [],
[AC_MSG_ERROR([unable to locate a GMP or MPIR library])])

# If only the C++ standard had defined name mangling...
# Let's just hope for "crypto++" and die if it isn't called that here.
LIBS="$LIBS -lcrypto++"
echo -n "checking for libcrypto++... "
AC_LINK_IFELSE([AC_LANG_CALL([], [malloc])], [],
               [AC_MSG_ERROR([could not link against crypto++])])
echo "yes"

# Checks for header files.
AC_LANG_PUSH([C++])
AC_CHECK_HEADER([asio.hpp], [], [AC_MSG_ERROR([could not find asio.hpp])])
AC_CHECK_HEADER([gmp.h], [], [AC_MSG_ERROR([could not find gmp.h])])
AC_CHECK_HEADER([crypto++/sha.h], [], [AC_MSG_ERROR([could not find crypto++/sha.h])])
AC_CHECK_HEADER([crypto++/hmac.h], [], [AC_MSG_ERROR([could not find crypto++/hmac.h])])
AC_LANG_POP([C++])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
