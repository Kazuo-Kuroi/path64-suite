#!/usr/bin/env python3
"""Exercise the runner's CI failure reporting without a compiler installation."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

RUNNER = Path(__file__).resolve().with_name('lcxx.py')
COMPILER = '''#!/usr/bin/env python3
import pathlib, sys, time
source = pathlib.Path(next(arg for arg in sys.argv if arg.endswith('.pass.cpp')))
if source.stem.startswith('requires_flag') and '-fno-exceptions' not in sys.argv:
    print('error: missing requested compiler flag')
    sys.exit(1)
if source.stem.startswith('compile_timeout'):
    time.sleep(5)
if source.stem.startswith('compile_fail'):
    print('error: deliberate compile failure')
    sys.exit(1)
output = pathlib.Path(sys.argv[sys.argv.index('-o') + 1])
output.write_text('#!/usr/bin/env python3\\n' + (
    'import time; time.sleep(5)\\n' if source.stem.startswith('run_timeout') else
    'raise SystemExit(%d)\\n' % (7 if source.stem.startswith('run_fail') else 0)))
output.chmod(0o755)
'''


class RunnerChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='lcxx-runner-check-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.stage = self.root / 'stage'
        (self.stage / 'bin').mkdir(parents=True)
        compiler = self.stage / 'bin' / 'pathCC'
        compiler.write_text(COMPILER)
        compiler.chmod(0o755)
        self.reference = self.root / 'reference'
        self.tests = self.reference / 'test' / 'std'
        self.tests.mkdir(parents=True)
        (self.reference / 'test' / 'support').mkdir()
        self.report = self.root / 'results.json'

    def run_checks(self, *arguments):
        environment = dict(os.environ, PATH64_STAGE='/invalid-stage')
        return subprocess.run([sys.executable, str(RUNNER), '--stage', str(self.stage),
                               '--libcxx', str(self.reference), '-j', '2',
                               '-o', str(self.report)] + list(arguments),
                              env=environment, capture_output=True, text=True, timeout=20)

    def add_source(self, name, text=''):
        (self.tests / (name + '.pass.cpp')).write_text(text)

    def test_strict_failures_and_expected_exclusions(self):
        for name in ('pass', 'compile_fail', 'run_fail'):
            self.add_source(name)
        self.add_source('skipped', '// UNSUPPORTED: c++11\n')
        self.add_source('compile_fail_expected', '// XFAIL: clang-4\n')
        result = self.run_checks('--strict', '.', 'pass.pass.cpp')
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        statuses = {name: item[0] for name, item in json.loads(self.report.read_text()).items()}
        self.assertEqual(statuses, {'pass.pass.cpp': 'PASS', 'compile_fail.pass.cpp': 'COMPILE',
                                   'run_fail.pass.cpp': 'RUN', 'skipped.pass.cpp': 'UNSUPPORTED',
                                   'compile_fail_expected.pass.cpp': 'XFAIL'})
        result = self.run_checks('--strict', 'pass.pass.cpp', 'skipped.pass.cpp',
                                 'compile_fail_expected.pass.cpp')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.run_checks('.').returncode, 0)

    def test_source_specific_flags(self):
        self.add_source('requires_flag', '// PATH64_FLAGS: -fno-exceptions\n')
        result = self.run_checks('--strict', 'requires_flag.pass.cpp')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.add_source('requires_flag')
        self.assertEqual(self.run_checks('--strict', 'requires_flag.pass.cpp').returncode, 1)

    def test_timeouts_and_missing_selection(self):
        self.add_source('compile_timeout')
        self.add_source('run_timeout')
        result = self.run_checks('--strict', '--compile-timeout', '.25', '--timeout', '.25', '.')
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertEqual([item[0] for item in json.loads(self.report.read_text()).values()],
                         ['TIMEOUT', 'TIMEOUT'])
        self.assertEqual(self.run_checks('missing').returncode, 2)
        empty = self.tests / 'empty'
        empty.mkdir()
        self.assertEqual(self.run_checks(str(empty)).returncode, 2)

    def test_reference_manifest_and_matrix_status(self):
        self.add_source('compile_fail')
        manifest = self.root / 'checkpoint.txt'
        manifest.write_text('# A deliberate checkpoint failure\ncompile_fail.pass.cpp\n')
        reports = self.root / 'matrix'
        result = subprocess.run([sys.executable, str(RUNNER.with_name('check.py')),
                                 '--stage', str(self.stage), '--reference', str(self.reference),
                                 '--manifest', str(manifest), '--output', str(reports), '-j', '4'],
                                capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertEqual(len(list(reports.glob('*.json'))), 4)
        for report in reports.glob('*.json'):
            self.assertEqual(json.loads(report.read_text())['compile_fail.pass.cpp'][0], 'COMPILE')


if __name__ == '__main__':
    unittest.main()
