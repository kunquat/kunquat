#!/usr/bin/env python3
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

from copy import deepcopy
from optparse import Option
import os
import os.path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True

try:
    import support.fabricate as fabricate
except ImportError:
    msg = 'Fabricate was not found. Please run ./get_build_support.sh to retrieve it.'

    # Check for unsafe build option
    check_args = sys.argv
    if '--' in check_args:
        check_args = check_args[:check_args.index('--')]
    if '--unsafe' in check_args:
        try:
            import support.fabricate_unverified as fabricate
        except ImportError:
            print(msg, file=sys.stderr)
            sys.exit(1)
    else:
        print(msg, file=sys.stderr)
        sys.exit(1)

import scripts.command as command
from scripts.cc import get_cc
from scripts.configure import test_add_external_deps, test_add_test_deps
from scripts.build_libkunquat import build_libkunquat
from scripts.test_libkunquat import test_libkunquat
from scripts.build_examples import build_examples
from scripts.install_libkunquat import install_libkunquat
from scripts.install_examples import install_examples
from scripts.install_share import install_share
import options


def process_cmd_line():
    if fabricate.main.options.prefix != None:
        options.prefix = os.path.abspath(os.path.expanduser(
            fabricate.main.options.prefix))


class PrettyBuilder(fabricate.Builder):

    def __init__(self, *args, **kwargs):
        fabricate.Builder.__init__(self, *args, **kwargs)

    def echo(self, message):
        '''Suppress printing of an empty string.'''
        if message:
            fabricate.Builder.echo(self, message)


def build():
    process_cmd_line()

    cc = get_cc()

    cc.set_debug(options.enable_debug)

    #if options.enable_profiling:
    #    compile_flags.append('-pg')
    #    link_flags.append('-pg')

    if options.optimise not in range(5):
        print('Unsupported optimisation level: {}'.format(options.optimise),
                file=sys.stderr)
        sys.exit(1)
    cc.set_optimisation(options.optimise)

    builder = PrettyBuilder()

    if options.enable_python_bindings:
        try:
            python_cmd = command.PythonCommand()
        except RuntimeError:
            print('Python bindings were requested but Python 2.7 was not found.',
                    file=sys.stderr)
            sys.exit(1)

    if options.enable_tests_mem_debug:
        try:
            output = subprocess.check_output(
                    ['valgrind', '--version'], stderr=subprocess.STDOUT)
        except (OSError, subprocess.CalledProcessError):
            output = b''
        if not output.startswith(b'valgrind'):
            print('Memory debugging of libkunquat tests was requested'
                    ' but Valgrind was not found.',
                    file=sys.stderr)
            sys.exit(1)

    test_add_external_deps(builder, options, cc)

    test_cc = deepcopy(cc)
    test_add_test_deps(builder, options, test_cc)

    if options.enable_libkunquat:
        build_libkunquat(builder, options, cc)
        if options.enable_tests:
            test_libkunquat(builder, options, test_cc)

    if options.enable_examples:
        build_examples(builder)


def clean():
    if os.path.exists('build'):
        # Remove Python-specific build directories first
        for name in os.listdir('build'):
            expected_suffix = '-{}.{}'.format(sys.version_info[0], sys.version_info[1])
            if name.endswith(expected_suffix) or name == 'lib':
                path = os.path.join('build', name)
                shutil.rmtree(path)

    fabricate.autoclean()


def install():
    build()

    install_builder = None

    if options.enable_libkunquat:
        install_libkunquat(
                install_builder, options.prefix, options.enable_libkunquat_dev)

    if options.enable_examples:
        install_examples(install_builder, options.prefix)

    install_share(install_builder, options.prefix)

    if options.enable_python_bindings:
        python_cmd = command.PythonCommand()
        args = ['py-setup.py', 'install', '--prefix={}'.format(options.prefix)]
        if not options.enable_export:
            args.append('--disable-export')
        if not options.enable_player:
            args.append('--disable-player')
        if not options.enable_tracker:
            args.append('--disable-tracker')
        try:
            python_cmd.run(install_builder, *args)
        except subprocess.CalledProcessError:
            sys.exit(1)


prefix_option = Option('--prefix', type='string',
        help='installation directory prefix (default: {})'.format(options.prefix))

unsafe_option = Option('--unsafe', action='store_false',
        help='allow building with an unverified version of Fabricate (not recommended)')

fabricate.main(extra_options=[prefix_option, unsafe_option])


