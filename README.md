#  :busts_in_silhouette: wtmpclean

A tool for dumping wtmp files and patching wtmp records.

Usage

	wtmpclean [-l|-r] [-t "YYYY.MM.DD HH:MM:SS"] [-f <wtmpfile>] <user> [<fake>]

Where

	-f, --file   Modify <wtmpfile> instead of /var/log/wtmp
	-l, --list   Show listing of <user> logins
	-r, --raw    Show the raw content of the wtmp database
	-t, --time   Delete the login at the specified time

Examples

	wtmpclean --raw -f /var/log/wtmp.1 root
	wtmpclean -t "2008.09.06 14:30:00" jekyll hide
	wtmpclean -t "2013\.12\.?? 23:.*" hide
	wtmpclean -f /var/log/wtmp.1 jekyll

## Installation

This package uses GNU autotools for configuration and installation.

It is highly likely that you will want to customise the installation
location to suit your needs, i.e.:

	autoreconf && ./configure --prefix=/usr

Run `./configure --help` to see a list of available install options.

After `./configure` has completed successfully run `make install` and
you're done!

## Supported Platforms

This tool is written in plain C, making as few assumptions as possible, and
sticking closely to ANSI C/POSIX.

This is a list of platforms this nagios plugin is known to compile and run on

* Linux (with kernel 3.6 and glibc 2.16.0)

## Some documentation

* [XPG User Accounting Database Functions](http://www.gnu.org/software/libc/manual/html_node/XPG-Functions.html)
* [Open Group Base Specifications for the function *getutxent*](http://pubs.opengroup.org/onlinepubs/9699919799/functions/getutxent.html)

## Bugs

If you find a bug please create an issue in the
[project bug tracker](https://github.com/madrisan/wtmpclean/issues).

This tool uses some POSIX functions that are missing on some platforms:
Mac OS X 10.3, FreeBSD 6.0, OpenBSD 3.8, Minix 3.1.8, mingw, MSVC 9, BeOS.

