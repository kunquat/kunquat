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
from scripts.build_libkunquat import build_libkunquat
from scripts.test_libkunquat import test_libkunquat
import support.fabricate as fabricate
import options


quiet_builder = fabricate.Builder(quiet=True)


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
        build_libkunquat(quiet_builder, options, cc, compile_flags, link_flags)

    if options.enable_tests:
        test_libkunquat(quiet_builder, options, cc, compile_flags, test_link_flags + link_flags)

    if options.enable_examples:
        build_examples()


def clean():
    fabricate.autoclean()


prefix_option = Option('--prefix', type='string',
        help='installation directory prefix (default: {})'.format(options.prefix))

fabricate.main(extra_options=[prefix_option])


