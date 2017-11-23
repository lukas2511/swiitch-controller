dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(librfn, 1.0)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([1.11 silent-rules foreign subdir-objects -Wall])
AM_SILENT_RULES([yes])

AC_PROG_CC_C99
AS_IF([test "x$ac_cv_prog_cc_c99" = "xno"], [AC_MSG_ERROR([compiler does not support C99])])
AM_PROG_CC_C_O
AM_PROG_AR
AC_PROG_RANLIB
gl_VALGRIND_TESTS
AC_SEARCH_LIBS([cos], [m])
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SEARCH_LIBS([clock_gettime], [rt],
	AM_CONDITIONAL(HAVE_CLOCK_GETTIME, true)
          AC_DEFINE(HAVE_CLOCK_GETTIME,1,[Have clock_gettime]),
	AM_CONDITIONAL(HAVE_CLOCK_GETTIME, false))
AC_CHECK_HEADERS([stdatomic.h],
	AM_CONDITIONAL(HAVE_STDATOMIC, true),
	AM_CONDITIONAL(HAVE_STDATOMIC, false)
	  AC_DEFINE(__STDC_NO_ATOMICS__,1,[Have C11 atomics]))

dnl Keep these near the bottom - adding -Werror breaks various tests
AX_CFLAGS_WARN_ALL
AX_APPEND_COMPILE_FLAGS([-Werror])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

