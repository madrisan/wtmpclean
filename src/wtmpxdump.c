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
#include <time.h>
#include <unistd.h>
#include <signal.h>             /* kill */

#include "wtmpclean.h"

static void
dumprecord (struct utmpxlist *p, int what)
{
    char *ct;
    char logintime[32];
    char length[32];
    int mins, hours, days;

    printf ("%-8.8s %-12.12s %-16.16s ",
            p->ut.ut_user, p->ut.ut_line, p->ut.ut_host);

    ct = ctime (&p->ut.ut_tv.tv_sec);
    printf ("%10.10s %4.4s %5.5s ", ct, ct + 20, ct + 11);

    mins = (p->delta / 60) % 60;
    hours = (p->delta / 3600) % 24;
    days = p->delta / SECINADAY;

    if (days)
        sprintf (length, "(%d+%02d:%02d)", days, hours, mins);
    else
        sprintf (length, " (%02d:%02d)", hours, mins);

    switch (what)
      {
      case R_REBOOT:
          strcpy (logintime, " ");
          length[0] = 0;
          break;
/*    case R_CRASH:
 *        strcpy (logintime, "- crash");
 *        length[0] = 0;
 *        break;  */
      case R_DOWN:
          strcpy (logintime, "- down  ");
          break;
      case R_NOW:
          strcpy (logintime, "- still logged in");
          length[0] = 0;
          break;
      case R_PHANTOM:
          strcpy (logintime, "   gone - no logout");
          length[0] = 0;
          break;
      case R_NORMAL:
      default:
          ct = ctime (&p->eos);
          sprintf (logintime, "- %5.5s ", ct + 11);
      }
    printf ("%s%s\n", logintime, length);
}

void
wtmpxdump (const char *wtmpfile, const char *user)
{
    struct utmpxlist *p, *curr = NULL, *next, *utmpxlist = NULL;
    STRUCT_UTMP *utp;
    char runlevel;
    int down = 0;

    if (access (wtmpfile, R_OK))
        die (errno, "cannot access the file");

    /* Ignore the return value for now.
       Solaris' utmpname returns 1 upon success -- which is contrary
       to what the GNU libc version does.  In addition, older GNU libc
       versions are actually void.   */
    UTMP_NAME_FUNCTION (wtmpfile);

    SET_UTMP_ENT ();

    while ((utp = GET_UTMP_ENT ()) != NULL)
      {
          /*if (user && strncmp (UT_USER (utp), user, sizeof utp->ut_user))
             continue; */

          switch (utp->ut_type)
            {
            default:
                break;
            case RUN_LVL:
                runlevel = (UT_PID (utp) % 256);
                if ((runlevel == '0') || (runlevel == '6'))
                    down = 1;
                break;
            case BOOT_TIME:
                down = 0;
                break;
            case USER_PROCESS:
                /*
                 * Just store the data if it is interesting enough.
                 */
                if (strncmp (UT_USER (utp), user, sizeof (UT_USER (utp))) == 0)
                  {
                      if ((p = malloc (sizeof (struct utmpxlist))) == NULL)
                          die (errno, "out of memory");

                      memcpy (&p->ut, utp, sizeof (STRUCT_UTMP));
                      p->delta = 0;
                      p->ltype = R_NONE;
                      p->next = NULL;
                      if (utmpxlist == NULL)
                        {
                            utmpxlist = curr = p;
                            p->prev = NULL;
                        }
                      else
                        {
                            curr->next = p;
                            p->prev = curr;
                            curr = p;
                        }
                  }
                break;
            case DEAD_PROCESS:
                for (p = curr; p; p = p->prev)
                  {
                      if (p->ut.ut_type != USER_PROCESS || p->ltype != R_NONE)
                          continue;
                      if (strncmp
                          (p->ut.ut_line, utp->ut_line, sizeof utp->ut_line))
                          continue;

                      p->eos = UT_TIME_MEMBER (utp);
                      p->delta = UT_TIME_MEMBER (utp) - p->ut.ut_tv.tv_sec;
                      p->ltype = (down ? R_DOWN : R_NORMAL);
                  }
                break;
            }
      }

    END_UTMP_ENT ();

    for (p = utmpxlist; p; p = next)
      {
          next = p->next;

          if (p->ltype == R_NONE)
            {
                /* Is process still alive? */
                if (p->ut.ut_pid > 0 && kill (p->ut.ut_pid, 0) != 0
                    && errno == ESRCH)
                    p->ltype = R_PHANTOM;
                else
                    p->ltype = R_NOW;
            }

          dumprecord (p, p->ltype);
          free (p);
      }
}
