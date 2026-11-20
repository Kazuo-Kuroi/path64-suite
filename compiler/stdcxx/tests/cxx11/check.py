#!/usr/bin/env python3
"""Run the original regressions and optional approved reference checkpoint."""
import argparse
import os
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stage', required=True)
    parser.add_argument('--reference', default=os.environ.get('LIBCXX_SRC', ''))
    parser.add_argument('--manifest', type=Path, default=HERE / 'checkpoint.txt')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('-j', type=int, default=4)
    parser.add_argument('--host', action='store_true')
    args = parser.parse_args()
    paths = [str(HERE)]
    if args.reference:
        if not (Path(args.reference) / 'test' / 'std').is_dir():
            parser.error('--reference must name the approved libc++ 8.0.1 source tree')
        try:
            entries = [line.split('#', 1)[0].strip()
                       for line in args.manifest.read_text().splitlines()]
        except OSError as error:
            parser.error(str(error))
        paths.extend(entry for entry in entries if entry)
    args.output.mkdir(parents=True, exist_ok=True)
    failed = False
    for standard in ('c++11', 'c++14'):
        for opt in ('O0', 'O2'):
            command = [sys.executable, str(HERE / 'lcxx.py'), '--strict', '-v',
                       '--stage', args.stage, '--libcxx', args.reference,
                       '-std', standard, '-O', opt, '-j', str(args.j),
                       '-o', str(args.output / ('%s-%s.json' % (standard, opt)))]
            if args.host:
                command.append('--host')
            result = subprocess.run(command + paths)
            failed |= result.returncode != 0
    return int(failed)


if __name__ == '__main__':
    raise SystemExit(main())
