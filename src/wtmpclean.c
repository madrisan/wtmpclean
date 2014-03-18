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

/*
 * indent -i4 -nut src/wtmpclean.c
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define _GNU_SOURCE

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

#include <locale.h>             /* setlocale */
#include <pwd.h>                /* getpwnam */
#include <regex.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

/* FIXME */
/*#if !defined HAVE_UTMPXPOSIX
 *# include "wrappers.h"
 *#endif
 */

#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#else
# ifdef HAVE_UTMP_H
#  include <utmp.h>
# endif
#endif

#include "wtmpclean.h"
#include "getopt.h"

static const char *progname;

/*
 *	Get the basename of a filename
 */
static char *mybasename(char *s)
{
    char *p;

    if ((p = strrchr(s, '/')) != NULL)
        p++;
    else
        p = s;
    return p;
}

void
usage (int status)
{
    static const char *usagemsg[] = {
        PACKAGE " version " PACKAGE_VERSION,
        "A tool for dumping wtmp files and patching wtmp records.",
        "Copyright (C) 2008,2009,2013 by Davide Madrisan <davide.madrisan@gmail.com>",
        "",
        "Usage: " PACKAGE " [-l|-r] [-t \"YYYY.MM.DD HH:MM:SS\"]"
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
            " [-f <wtmpfile>]"
#endif
            " <user> [<fake>]",
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
        "  -f, --file       Modify <wtmpfile> instead of " WTMP_FILE,
#endif
        "  -l, --list       Show listing of <user> logins",
        "  -r, --raw        Show the raw content of the wtmp database",
        "  -t, --time       Delete the login at the specified time",
        "",
        "Samples:",
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
        "  ./" PACKAGE " --raw -f " WTMP_FILE ".1 root",
        "  ./" PACKAGE " -t \"2008.09.06 14:30:00\" jekyll hide",
        "  ./" PACKAGE " -t \"2013\\.12\\.?? 23:.*\" hide",
        "  ./" PACKAGE " -f " WTMP_FILE ".1 jekyll",
#else
        "  ./" PACKAGE " root",
#endif
        "",
        "This is free software; see the source for copying conditions.  There is NO",
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
        (char *) 0
    };

    unsigned int i;

    for (i = 0; usagemsg[i]; i++)
        fprintf (status ? stderr : stdout, "%s\n", usagemsg[i]);

    exit (status);
}

void
die (int err_no, const char *fmt, ...)
{
    va_list args;

    fflush (NULL);

    va_start (args, fmt);
    fprintf (stderr, "%s: ", progname);
    vfprintf (stderr, fmt, args);
    if (err_no)
        fprintf (stderr, ": %s\n", strerror (err_no));
    else
        putc ('\n', stderr);
    va_end (args);

    exit (EXIT_FAILURE);
}

static void
userchk (const char *usr)
{
    struct passwd *pw;

    if (usr && !(pw = getpwnam (usr)))
      {
          fprintf (stderr, "%s: unknown/bad user `%s'\n", progname, usr);
          exit (EXIT_FAILURE);
      }
}

int
main (int argc, char **argv)
{
    char *wtmpfile =
#ifdef HAVE___SECURE_GETENV
    __secure_getenv (WTMP_FILE) ? : WTMP_FILE;
#else
# ifdef HAVE_SECURE_GETENV
    secure_getenv (WTMP_FILE) ? : WTMP_FILE;
# else
    getenv (WTMP_FILE) ? : WTMP_FILE;
# endif
#endif
    char *user = NULL, *fake = NULL, *timepattern = ".*";;
    unsigned char dump = 0, rawdump = 0, numeric = 0;

    int opt_index = 0;
    unsigned int cleanrec, cleanerr;

    setlocale (LC_ALL, "C");

    progname = argv[0] ? mybasename (argv[0]) : PACKAGE;
    opterr = 0;

    while (1)
      {
          static struct option long_options[] = {
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
              {"file", required_argument, 0, 'f'},
#endif
              {"list", no_argument, 0, 'l'},
              {"numeric", no_argument, 0, 'n'},
              {"raw", no_argument, 0, 'r'},
              {"time", required_argument, 0, 't'},
              {"help", no_argument, 0, 'h'},
              {0, 0, 0, 0}
          };
          static const char *options =
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
              "f:"
#endif
              "lrnt:h";

          int opt =
              getopt_long (argc, argv, options, long_options, &opt_index);
          if (opt == -1)
              break;

          switch (opt)
            {
            default:
                usage ((opt == 'h') ? EXIT_SUCCESS : EXIT_FAILURE);
                break;
            case 'f':
                wtmpfile = optarg;
                break;
            case 'l':
                if (rawdump)
                    usage (EXIT_FAILURE);
                dump = 1;
                break;
            case 'n':
                /* FIXME : not implemented yet */
                numeric = 1;
                break;
            case 'r':
                if (dump)
                    usage (EXIT_FAILURE);
                rawdump = 1;
                break;
            case 't':
                timepattern = optarg;
                break;
            }
      }

    if (argc == optind + 1)
        user = argv[optind];
    else if (argc == optind + 2)
      {
          user = argv[optind];

          fake = argv[optind + 1];
          userchk (fake);
      }
    else if (!((argc == optind) && rawdump))
        usage (EXIT_FAILURE);

    if (dump)
      {
          wtmpxdump (wtmpfile, user);
          exit (EXIT_SUCCESS);
      }
    else if (rawdump)
      {
          wtmpxrawdump (wtmpfile, user);
          exit (EXIT_SUCCESS);
      }

    userchk (user);
    cleanrec = wtmpedit (wtmpfile, user, fake, timepattern, &cleanerr);
    if (cleanerr > 0)
      {
          fprintf (stderr, "%s: cannot clean up %s\n", progname, wtmpfile);
          exit (EXIT_FAILURE);
      }

    if (fake)
        printf
            ("%s: %u block(s) logging user `%s' now belong to user `%s'.\n",
             wtmpfile, cleanrec, user, fake);
    else
        printf ("%s: patched %u block(s) logging user `%s'.\n",
                wtmpfile, cleanrec, user);

    return 0;
}
