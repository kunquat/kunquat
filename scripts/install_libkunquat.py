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

import glob
from itertools import dropwhile, islice, takewhile
import os.path

from . import command


def _get_man_funcs(man_file_path):
    with open(man_file_path) as f:
        all_lines = (line for line in f)
        tail_lines = dropwhile(lambda line: 'SYNOPSIS' not in line, all_lines)
        synopsis_lines = list(takewhile(
            lambda line: not line.startswith('.SH'), islice(tail_lines, 1, None)))

    func_lines = (line for line in synopsis_lines if '(' in line)
    func_line_prefixes = (line[:line.index('(')] for line in func_lines)
    func_names = (prefix.split()[-1] for prefix in func_line_prefixes)
    return func_names


def install_libkunquat(builder, install_prefix, enable_dev):
    def _install_headers():
        install_include_dir = os.path.join(install_prefix, 'include', 'kunquat')

        source_dir = os.path.join('src', 'include', 'kunquat')
        header_paths = glob.glob(os.path.join(source_dir, '*.h'))
        header_names = sorted([os.path.basename(path) for path in header_paths])
        for name in header_names:
            in_path = os.path.join(source_dir, name)
            out_path = os.path.join(install_include_dir, name)
            command.copy(builder, in_path, out_path)

    def _install_man_pages():
        install_man_dir = os.path.join(install_prefix, 'share', 'man', 'man3')

        source_dir = os.path.join('src', 'include', 'kunquat')
        man_paths = glob.glob(os.path.join(source_dir, 'kunquat-*.3'))
        man_names = sorted([os.path.basename(path) for path in man_paths])
        for name in man_names:
            in_path = os.path.join(source_dir, name)
            out_path = os.path.join(install_man_dir, name)
            command.copy(builder, in_path, out_path)

        for name in man_names:
            man_path = os.path.join(source_dir, name)
            links = (func_name + '.3' for func_name in _get_man_funcs(man_path))
            for link in links:
                link_path = os.path.join(install_man_dir, link)
                command.link(builder, name, link_path)

    def _install_library():
        install_lib_dir = os.path.join(install_prefix, 'lib')

        build_dir = os.path.join('build', 'src', 'lib')
        lib_names = ['libkunquat.so', 'libkunquat.so.0']
        for name in lib_names:
            in_path = os.path.join(build_dir, name)
            out_path = os.path.join(install_lib_dir, name)
            command.copy(builder, in_path, out_path)

    if enable_dev:
        _install_headers()
        _install_man_pages()

    _install_library()


