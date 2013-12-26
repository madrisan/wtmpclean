# Copyright (C) 2008,2009 by Davide Madrisan <davide.madrisan@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ(2.60)
AC_INIT([wtmpclean], [0.8.0], [davide.madrisan@gmail.com])
AC_COPYRIGHT([Copyright 2008,2009,2013 by Davide Madrisan <davide.madrisan@gmail.com>])

AC_CONFIG_SRCDIR(src/wtmpclean.c)

AM_INIT_AUTOMAKE([check-news -Wall 1.9 dist-bzip2])

AC_CANONICAL_BUILD

AC_PROG_CC_STDC

AC_HEADER_TIME

AC_CHECK_HEADERS_ONCE([errno.h utmp.h utmpx.h])
if test $ac_cv_header_utmp_h = yes || test $ac_cv_header_utmpx_h = yes; then
  AC_CHECK_FUNC([utmpxname],
     [AC_DEFINE(HAVE_UTMPXNAME, 1,
                [Define to 1 to if you have the non-POSIX function utmpxname.])])
  AC_CHECK_FUNC([utmpname],
     [AC_DEFINE(HAVE_UTMPNAME, 1,
                [Define to 1 to if you have the non-POSIX function utmpname.])])
  AC_CHECK_DECLS([getutent],,,[[
/* <sys/types.h> is a prerequisite of <utmp.h> on FreeBSD 8.0, OpenBSD 4.6.  */
#include <sys/types.h>
#ifdef HAVE_UTMP_H
# include <utmp.h>
#endif
]])
    utmp_includes="\
AC_INCLUDES_DEFAULT
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#endif
#ifdef HAVE_UTMP_H
# if defined _THREAD_SAFE && defined UTMP_DATA_INIT
   /* When including both utmp.h and utmpx.h on AIX 4.3, with _THREAD_SAFE
      defined, work around the duplicate struct utmp_data declaration.  */
#  define utmp_data gl_aix_4_3_workaround_utmp_data
# endif
# include <utmp.h>
#endif
"
  AC_CHECK_MEMBERS([struct utmpx.ut_user],,,[$utmp_includes])
  AC_CHECK_MEMBERS([struct utmp.ut_user],,,[$utmp_includes])
  AC_CHECK_MEMBERS([struct utmpx.ut_name],,,[$utmp_includes])
  AC_CHECK_MEMBERS([struct utmp.ut_name],,,[$utmp_includes])
  AC_CHECK_MEMBERS([struct utmpx.ut_type],,,[$utmp_includes])
  AC_CHECK_MEMBERS([struct utmp.ut_type],,,[$utmp_includes])
  AC_CHECK_MEMBERS([struct utmpx.ut_pid],,,[$utmp_includes])
  AC_CHECK_MEMBERS([struct utmp.ut_pid],,,[$utmp_includes])
fi

#AC_CHECK_FUNC([getutxent],[],
#   [AC_MSG_ERROR([this system is not POSIX.1-2001 compliant])])
AC_CHECK_FUNC([getutxent],
   [AC_DEFINE(HAVE_UTMPXPOSIX, 1,
              [Define to 1 to if you have the POSIX function for managing utmpx databases.])])

# note: utp.ut_addr_v6 is only available on Linux
AC_CACHE_CHECK(
   [for ut_addr_v6 in struct utp],
   [ac_cv_utp_ut_addr_v6],
   [AC_RUN_IFELSE(
      [AC_LANG_SOURCE([[
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#else
# ifdef HAVE_UTMP_H
#  include <utmp.h>
# endif
#endif
int main(void)
{
   struct utmpx utp;
   utp.ut_addr_v6[0] = 0;
}
      ]])],
      [ac_cv_utp_ut_addr_v6="yes"],
      [ac_cv_utp_ut_addr_v6="no"],
      [ac_cv_utp_ut_addr_v6="no"])])
if test "x$ac_cv_utp_ut_addr_v6" = "xyes"; then
   AC_DEFINE(HAVE_UTP_UT_ADDR_V6, 1,
             [Define to 1 to if you have ut_addr_v6 in struct utp.])
fi

AC_CHECK_DECLS([getopt])

AC_CHECK_TOOL(LD,ld,ld,$PATH)
AC_CHECK_TOOL(AR,ar,ar,$PATH)
AC_PROG_RANLIB

AC_CONFIG_HEADERS(src/config.h:src/config.hin)
AC_CONFIG_FILES([
   Makefile
   src/Makefile
   src/missing/Makefile
])

AC_OUTPUT

cat <<_EOF

Good - your configure finished. Start make now.

_EOF
