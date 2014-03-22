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
from copy import deepcopy
import os.path
import sys

import support.fabricate as fabricate

import command


def test_add_external_deps(builder, options, cc):
    conf_errors = []

    if options.enable_kunquat_assert and options.enable_debug:
        cc.add_define('ENABLE_KUNQUAT_ASSERT')
        if _test_header(builder, cc, 'execinfo.h'):
            cc.add_define('HAS_EXECINFO')
            cc.set_dynamic_export(True)

    if options.with_wavpack:
        if _test_add_lib_with_header(builder, cc, 'wavpack', 'wavpack/wavpack.h'):
            cc.add_define('WITH_WAVPACK')
        else:
            conf_errors.append(
                    'WavPack support was requested but WavPack was not found.')
    else:
        print('Warning: WavPack support is disabled!'
                ' Sample support will be very minimal.', file=sys.stderr)

    '''
    if not _test_add_lib_with_header(builder, cc, 'archive', 'archive.h'):
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

    if options.enable_profiling:
        if not _test_add_lib_with_header(builder, cc, 'm_p', 'math.h'):
            conf_errors.append(
                    'Profiling was requested but profiling math library was not found.')
    else:
        if not _test_add_lib_with_header(builder, cc, 'm', 'math.h'):
            conf_errors.append('Math library was not found.')

    if conf_errors:
        print('\nCould not configure Kunquat due to the following error{}:\n'.format(
            's' if len(conf_errors) != 1 else ''), file=sys.stderr)
        print('\n'.join(conf_errors) + '\n', file=sys.stderr)
        sys.exit(1)


def test_add_test_deps(builder, options, cc):
    conf_errors = []

    if options.enable_tests:
        if _test_add_lib_with_header(builder, cc, 'check', 'check.h'):
            cc.add_lib('m')
        else:
            conf_errors.append(
                    'Building of libkunquat tests was requested'
                    ' but Check was not found.')

        if not _test_add_lib_with_header(builder, cc, 'pthread', 'pthread.h'):
            conf_errors.append('Building of unit tests requires libpthread.')

        if not _test_add_lib_with_header(builder, cc, 'rt', 'time.h'):
            conf_errors.append('Building of unit tests requires librt.')

    if conf_errors:
        print('\nCould not configure Kunquat due to the following error{}:\n'.format(
            's' if len(conf_errors) != 1 else ''), file=sys.stderr)
        print('\n'.join(conf_errors) + '\n', file=sys.stderr)
        sys.exit(1)


def _write_external_header_test(builder, out_base, header_name):
    script_path = os.path.join('scripts', 'write_external_header_test.py')
    builder.run('python', script_path, out_base, header_name)


def _get_fresh_cc(from_cc):
    cons = from_cc.__class__
    return cons()


def _build_external_lib_test(builder, cc, out_base, lib_name):
    in_path = out_base + '.c'
    out_path = out_base
    temp_cc = _get_fresh_cc(cc)
    temp_cc.add_lib(lib_name)
    temp_cc.build_exe(builder, in_path, out_path)


def _compile_external_header_test(builder, cc, out_base):
    in_path = out_base + '.c'
    out_path = out_base + '.o'
    temp_cc = _get_fresh_cc(cc)
    temp_cc.compile(builder, in_path, out_path)


def _test_add_lib_with_header(builder, cc, lib_name, header_name):
    out_dir = 'conf_tests'
    command.make_dirs(builder, out_dir)

    print('Checking for {}... '.format(lib_name), end='')
    out_base = os.path.join(out_dir, lib_name)
    _write_external_header_test(builder, out_base, header_name)
    try:
        _build_external_lib_test(builder, cc, out_base, lib_name)
    except fabricate.ExecutionError:
        return False
    print('ok')
    cc.add_lib(lib_name)
    return True


def _test_header(builder, cc, header_name):
    out_dir = 'conf_tests'
    command.make_dirs(builder, out_dir)

    print('Checking for header {}... '.format(header_name), end='')
    name_base = header_name[:header_name.rindex('.')]
    out_base = os.path.join(out_dir, name_base)
    _write_external_header_test(builder, out_base, header_name)
    try:
        _compile_external_header_test(builder, cc, out_base)
    except fabricate.ExecutionError:
        return False
    print('ok')
    return True


