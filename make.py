#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy
from optparse import Option, SUPPRESS_HELP
import ast
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


# Add definitions of options.py as command line switches
cmdline_opts = []
opt_vars = []

options_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'options.py')

with open(options_path) as f:
    data = f.read()
    raw_entries = [e.strip() for e in data.split('\n\n') if e.strip()]
    type_names = { str: 'string', int: 'int' }
    for raw_entry in raw_entries:
        lines = raw_entry.split('\n')
        desc_lines = lines[:-1]
        def_line = lines[-1]

        desc = '\n'.join(dl[1:].strip() for dl in desc_lines)
        var_name, _, value_str = (s.strip() for s in def_line.partition('='))
        name = '--' + var_name.replace('_', '-')
        value = ast.literal_eval(value_str)

        opt_vars.append(var_name)

        if type(value) == bool:
            first_word = var_name.split('_')[0]
            if first_word == 'enable':
                negated_name = name.replace('enable', 'disable', 1)
            elif first_word == 'with':
                negated_name = name.replace('with', 'without', 1)
            else:
                assert False

            if value == True:
                negated_desc = (desc.replace('enable', 'disable', 1)
                        if desc.startswith('enable') else ('do not ' + desc))
                full_desc = '{} (default: enabled)'.format(negated_desc)
                neg_opt = Option(
                        negated_name,
                        action='store_false',
                        dest=var_name,
                        help=full_desc)

                pos_opt = Option(
                        name, action='store_true', dest=var_name, help=SUPPRESS_HELP)
            else:
                full_desc = '{} (default: disabled)'.format(desc)
                pos_opt = Option(
                        name, action='store_true', dest=var_name, help=full_desc)
                neg_opt = Option(
                        negated_name,
                        action='store_false',
                        dest=var_name,
                        help=SUPPRESS_HELP)

            cmdline_opts.extend((neg_opt, pos_opt))

        elif value == None:
            if var_name == 'cc':
                desc = ('select C compiler'
                        ' (supported values: gcc (default), clang)')
                option = Option(name, type='choice', choices=['gcc', 'clang'], help=desc)
                cmdline_opts.append(option)
            else:
                assert False
        else:
            type_name = type_names[type(value)]
            full_desc = '{} (default: {})'.format(desc, value)
            option = Option(name, type=type_name, help=full_desc)
            cmdline_opts.append(option)

unsafe_option = Option('--unsafe', action='store_false',
        help='allow building with an unverified version of Fabricate (not recommended)')

cmdline_opts.append(unsafe_option)


def process_cmd_line():
    for var_name in opt_vars:
        override = fabricate.main.options.__dict__[var_name]
        if override != None:
            options.__dict__[var_name] = override

    # Make sure the installation prefix is absolute
    options.prefix = os.path.abspath(os.path.expanduser(options.prefix))


class PrettyBuilder(fabricate.Builder):

    def __init__(self, *args, **kwargs):
        fabricate.Builder.__init__(self, *args, **kwargs)

    def echo(self, message):
        '''Suppress printing of an empty string.'''
        if message:
            fabricate.Builder.echo(self, message)


def build():

    process_cmd_line()

    if options.enable_long_tests:
        python_modules = ['scripts', 'kunquat']
        fabricate.run('pylint', *python_modules)
        fabricate.run('flake8', *python_modules)

    cc = get_cc(options.cc)

    cc.set_debug(options.enable_debug)

    if options.enable_debug_asserts:
        cc.add_define('ENABLE_DEBUG_ASSERTS')

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
            fabricate.run('bash', ['-c','LD_LIBRARY_PATH=build/src/lib python3 -m unittest discover -v'])

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


fabricate.main(extra_options=cmdline_opts)


