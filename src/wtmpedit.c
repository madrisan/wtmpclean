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

#include <errno.h>
#include <regex.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include "wtmpclean.h"

unsigned int
wtmpedit (const char *wtmpfile, const char *user, const char *fake,
          const char *timepattern, unsigned int *cleanerr)
{
    STRUCT_UTMP *utp;
    unsigned int cleanrec;
    int rc;
    struct stat sb;
    struct utimbuf currtime;
    uid_t owner;
    gid_t group;
    regex_t regex;
    char msgbuf[100];

    if (access (wtmpfile, W_OK))
        die ("cannot access the file: %s\n", strerror (errno));

    if (stat (wtmpfile, &sb))
        die ("cannot get file status: %s\n", strerror (errno));

    if (rc = regcomp (&regex, timepattern, REG_EXTENDED | REG_NOSUB))
      {
          regerror (rc, &regex, msgbuf, sizeof (msgbuf));
          die ("regcomp() failed: %s\n", msgbuf);
      }

    currtime.actime = sb.st_atime;
    currtime.modtime = sb.st_mtime;
    owner = sb.st_uid;
    group = sb.st_gid;

    UTMP_NAME_FUNCTION (wtmpfile);
    SET_UTMP_ENT ();

    cleanrec = *cleanerr = 0;
    while ((utp = GET_UTMP_ENT ()))
      {
          if (utp->ut_type == USER_PROCESS &&
              strncmp (UT_USER (utp), user, sizeof (UT_USER (utp))) == 0)
            {
                if (regexec
                    (&regex, timetostr (UT_TIME_MEMBER (utp)), (size_t) 0, NULL,
                     0))
                    continue;

                if (fake)
                    strncpy (UT_USER (utp), fake, sizeof (UT_USER (utp)));
                else
                  {
                      /* Simulates the job of init when a process has exited:
                       * leave ut_pid untouched, sets ut_type to DEAD_PROCESS
                       * and fills ut_user, ut_host with null bytes
                       * ex:
                       * root [11735] [pts/0] [ts/0] [10.0.0.1] [10.0.0.1] [Mon Jan 12 17:31:24 2009 CET]
                       * DEAD [11735] [pts/0] [    ] [        ] [0.0.0.0 ] [Mon Jan 12 17:31:24 2009 CET]
                       */
                      utp->ut_type = DEAD_PROCESS;
                      memset (UT_USER (utp), 0, sizeof (UT_USER (utp)));
                      memset (utp->ut_id, 0, sizeof utp->ut_id);
                      memset (utp->ut_host, 0, sizeof utp->ut_host);
                      memset (utp->ut_addr_v6, 0, sizeof utp->ut_addr_v6);
                      /*UT_TIME_MEMBER (utp) = utp->ut_tv.tv_usec = 0; */
                  }

                if (PUT_UTMP_LINE (utp))
                    cleanrec++;
                else
                    cleanerr++;
            }
      }

    END_UTMP_ENT ();
    regfree (&regex);

    if (chown (wtmpfile, owner, group) < 0)
        fprintf (stderr, "cannot preserve the ownership of the wtmp file");
    if (utime (wtmpfile, &currtime) < 0)
        fprintf (stderr, "cannot preserve access and modification times");

    return cleanrec;
}
