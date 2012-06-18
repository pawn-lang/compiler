#!/bin/sh

me=`basename $0`

if [ $# -ne 1 ]
then
  echo "Usage: $me <source dir>"
  exit
fi

srcdir=$1

# Convert all CRLF to LF to fix patch errors on *nix
find $srcdir -name '*.c' -or -name '*.h' | xargs sed -i 's/\r//g'

# Remember the current directory
patchdir=`pwd`

# Go to the source directory and patch all files from there
cd "$srcdir"

patch="patch -p1 --force -i"

$patch "$patchdir/not_samp_compatible/32bitLinesInDebugHeader.patch"
$patch "$patchdir/not_samp_compatible/NewDebugMagic.patch"

$patch "$patchdir/samp_compatible/CompileTimeStringOps.patch"
$patch "$patchdir/samp_compatible/FixMDArrayInitialization.patch"
$patch "$patchdir/samp_compatible/IncreaseLineLimitTo4095.patch"
$patch "$patchdir/samp_compatible/TripleStateCrash.patch"
