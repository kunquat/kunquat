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
from optparse import Option
import os
import os.path
import shutil
import subprocess
import sys

import support.fabricate as fabricate

import scripts.command as command
from scripts.configure import test_external_deps
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

    quiet_builder = fabricate.Builder(quiet=True)

    if options.enable_python_bindings:
        try:
            python_cmd = command.PythonCommand()
        except RuntimeError:
            print('Python bindings were requested but Python 2.7 was not found.',
                    file=sys.stderr)
            sys.exit(1)

    # Configure
    conf_flags = test_external_deps(quiet_builder, options)
    compile_flags.extend(conf_flags['compile'])
    link_flags.extend(conf_flags['link'])
    test_link_flags.extend(conf_flags['test_link'])

    if options.enable_libkunquat:
        build_libkunquat(quiet_builder, options, cc, compile_flags, link_flags)
        if options.enable_tests:
            test_libkunquat(quiet_builder, options, cc, compile_flags, test_link_flags + link_flags)

    if options.enable_examples:
        build_examples(quiet_builder)


def clean():
    for name in os.listdir('build'):
        if name.endswith('-2.7'):
            path = os.path.join('build', name)
            shutil.rmtree(path)

    fabricate.autoclean()


def _get_install_builder():
    return fabricate.Builder(depsname='.install_deps')


def install():
    build()

    install_builder = _get_install_builder()

    if options.enable_libkunquat:
        install_libkunquat(install_builder, options.prefix)

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
            python_cmd.run(None, *args)
        except subprocess.CalledProcessError:
            sys.exit(1)


prefix_option = Option('--prefix', type='string',
        help='installation directory prefix (default: {})'.format(options.prefix))

fabricate.main(extra_options=[prefix_option])


