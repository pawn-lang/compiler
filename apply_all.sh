#!/bin/sh

me=`basename $0`

if [ $# -ne 1 ]
then
  echo "Usage: $me <source dir>"
  exit
fi

srcdir=$1

patch "$srcdir/amx/amxdbg.h"   "not_samp_compatible/amx_amxdbg.h_32bit-lines-in-header.patch"

patch "$srcdir/compiler/sc.h"  "samp_compatible/compiler_sc.h_increase-max-line-length.patch"
patch "$srcdir/compiler/sc1.c" "samp_compatible/compiler_sc1.c_md-array-fix.patch"
patch "$srcdir/compiler/sc2.c" "samp_compatible/compiler_sc2.c_stringize-ops.patch"
