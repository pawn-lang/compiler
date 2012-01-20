#!/bin/sh

me=`basename $0`

if [ $# -ne 1 ]
then
  echo "Usage: $me <source dir>"
  exit
fi

srcdir=$1
patch="patch --batch"

$patch "$srcdir/AMX/amxdbg.h"   "not_samp_compatible/AMX_amxdbg.h_32bit-lines-in-header.patch"

$patch "$srcdir/COMPILER/sc.h"  "samp_compatible/COMPILER_sc.h_increase-max-line-length.patch"
$patch "$srcdir/COMPILER/sc1.c" "samp_compatible/COMPILER_sc1.c_md-array-fix.patch"
$patch "$srcdir/COMPILER/sc2.c" "samp_compatible/COMPILER_sc2.c_stringize-ops.patch"
