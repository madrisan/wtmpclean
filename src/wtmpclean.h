/* This file is part of wtmpclean', a tool for hacking the wtmp databases
 * Copyright (C) 2008,2009 by Davide Madrisan <davide.madrisan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WTMPCLEAN_H
#define WTMPCLEAN_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

# if HAVE_UTMPX_H
#  if HAVE_UTMP_H
    /* HPUX 10.20 needs utmp.h, for the definition of e.g., UTMP_FILE.  */
#   include <utmp.h>
#  endif
#  if defined _THREAD_SAFE && defined UTMP_DATA_INIT
    /* When including both utmp.h and utmpx.h on AIX 4.3, with _THREAD_SAFE
       defined, work around the duplicate struct utmp_data declaration.  */
#   define utmp_data gl_aix_4_3_workaround_utmp_data
#  endif
#  include <utmpx.h>
#  define UTMP_STRUCT_NAME utmpx
#  define UT_TIME_MEMBER(UT_PTR) ((UT_PTR)->ut_tv.tv_sec)
#  define SET_UTMP_ENT setutxent
#  define GET_UTMP_ENT getutxent
#  define END_UTMP_ENT endutxent
#  define PUT_UTMP_LINE pututxline
#  ifdef HAVE_UTMPXNAME
#   define UTMP_NAME_FUNCTION utmpxname
#  endif

# elif HAVE_UTMP_H

#  include <utmp.h>
#  define UTMP_STRUCT_NAME utmp
#  define UT_TIME_MEMBER(UT_PTR) ((UT_PTR)->ut_time)
#  define SET_UTMP_ENT setutent
#  define GET_UTMP_ENT getutent
#  define END_UTMP_ENT endutent
#  define PUT_UTMP_LINE pututline
#  ifdef HAVE_UTMPNAME
#   define UTMP_NAME_FUNCTION utmpname
#  endif

# endif

/* Accessor macro for the member named ut_user or ut_name.  */
# if HAVE_UTMPX_H

#  if HAVE_STRUCT_UTMPX_UT_USER
#   define UT_USER(Utmp) ((Utmp)->ut_user)
#  endif
#  if HAVE_STRUCT_UTMPX_UT_NAME
#   undef UT_USER
#   define UT_USER(Utmp) ((Utmp)->ut_name)
#  endif

# elif HAVE_UTMP_H

#  if HAVE_STRUCT_UTMP_UT_USER
#   define UT_USER(Utmp) ((Utmp)->ut_user)
#  endif
#  if HAVE_STRUCT_UTMP_UT_NAME
#   undef UT_USER
#   define UT_USER(Utmp) ((Utmp)->ut_name)
#  endif

# endif

# define HAVE_STRUCT_XTMP_UT_PID \
    (HAVE_STRUCT_UTMP_UT_PID \
     || HAVE_STRUCT_UTMPX_UT_PID)

typedef struct UTMP_STRUCT_NAME STRUCT_UTMP;

# if HAVE_STRUCT_XTMP_UT_PID
#  define UT_PID(U) ((U)->ut_pid)
# else
#  define UT_PID(U) 0
# endif

/* Solaris : see http://docs.sun.com/app/docs/doc/816-5168/getutxent-3c?a=view
 * /var/adm/utmpx
 *   user access and adminstration information
 * /var/adm/wtmpx
 *   history of user access and adminstrative information
 *
 * #define    _UTMPX_FILE    "/var/adm/utmpx"
 * #define    _WTMPX_FILE    "/var/adm/wtmpx"
 */

# if !defined UTMP_FILE && defined _PATH_UTMP
#  define UTMP_FILE _PATH_UTMP
# endif

# if !defined WTMP_FILE && defined _PATH_WTMP
#  define WTMP_FILE _PATH_WTMP
# endif

# ifdef UTMPX_FILE /* Solaris, SysVr4 */
#  undef UTMP_FILE
#  define UTMP_FILE UTMPX_FILE
# endif

# ifdef WTMPX_FILE /* Solaris, SysVr4 */
#  undef WTMP_FILE
#  define WTMP_FILE WTMPX_FILE
# endif

# ifndef UTMP_FILE
#  define UTMP_FILE "/etc/utmp"
# endif

# ifndef WTMP_FILE
#  define WTMP_FILE "/etc/wtmp"
# endif

#define SECINADAY (24*60*60)    /* seconds in a day */

/* Types of listing */
#define R_NONE        0
#define R_CRASH       1         /* No logout record, system boot in between */
#define R_DOWN        2         /* System brought down in decent way */
#define R_NORMAL      3         /* Normal */
#define R_NOW         4         /* Still logged in */
#define R_REBOOT      5         /* Reboot record. */
#define R_PHANTOM     6         /* No logout record but session is stale. */
#define R_TIMECHANGE  7         /* NEW_TIME or OLD_TIME */

/* Double linked list of struct UTMP_STRUCT_NAME's */
struct utmpxlist
{
    STRUCT_UTMP ut;
    time_t eos;                 /* end of session */
    time_t delta;               /* time difference */
    int ltype;                  /* R_NONE, R_CRASH, ... */
    struct utmpxlist *prev, *next;
};

void usage (int status);
void wtmpxdump (const char *wtmpfile, const char *user);
void wtmpxrawdump (const char *wtmpfile, const char *user);
unsigned int wtmpedit (const char *wtmpfile, const char *user,
                       const char *newuser, const char *timepattern,
                       unsigned int *cleanerr);
char *timetostr (const time_t time);
void die (const char *fmt, ...);

#endif /* WTMPCLEAN_H */
