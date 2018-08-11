#!/usr/bin/env python

import argparse
import glob
import os.path
import re
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument('-c', '--compiler',
                    required=True,
                    help='path to compiler executable (pawncc or pawncc.exe)')
parser.add_argument('-i', '--include',
                    dest='include_dirs',
                    action='append',
                    help='add custom include directories for compile tests')
parser.add_argument('-r', '--runner',
                    required=True,
                    help='path to runner executable (pawnrun or pawnrun.exe)')
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
  def __init__(self, name, errors=None, extra_args=None):
    self.name = name
    self.errors = errors
    self.extra_args = extra_args

  def run(self):
    args = [self.name + '.pwn']
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
      errors = stderr.decode('utf-8').strip(' \t\r\n').replace('\r', '')
      expected_errors = self.errors.strip(' \t\r\n').replace('\r', '')
      if errors != expected_errors:
        result = False
        self.fail_reason = (
          'Error output didn\'t match\n\nExpected errors:\n\n{}\n\n'
          'Actual errors:\n\n{}'
        ).format(expected_errors, errors)
      return result

class RuntimeTest:
  def __init__(self, name, output, should_fail):
    self.name = name
    self.output = output
    self.should_fail = should_fail

  def run(self):
    process = run_compiler([self.name + '.pwn'])
    stdout, stderr = process.communicate()
    if process.returncode != 0:
      self.fail_reason = \
        'Compiler exited with status {}'.format(process.returncode)
      errors = stderr.decode('utf-8')
      if errors:
        self.fail_reason += '\n\nErrors:\n\n{}'.format(errors)
      return False
    process = subprocess.Popen([options.runner, self.name + '.amx'],
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    output = ''
    if stdout is not None:
      output = stdout.decode('utf-8')
    if stderr is not None:
      output += stderr.decode('utf-8')
    output = output.strip(' \t\r\n').replace('\r', '')
    expected_output = self.output.strip(' \t\r\n').replace('\r', '')
    if not self.should_fail and process.returncode != 0:
      self.fail_reason = (
        'Runner exited with status {}\n\nOutput: {}'
      ).format(process.returncode, output)
      return False
    if output != expected_output:
      self.fail_reason = (
        'Output didn\'t match\n\nExpected output:\n\n{}\n\n'
        'Actual output:\n\n{}'
      ).format(expected_output, output)
      return False
    return True

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
                                 errors=metadata.get('errors'),
                                 extra_args=metadata.get('extra_args')))
  elif test_type == 'crash':
    tests.append(CrashTest(name=name))
  elif test_type == 'runtime':
    tests.append(RuntimeTest(name=name,
                             output=metadata.get('output'),
                             should_fail=metadata.get('should_fail')))
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
