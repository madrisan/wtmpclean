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

#if defined(HAVE_UTMPXNAME)
# define UTMPXNAME(wtmpfile) utmpxname(wtmpfile)
#elif defined(HAVE_UTMPNAME)
# define UTMPXNAME(wtmpfile) utmpname(wtmpfile);
#endif

/* Solaris : see http://docs.sun.com/app/docs/doc/816-5168/getutxent-3c?a=view
 * /var/adm/utmpx
 *   user access and adminstration information
 * /var/adm/wtmpx
 *   history of user access and adminstrative information
 *
 * #define    _UTMPX_FILE    "/var/adm/utmpx"
 * #define    _WTMPX_FILE    "/var/adm/wtmpx"
 */

#ifdef _PATH_WTMPX
/* Linux (glibc 2) */
# define DEFAULT_WTMP _PATH_WTMPX
#else
# ifdef _PATH_WTMP
/* Linux, FreeBSD, OpenBSD */
#  define DEFAULT_WTMP _PATH_WTMP
# else
#  ifdef WTMP_FILE
/* AIX */
#   define DEFAULT_WTMP WTMP_FILE
#  else
#   ifdef _WTMPX_FILE
/* OpenSolaris */
#    define DEFAULT_WTMP _WTMPX_FILE
#   endif
#  endif
# endif
#endif

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

/* Double linked list of struct utmpx's */
struct utmpxlist
{
    struct utmpx ut;
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

#endif /* WTMPCLEAN_H */
