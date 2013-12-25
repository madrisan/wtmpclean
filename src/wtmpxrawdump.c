/*
 * wtmpclean.c -- A tool for dumping wtmp files and patching wtmp records.
 * Copyright (C) 2008,2009,2013 by Davide Madrisan <davide.madrisan@gmail.com>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include <arpa/inet.h>          /* inet_ntoa */
#include <errno.h>
/*#include <stdarg.h>*/
#include <time.h>
#include <unistd.h>
#include <utime.h>

#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#else
# ifdef HAVE_UTMP_H
#  include <utmp.h>
# endif
#endif

#include "wtmpclean.h"

char *
timetostr (const time_t time)
{
    static char s[20];          /* [2008.09.06 14:30:00] */

    if (time != 0)
        strftime (s, 20, "%Y.%m.%d %H:%M:%S", localtime (&time));
    else
        s[0] = '\0';

    return s;
}

void
wtmpxrawdump (const char *wtmpfile, const char *user)
{
    struct utmpx *utp;
    struct in_addr addr;
    char *addr_string, *time_string;

    if (access (wtmpfile, R_OK))
        die ("cannot access the file: %s\n", strerror (errno));

    UTMPXNAME (wtmpfile);
    setutxent ();

    while ((utp = getutxent ()))
      {
          if (user && strncmp (utp->ut_user, user, sizeof utp->ut_user))
              continue;

          /* FIXME: missing support for IPv6 */
#ifdef HAVE_UTP_UT_ADDR_V6
          addr.s_addr = utp->ut_addr_v6[0];
#endif

          addr_string = inet_ntoa (addr);
          time_string = timetostr (utp->ut_tv.tv_sec);

          switch (utp->ut_type)
            {
            default:
                /* Note: also catch EMPTY/UT_UNKNOWN values */
                printf ("%-9s", "NONE");
                break;
#ifdef RUN_LVL
                /* Undefined on AIX if _ALL_SOURCE is false */
            case RUN_LVL:
                printf ("%-9s", "RUNLEVEL");
                break;
#endif
            case BOOT_TIME:
                printf ("%-9s", "REBOOT");
                break;
            case OLD_TIME:
            case NEW_TIME:
                /* FIXME */
                break;
            case INIT_PROCESS:
                printf ("%-9s", "INIT");
                break;
            case LOGIN_PROCESS:
                printf ("%-9s", "LOGIN");
                break;
            case USER_PROCESS:
                printf ("%-9.*s", sizeof (utp->ut_user), utp->ut_user);
                break;
            case DEAD_PROCESS:
                printf ("%-9s", "DEAD");
                break;
#ifdef ACCOUNTING
                /* Undefined on AIX if _ALL_SOURCE is false */
            case ACCOUNTING:
                printf ("%-9s", "ACCOUNT");
                break;
#endif
            }

          /* pid */
          utp->ut_pid ? printf ("[%05d]", utp->ut_pid) : printf ("[%5s]",
                                                                 "-");

          /*     line      id       host      addr       date&time */
          printf
              (" [%-12.*s] [%-4.*s] [%-19.*s] [%-15.15s] [%-19.19s]\n",
               sizeof (utp->ut_line), utp->ut_line,
               sizeof (utp->ut_id), utp->ut_id,
               sizeof (utp->ut_host), utp->ut_host, addr_string, time_string);

      }

    endutxent ();
}
