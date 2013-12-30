#!/usr/bin/env python

import argparse
import os
import os.path
import subprocess
import sys

patch_opcions = ['-p1', '--force', '--binary', '-i']

# Patches compatible with the current SA-MP server.
patches_compat = [
  'samp_compatible/stringize.patch',
  'samp_compatible/fix_md_array_init.patch',
  'samp_compatible/fix_triple_state_crash.patch',
  'samp_compatible/increase_line_length_limit.patch',
]

# Incompatible patches (or patches that touch the VM).
patches_incompat = [
  'not_samp_compatible/debug_info.patch',
]

patches_all = patches_compat + patches_incompat

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument('-s', '--source', metavar='dir', required=True,
                        help='path to source directory')
arg_parser.add_argument('-a', '--all', action='store_true',
                        help='apply *all* patches (even incompatible ones)')
args = arg_parser.parse_args(sys.argv[1:])

home = os.path.dirname(os.path.realpath(__file__))
os.chdir(args.source)

patches = patches_all if args.all else patches_compat
for p in patches:
  print('aplying patch: ' + p)
  subprocess.call(['patch'] + patch_opcions + [os.path.join(home, p)])