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
                    help='path to compiler executable (pawncc)')
parser.add_argument('-d', '--disassembler',
                    help='path to disassembler executable (pawndisasm)')
parser.add_argument('-i', '--include',
                    dest='include_dirs',
                    action='append',
                    help='add custom include directories for compile tests')
parser.add_argument('-r', '--runner',
                    help='path to runner executable (pawnruns)')
parser.add_argument('tests', metavar='test_name', nargs='*')
options = parser.parse_args(sys.argv[1:])

def run_command(args, executable=None, merge_stderr=False):
  process = subprocess.Popen(args,
                             executable=executable,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
  stdout, stderr = process.communicate()
  if merge_stderr:
    output = ''
    if stdout:
      output += stdout
    if stderr:
      output += stderr
    return (process, output)
  else:
    return (process, stdout, stderr)

def run_compiler(args):
  final_args = [';+', '-(+']
  if options.include_dirs is not None:
    for dir in options.include_dirs:
      final_args.append('-i' + dir)
  if args is not None:
    final_args += args
  return run_command(executable=options.compiler, args=final_args)

def normalize_newlines(s):
  return s.replace('\r', '')

def remove_asm_comments(s):
  return re.sub(r'\s*;.*\n', '\n', s)

def strip(s):
  return s.strip(' \t\r\n')

class OutputCheckTest:
  def __init__(self, name, errors=None, extra_args=None):
    self.name = name
    self.errors = errors
    self.extra_args = extra_args

  def run(self):
    args = [self.name + '.pwn']
    if self.extra_args is not None:
      args += self.extra_args
    process, stdout, stderr = run_compiler(args=args)
    if self.errors is None:
      if process.returncode != 0:
        result = False
        self.fail_reason = """
          No errors specified and process exited with non-zero status
        """
        return False

    errors = strip(stderr)
    expected_errors = strip(self.errors) if self.errors is not None else ''
    if errors != expected_errors:
      self.fail_reason = (
        'Error output didn\'t match\n\nExpected errors:\n\n{}\n\n'
        'Actual errors:\n\n{}'
      ).format(expected_errors, errors)
      return False
    return True

class PCodeCheckTest:
  def __init__(self,
               name,
               code_pattern=None,
               extra_args=None):
    self.name = name
    self.code_pattern = code_pattern
    self.extra_args = extra_args

  def run(self):
    args = ['-d0', self.name + '.pwn']
    if self.extra_args is not None:
      args += extra_args
    process, stdout, stderr = run_compiler(args=args)
    if process.returncode != 0:
      self.fail_reason = \
        'Compiler exited with status {}'.format(process.returncode)
      errors = stderr
      if errors:
        self.fail_reason += '\n\nErrors:\n\n{}'.format(errors)
      return False

    if options.disassembler is None:
      self.fail_reason = 'Disassembler path is not set, can\'t run this test'
      return False
    process, output = run_command([
      options.disassembler,
      self.name + '.amx'
    ], merge_stderr=True)
    if process.returncode != 0:
      self.fail_reason = \
        'Disassembler exited with status {}'.format(process.returncode)
      if output:
        self.fail_reason += '\n\nOutput:\n\n{}'.format(output)
      return False
    with open(self.name + '.lst', 'r') as dump_file:
      dump = dump_file.read()
      if self.code_pattern:
        dump = remove_asm_comments(dump)
        code_pattern = strip(normalize_newlines(self.code_pattern))
        if re.search(code_pattern, dump, re.MULTILINE) is None:
          self.fail_reason = (
            'Code didn\'t match\n\nExpected code:\n\n{}\n\n'
            'Actual code:\n\n{}'
          ).format(code_pattern, dump)
          return False
      else:
        self.fail_reason = 'Code pattern is required'
        return False
    return True

class RuntimeTest:
  def __init__(self, name, output, should_fail):
    self.name = name
    self.output = output
    self.should_fail = should_fail

  def run(self):
    process, stdout, stderr = run_compiler([self.name + '.pwn'])
    if process.returncode != 0:
      self.fail_reason = \
        'Compiler exited with status {}'.format(process.returncode)
      errors = stderr
      if errors:
        self.fail_reason += '\n\nErrors:\n\n{}'.format(errors)
      return False

    if options.runner is None:
      self.fail_reason = 'Runner path is not set, can\'t run this test'
      return False
    process, output = run_command([
      options.runner, self.name + '.amx'
    ], merge_stderr=True)
    if not self.should_fail and process.returncode != 0:
      self.fail_reason = (
        'Runner exited with status {}\n\nOutput: {}'
      ).format(process.returncode, output)
      return False

    output = strip(output)
    expected_output = strip(self.output)
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
  if options.tests and name not in options.tests:
    continue
  metadata = eval(open(meta_file).read(), None, None)
  if metadata.get('disabled'):
    num_tests_disabled += 1
    continue

  test_type = metadata['test_type']
  if test_type == 'output_check':
    tests.append(OutputCheckTest(
      name=name,
      errors=metadata.get('errors'),
      extra_args=metadata.get('extra_args')))
  elif test_type == 'pcode_check':
    tests.append(PCodeCheckTest(
      name=name,
      code_pattern=metadata.get('code_pattern'),
      extra_args=metadata.get('extra_args')))
  elif test_type == 'runtime':
    tests.append(RuntimeTest(
      name=name,
      output=metadata.get('output'),
      should_fail=metadata.get('should_fail')))
  else:
    raise KeyError('Unknown test type: ' + test_type)

num_tests = len(tests)
sys.stdout.write(
  'DISCOVERED {} TEST{}'.format(num_tests, '' if num_tests == 1 else 'S'))
if num_tests_disabled > 0:
  sys.stdout.write(' ({} DISABLED)'.format(num_tests_disabled))

if num_tests > 0:
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
