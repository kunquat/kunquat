# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import defaultdict, deque
import glob
import os.path
import subprocess
import sys

from . import command


def test_libkunquat(builder, options, cc):
    build_dir = os.path.join('build', 'src')
    test_dir = os.path.join(build_dir, 'test')

    src_dir = os.path.join('src', 'test')

    include_dirs = [
            os.path.join('src', 'lib'),
            os.path.join('src', 'include'),
            src_dir
        ]
    for d in include_dirs:
        cc.add_include_dir(d)

    libkunquat_dir = os.path.join(build_dir, 'lib')
    cc.add_lib_dir(libkunquat_dir)
    cc.add_lib('kunquat')

    if options.enable_tests_mem_debug:
        cc.add_define('K_MEM_DEBUG')

    # Define which tests depend on others
    deps = defaultdict(lambda: [], {
            'handle': ['streader', 'tstamp'],
            'player': ['handle', 'streader', 'fast_sin', 'fast_exp2', 'fast_log2'],
            'memory': ['handle'],
            'connections': ['handle', 'player'],
            'generator': ['connections'],
            'instrument': ['connections'],
            'dsp': ['connections', 'fast_sin'],
            'validation': ['handle'],
        })
    finished_tests = set()

    # Specify tests that should always run without memory debugging (for performance)
    force_disable_mem_tests = set(['fast_sin', 'fast_exp2', 'fast_log2'])

    echo = '\n   Testing libkunquat\n'

    source_paths = deque(glob.glob(os.path.join(src_dir, '*.c')))
    max_iters = len(source_paths) * len(source_paths)
    while source_paths:
        # Avoid infinite loop
        max_iters -= 1
        if max_iters < 0:
            raise RuntimeError(
                    'Tests have invalid dependencies,'
                    ' stuck with: {}'.format(source_paths))

        src_path = source_paths.popleft()
        base = os.path.basename(src_path)
        name = base[:base.rindex('.')]

        # Make sure that tests we depend on have succeeded
        ok_to_test = True
        for prereq in deps[name]:
            if prereq not in finished_tests:
                ok_to_test = False
                break
        if not ok_to_test:
            source_paths.append(src_path)
            continue

        # Build and run
        out_path = os.path.join(test_dir, name)
        if cc.build_exe(builder, src_path, out_path, echo=echo):
            echo = ''
            run_prefix = 'env LD_LIBRARY_PATH={} '.format(libkunquat_dir)
            if options.enable_tests_mem_debug and name not in force_disable_mem_tests:
                mem_debug_path = os.path.join(src_dir, 'mem_debug_run.py')
                run_prefix += mem_debug_path + ' '

            call = run_prefix + out_path
            try:
                subprocess.check_call(call.split())
            except KeyboardInterrupt:
                os.remove(out_path)
                raise
            except subprocess.CalledProcessError as e:
                print('Test {} failed with return code {}'.format(name, e.returncode),
                        file=sys.stderr)
                os.remove(out_path)
                sys.exit(1)

        finished_tests.add(name)


