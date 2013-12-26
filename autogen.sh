#!/bin/sh
# Use this script to create generated files from the GIT dist

# You need autoconf 2.5x, preferably 2.57 or later
# You need automake 1.7 or later. 1.6 might work.

set -e

aclocal
autoheader
automake --foreign --add-missing --copy
autoconf --warnings=all
