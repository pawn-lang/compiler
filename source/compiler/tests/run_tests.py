#!/usr/bin/env python3

import argparse
import glob
import os.path
import re
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument('-c', '--compiler',
                    required=True,
                    help='path to the pawncc executable')
parser.add_argument('-i', '--include',
                    dest='include_dirs',
                    action='append',
                    help='add specified directory to include path')
options = parser.parse_args(sys.argv[1:])

def run_compiler(args):
    process_args = [';+', '-(+']
    if options.include_dirs is not None:
      for dir in options.include_dirs:
        process_args.append('-i' + dir)
    if args is not None:
      process_args += args
    return subprocess.Popen(executable=options.compiler,
                            args=process_args,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)

class OutputCheckTest:
  def __init__(self,
               name,
               source_file,
               errors=None,
               extra_args=None):
    self.name = name
    self.source_file = source_file
    self.errors = errors
    self.extra_args = extra_args

  def run(self):
    args = [self.source_file]
    if self.extra_args is not None:
      args += extra_args
    process = run_compiler(args=args)
    stdout, stderr = process.communicate()
    result = True
    if self.errors is None:
      if process.returncode != 0:
        result = False
        self.fail_reason = """
          No errors specified and process exited with non-zero status
        """
    else:
      errors = stderr.decode('utf-8').splitlines()
      errors = [e.strip() for e in errors if e.strip()]
      expected_errors = self.errors.splitlines()
      expected_errors = [e.strip() for e in expected_errors if e.strip()]
      if errors != expected_errors:
        result = False
        self.fail_reason = (
          'Error output didn\'t match\n\nExpected errors:\n\n{}\n\n'
          'Actual errors:\n\n{}'
        ).format(
          '\n'.join(expected_errors).strip(' \t\r\n'),
          '\n'.join(errors).strip(' \t\r\n')
        )
      return result

class CrashTest:
  def __init__(self, name, source_file, extra_args=None):
    self.name = name
    self.source_file = source_file
    self.extra_args = extra_args

  def run(self):
    # TODO: Check if the process crashed.
    return True

test_types = {
  'output_check': OutputCheckTest,
  'crash': CrashTest
}

tests = []
num_tests_disabled = 0
for meta_file in glob.glob('*.meta'):
  name = os.path.splitext(meta_file)[0]
  metadata = eval(open(meta_file).read(), None, None)
  if metadata.get('disabled'):
    num_tests_disabled += 1
    continue
  test_type = metadata['test_type']
  if test_type == 'output_check':
    tests.append(OutputCheckTest(name=name,
                                 source_file=name + '.pwn',
                                 errors=metadata.get('errors'),
                                 extra_args=metadata.get('extra_args')))
  elif test_type == 'crash':
    tests.append(CrashTest(name=name, source_file=name + '.pwn'))
  else:
    raise KeyError('Unknown test type: ' + test_type)

num_tests = len(tests)
sys.stdout.write('DISCOVERED {} TEST{}'.format(num_tests, '' if num_tests == 1 else 'S'))
if num_tests_disabled > 0:
  sys.stdout.write(' ({} DISABLED)'.format(num_tests_disabled))
sys.stdout.write('\n\n')

num_tests_failed = 0
for test in tests:
  sys.stdout.write('Running ' + test.name + '... ')
  if not test.run():
    sys.stdout.write('FAILED\n')
    print('Test {} failed for the following reason: {}'.format(
      test.name, test.fail_reason))
    print('')
    num_tests_failed += 1
  else:
    sys.stdout.write('PASSED\n')

num_tests_passed = len(tests) - num_tests_failed
if num_tests_failed > 0:
  print('\n{} TEST{} PASSED, {} FAILED'.format(
    num_tests_passed,
    '' if num_tests_passed == 1 else 'S',
    num_tests_failed))
  sys.exit(1)
else:
  print('\nALL TESTS PASSED')
