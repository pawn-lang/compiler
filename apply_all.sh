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

$patch "$patchdir/not_samp_compatible/32_bit_lines_in_debug_header.patch"
$patch "$patchdir/not_samp_compatible/new_debug_magic.patch"

$patch "$patchdir/samp_compatible/fix_md_array_init.patch"
$patch "$patchdir/samp_compatible/fix_triple_state_crash.patch"
$patch "$patchdir/samp_compatible/increase_line_length_limit.patch"
$patch "$patchdir/samp_compatible/stringize.patch"
