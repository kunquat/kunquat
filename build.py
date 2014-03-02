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
import glob
from optparse import Option
import os
import os.path
import stat
import sys

import fabricate
import options


quiet_builder = fabricate.Builder(quiet=True)


# Test external dependencies

def write_external_header_test(out_base, header_name):
    script_path = os.path.join('scripts', 'write_external_header_test.py')
    quiet_builder.run('python', script_path, out_base, header_name)


def build_external_lib_test(out_base, lib_name):
    in_path = out_base + '.c'
    out_path = out_base
    link_flag = '-l' + lib_name
    quiet_builder.run('gcc', '-o', out_path, in_path, link_flag)


def compile_external_header_test(out_base):
    in_path = out_base + '.c'
    out_path = out_base + '.o'
    quiet_builder.run('gcc', '-c', '-o', out_path, in_path)


def test_external_deps():
    out_dir = 'conf_tests'
    quiet_builder.run('mkdir', '-p', out_dir)

    compile_flags = []
    link_flags = []
    test_link_flags = []

    def test_lib_with_header(lib_name, header_name, flags=link_flags):
        print('Checking for {}... '.format(lib_name), end='')
        out_base = os.path.join(out_dir, lib_name)
        write_external_header_test(out_base, header_name)
        try:
            build_external_lib_test(out_base, lib_name)
        except fabricate.ExecutionError:
            return False
        print('ok')
        flags.append('-l' + lib_name)
        return True

    def test_header(header_name):
        print('Checking for header {}... '.format(header_name), end='')
        name_base = header_name[:header_name.rindex('.')]
        out_base = os.path.join(out_dir, name_base)
        write_external_header_test(out_base, header_name)
        try:
            compile_external_header_test(out_base)
        except fabricate.ExecutionError:
            return False
        print('ok')
        return True

    conf_errors = []

    # Test dependencies

    if options.enable_kunquat_assert and options.enable_debug:
        compile_flags.append('-DENABLE_KUNQUAT_ASSERT')
        if test_header('execinfo.h'):
            compile_flags.append('-DHAS_EXECINFO')
            link_flags.append('-rdynamic')

    if options.with_wavpack:
        if test_lib_with_header('wavpack', 'wavpack/wavpack.h'):
            compile_flags.append('-DWITH_WAVPACK')
        else:
            conf_errors.append(
                    'WavPack support was requested but WavPack was not found.')
    else:
        print('Warning: WavPack support is disabled!'
                ' Sample support will be very minimal.')

    '''
    if not test_lib_with_header('archive', 'archive.h'):
        conf_errors.append('libarchive was not found.')
    '''

    if options.enable_player:
        if not options.enable_python_bindings:
            conf_errors.append('kunquat-player was requested without Python bindings.')

    if options.enable_tracker:
        if not options.enable_python_bindings:
            conf_errors.append('kunquat-tracker was requested without Python bindings.')

    if options.enable_export:
        if not options.enable_python_bindings:
            conf_errors.append('kunquat-export was requested without Python bindings.')

    if options.enable_tests:
        if not test_lib_with_header('check', 'check.h', test_link_flags):
            conf_errors.append(
                    'Building of libkunquat tests was requested'
                    ' but Check was not found.')
        if not test_lib_with_header('pthread', 'pthread.h', test_link_flags):
            conf_errors.append('Building of libkunquat tests requires libpthread.')
        if not test_lib_with_header('rt', 'time.h', test_link_flags):
            conf_errors.append('Building of unit tests requires librt.')

    if options.enable_profiling:
        if not test_lib_with_header('m_p', 'math.h'):
            conf_errors.append(
                    'Profiling was requested but profiling math library was not found.')
    else:
        if not test_lib_with_header('m', 'math.h'):
            conf_errors.append('Math library was not found.')

    if conf_errors:
        print('\nCould not configure Kunquat due to the following error{}:\n'.format(
            's' if len(conf_errors) != 1 else ''))
        print('\n'.join(conf_errors) + '\n')
        sys.exit(1)

    flags = {
            'compile': compile_flags,
            'link': link_flags,
            'test_link': test_link_flags,
        }
    return flags


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
    link_flags.extend(['-shared', soname_flag])

    print('Linking libkunquat')
    quiet_builder.run(cc, '-o', lib_path, objs, link_flags)
    os.chmod(lib_path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH)


def build_libkunquat(cc, compile_flags, link_flags):
    build_dir = 'build_src'
    quiet_builder.run('mkdir', '-p', build_dir)
    out_dir = os.path.join(build_dir, 'lib')

    # TODO: clean up code so that subdirectories inside src/lib are not needed
    include_dirs = [
            os.path.join('src', 'lib'),
            os.path.join('src', 'lib', 'events'),
            os.path.join('src', 'lib', 'generators'),
            os.path.join('src', 'lib', 'dsps'),
            os.path.join('src', 'include')
        ]
    include_flags = ['-I' + d for d in include_dirs]

    src_dir = os.path.join('src', 'lib')
    compile_libkunquat_dir(cc, compile_flags + include_flags, out_dir, src_dir)
    link_libkunquat(cc, link_flags, out_dir)


def test_libkunquat(cc, compile_flags, link_flags):
    pass


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

    compile_flags.append('-fPIC')

    opt_flags = ['-O{:d}'.format(options.optimise)]
    compile_flags.extend(opt_flags)

    # Configure
    conf_flags = test_external_deps()
    compile_flags.extend(conf_flags['compile'])
    link_flags.extend(conf_flags['link'])
    test_link_flags.extend(conf_flags['test_link'])

    if options.enable_libkunquat:
        build_libkunquat(cc, compile_flags, link_flags)

    if options.enable_tests:
        test_libkunquat(cc, compile_flags, test_link_flags)

    if options.enable_examples:
        build_examples()


def clean():
    fabricate.autoclean()


prefix_option = Option('--prefix', type='string',
        help='installation directory prefix (default: {})'.format(options.prefix))

fabricate.main(extra_options=[prefix_option])


