# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
from collections import defaultdict, deque
import glob
import os.path
import subprocess

import command


def test_libkunquat(builder, options, cc):
    build_dir = os.path.join('build', 'src')
    test_dir = os.path.join(build_dir, 'test')
    command.make_dirs(builder, test_dir)

    src_dir = os.path.join('src', 'lib', 'test')

    # TODO: clean up code so that subdirectories inside src/lib are not needed
    include_dirs = [
            os.path.join('src', 'lib'),
            os.path.join('src', 'lib', 'test'),
            os.path.join('src', 'lib', 'events'),
            os.path.join('src', 'lib', 'generators'),
            os.path.join('src', 'lib', 'dsps'),
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

    # Define which tests are dependent on others
    deps = defaultdict(lambda: [], {
            'handle': ['streader', 'tstamp'],
            'player': ['handle', 'streader'],
            'memory': ['handle'],
            'connections': ['handle', 'player'],
            'generator': ['connections'],
            'instrument': ['connections'],
            'dsp': ['connections'],
            'validation': ['handle'],
        })
    finished_tests = set()

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
        print('Testing {}'.format(name))
        if cc.build_exe(builder, src_path, out_path):
            run_prefix = 'env LD_LIBRARY_PATH={} '.format(libkunquat_dir)
            if options.enable_tests_mem_debug:
                mem_debug_path = os.path.join(src_dir, 'mem_debug_run.py')
                run_prefix += mem_debug_path + ' '

            call = run_prefix + out_path
            try:
                subprocess.check_call(call.split())
            except subprocess.CalledProcessError as e:
                print('Test {} failed with return code {}'.format(name, e.returncode))
                os.remove(out_path)
                sys.exit(1)

        finished_tests.add(name)


