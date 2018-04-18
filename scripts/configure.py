# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy
import os.path
import shlex
import subprocess
import sys

try:
    import support.fabricate as fabricate
except ImportError:
    # If we have got this far without verified fabricate, we must be in unsafe mode
    assert '--unsafe' in sys.argv
    import support.fabricate_unverified as fabricate

from . import command


def _check_conf_errors(conf_errors):
    if conf_errors:
        print('\nCould not configure Kunquat due to the following error{}:\n'.format(
            's' if len(conf_errors) != 1 else ''), file=sys.stderr)
        print('\n'.join(conf_errors) + '\n', file=sys.stderr)
        sys.exit(1)


def test_add_common_external_deps(builder, options, cc):
    conf_errors = []

    if options.enable_debug:
        if _test_header(builder, cc, 'execinfo.h'):
            cc.add_define('HAS_EXECINFO')
            cc.set_dynamic_export(True)

    if not _test_add_lib_with_header(builder, cc, 'm', 'math.h'):
        conf_errors.append('Math library was not found.')

    _check_conf_errors(conf_errors)


def test_add_libkunquat_external_deps(builder, options, cc):
    conf_errors = []

    if options.enable_threads:
        if options.with_pthread:
            if _test_header(builder, cc, 'pthread.h'):
                cc.add_compile_flag('-pthread')
                cc.add_define('_XOPEN_SOURCE', 700)
                cc.add_define('WITH_PTHREAD')
            else:
                conf_errors.append(
                        'POSIX threads support was requested but Pthreads was not found.')
        else:
            conf_errors.append(
                    'Multithreading support was requested without'
                    ' threading implementation specified.')
        cc.add_define('ENABLE_THREADS')

    if options.with_sndfile:
        if _test_add_lib_with_header(builder, cc, 'sndfile', 'sndfile.h'):
            cc.add_define('WITH_SNDFILE')
        else:
            conf_errors.append(
                    'libsndfile support was requested but libsndfile was not found.')
    else:
        print('Warning: libsndfile support is disabled!'
                ' Additive synthesis support will be very minimal.', file=sys.stderr)

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
        if not options.with_sndfile:
            conf_errors.append('kunquat-export was requested without libsndfile.')

    _check_conf_errors(conf_errors)


def test_add_test_deps(builder, options, cc):
    conf_errors = []

    if options.enable_tests:
        if not _test_add_lib_with_pkgconfig(builder, cc, 'check', 'check.h'):
            conf_errors.append(
                    'Building of libkunquat tests was requested'
                    ' but not all Check dependencies were found.')

    _check_conf_errors(conf_errors)


def test_add_libkunquatfile_external_deps(builder, options, cc):
    conf_errors = []

    if options.with_zip:
        if not _test_add_lib_with_header(builder, cc, 'zip', 'zip.h'):
            conf_errors.append('libzip was not found.')

    if options.enable_libkunquatfile:
        if not options.with_zip:
            conf_errors.append('libkunquatfile was requested without libzip.')

    _check_conf_errors(conf_errors)


def _write_external_header_test(builder, out_base, header_name):
    script_path = os.path.join('scripts', 'write_external_header_test.py')
    command.PythonCommand().run(builder, script_path, out_base, header_name, echo='')


def _get_fresh_cc(from_cc):
    cons = from_cc.__class__
    return cons()


def _build_external_lib_test(builder, cc, out_base, lib_name):
    in_path = out_base + '.c'
    out_path = out_base
    temp_cc = _get_fresh_cc(cc)
    temp_cc.add_lib(lib_name)
    return temp_cc.build_exe(builder, in_path, out_path, echo='')


def _compile_external_header_test(builder, cc, out_base):
    in_path = out_base + '.c'
    out_path = out_base + '.o'
    temp_cc = _get_fresh_cc(cc)
    return temp_cc.compile(builder, in_path, out_path, echo='')


def _test_add_lib_with_header(builder, cc, lib_name, header_name):
    out_dir = 'conf_tests'
    command.make_dirs(builder, out_dir, echo='')

    print('Checking for {}... '.format(lib_name), end='')
    out_base = os.path.join(out_dir, lib_name)
    _write_external_header_test(builder, out_base, header_name)
    try:
        was_run = _build_external_lib_test(builder, cc, out_base, lib_name)
    except fabricate.ExecutionError:
        return False
    print('{}ok'.format('' if was_run else '(cached) '))
    cc.add_lib(lib_name)
    return True


def _test_add_lib_with_pkgconfig(builder, cc, lib_name, header_name):
    out_dir = 'conf_tests'
    command.make_dirs(builder, out_dir, echo='')

    print('Checking for {}... '.format(lib_name), end='')

    # Get flags
    cflags_cmd = 'pkg-config --cflags {}'.format(lib_name)
    libs_cmd = 'pkg-config --libs {}'.format(lib_name)
    try:
        cflags_b = subprocess.check_output(shlex.split(cflags_cmd))
        libs_b = subprocess.check_output(shlex.split(libs_cmd))
    except subprocess.CalledProcessError as e:
        return False
    cflags = shlex.split(str(cflags_b, encoding='utf-8'))
    libs = shlex.split(str(libs_b, encoding='utf-8'))

    # Build the test
    out_base = os.path.join(out_dir, lib_name)
    _write_external_header_test(builder, out_base, header_name)
    try:
        was_run = _build_external_lib_test(builder, cc, out_base, lib_name)
    except fabricate.ExecutionError:
        return False

    # Add flags
    print('{}ok'.format('' if was_run else '(cached) '))
    for flag in cflags:
        cc.add_compile_flag(flag)
    for flag in libs:
        cc.add_link_flag(flag)
    return True


def _test_header(builder, cc, header_name):
    out_dir = 'conf_tests'
    command.make_dirs(builder, out_dir, echo='')

    print('Checking for header {}... '.format(header_name), end='')
    name_base = header_name[:header_name.rindex('.')]
    out_base = os.path.join(out_dir, name_base)
    _write_external_header_test(builder, out_base, header_name)
    try:
        was_run = _compile_external_header_test(builder, cc, out_base)
    except fabricate.ExecutionError:
        return False
    print('{}ok'.format('' if was_run else '(cached) '))
    return True


