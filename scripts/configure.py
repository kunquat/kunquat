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
import os.path

import command


def test_external_deps(builder, options):
    out_dir = 'conf_tests'
    command.make_dirs(builder, out_dir)

    compile_flags = []
    link_flags = []
    test_link_flags = []

    def write_external_header_test(out_base, header_name):
        script_path = os.path.join('scripts', 'write_external_header_test.py')
        builder.run('python', script_path, out_base, header_name)

    def build_external_lib_test(out_base, lib_name):
        in_path = out_base + '.c'
        out_path = out_base
        link_flag = '-l' + lib_name
        builder.run('gcc', '-o', out_path, in_path, link_flag)

    def compile_external_header_test(out_base):
        in_path = out_base + '.c'
        out_path = out_base + '.o'
        builder.run('gcc', '-c', '-o', out_path, in_path)

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


