#!/usr/bin/env python
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
from optparse import Option
import os
import os.path
import subprocess
import stat
import sys

from scripts.configure import test_external_deps
import support.fabricate as fabricate
import options


quiet_builder = fabricate.Builder(quiet=True)


# Compile and link

def compile_libkunquat_dir(cc, compile_flags, out_dir, src_dir):
    quiet_builder.run('mkdir', '-p', out_dir)

    source_paths = glob.glob(os.path.join(src_dir, '*.c'))
    sources = sorted([os.path.basename(path) for path in source_paths])

    for source in sources:
        src_path = os.path.join(src_dir, source)
        obj_name = source[:source.rindex('.')] + '.o'
        out_path = os.path.join(out_dir, obj_name)
        print('Compiling {}'.format(src_path))
        quiet_builder.run(cc, '-c', src_path, '-o', out_path, compile_flags)

    # Recurse to subdirectories, excluding test directories
    subdir_names = sorted([name for name in os.listdir(src_dir)
            if os.path.isdir(os.path.join(src_dir, name)) and name != 'test'])
    for name in subdir_names:
        sub_out_dir = os.path.join(out_dir, name)
        sub_src_dir = os.path.join(src_dir, name)
        compile_libkunquat_dir(cc, compile_flags, sub_out_dir, sub_src_dir)


def link_libkunquat(cc, link_flags, build_lib_dir):
    objs = []
    for (dirpath, _, filenames) in os.walk(build_lib_dir):
        objs.extend(os.path.join(dirpath, name)
                for name in filenames if name.endswith('.o'))

    lib_name = 'libkunquat.so'
    lib_path = os.path.join(build_lib_dir, lib_name)

    version_major = 0
    soname_flag = '-Wl,-soname,{}.{}'.format(lib_name, version_major)
    libkunquat_link_flags = link_flags + ['-shared', soname_flag]

    print('Linking libkunquat')
    quiet_builder.run(cc, '-o', lib_path, objs, libkunquat_link_flags)
    os.chmod(lib_path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH)

    # Add symlink so that our tests will run
    symlink_path = lib_path + '.{}'.format(version_major)
    quiet_builder.run('ln', '-s', lib_name, symlink_path)


def build_libkunquat(cc, compile_flags, link_flags):
    build_dir = 'build_src'
    quiet_builder.run('mkdir', '-p', build_dir)
    out_dir = os.path.join(build_dir, 'lib')

    shared_flags = ['-fPIC']

    # TODO: clean up code so that subdirectories inside src/lib are not needed
    include_dirs = [
            os.path.join('src', 'lib'),
            os.path.join('src', 'lib', 'events'),
            os.path.join('src', 'lib', 'generators'),
            os.path.join('src', 'lib', 'dsps'),
            os.path.join('src', 'include')
        ]
    include_flags = ['-I' + d for d in include_dirs]

    libkunquat_compile_flags = compile_flags + shared_flags + include_flags

    src_dir = os.path.join('src', 'lib')
    compile_libkunquat_dir(cc, libkunquat_compile_flags, out_dir, src_dir)
    link_libkunquat(cc, link_flags, out_dir)


# libkunquat tests

def test_libkunquat(cc, compile_flags, link_flags):
    build_dir = 'build_src'
    test_dir = os.path.join(build_dir, 'test')
    quiet_builder.run('mkdir', '-p', test_dir)

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
    include_flags = ['-I' + d for d in include_dirs]

    test_compile_flags = compile_flags + include_flags
    libkunquat_dir = os.path.join(build_dir, 'lib')
    test_link_flags = ['-L{}'.format(libkunquat_dir), '-lkunquat'] + link_flags

    if options.enable_tests_mem_debug:
        test_compile_flags += ['-DK_MEM_DEBUG']

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
        (_, _, outputs_list) = quiet_builder.run(
                cc, '-o', out_path, src_path, test_compile_flags, test_link_flags)

        if outputs_list:
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


def build_examples():
    pass


def process_cmd_line():
    if fabricate.main.options.prefix != None:
        options.prefix = fabricate.main.options.prefix


def build():
    process_cmd_line()

    cc = 'gcc'
    compile_flags = [
            '-std=c99',
            '-pedantic',
            '-Wall',
            '-Wextra',
            '-Werror',
        ]
    link_flags = []
    test_link_flags = []

    if options.enable_debug:
        compile_flags.append('-g')
    else:
        compile_flags.append('-DNDEBUG')

    if options.enable_profiling:
        compile_flags.append('-pg')
        link_flags.append('-pg')

    opt_flags = ['-O{:d}'.format(options.optimise)]
    compile_flags.extend(opt_flags)

    # Configure
    conf_flags = test_external_deps(quiet_builder, options)
    compile_flags.extend(conf_flags['compile'])
    link_flags.extend(conf_flags['link'])
    test_link_flags.extend(conf_flags['test_link'])

    if options.enable_libkunquat:
        build_libkunquat(cc, compile_flags, link_flags)

    if options.enable_tests:
        test_libkunquat(cc, compile_flags, test_link_flags + link_flags)

    if options.enable_examples:
        build_examples()


def clean():
    fabricate.autoclean()


prefix_option = Option('--prefix', type='string',
        help='installation directory prefix (default: {})'.format(options.prefix))

fabricate.main(extra_options=[prefix_option])


