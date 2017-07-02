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

import os.path
import subprocess
import sys


class PythonCommand():

    def __init__(self):
        self._cmd = self._get_python_cmd()

    def run(self, builder, *args, **kwargs):
        all_args = [self._cmd] + list(args)
        return run_command(builder, *all_args, **kwargs)

    def _get_python_cmd(self):
        test_cmds = ['python3']
        for cmd in test_cmds:
            try:
                output = subprocess.check_output(
                        [cmd, '--version'], stderr=subprocess.STDOUT)
            except (OSError, subprocess.CalledProcessError):
                output = b''
            if output.startswith(b'Python 3'):
                return cmd
        else:
            raise RuntimeError('Python 3 not found')


def copy(builder, src, dest, echo=None):
    dir_name = os.path.dirname(dest)
    make_dirs(builder, dir_name, echo='')
    return run_command(builder, 'cp', '-P', src, dest, echo=echo)


def link(builder, file_name, link_name, echo=None):
    dir_name = os.path.dirname(link_name)
    make_dirs(builder, dir_name, echo='')
    return run_command(builder, 'ln', '-f', '-s', file_name, link_name, echo=echo)


def make_dirs(builder, path, echo=None):
    path_components = _split_all(path)
    # Make sure that the builder keeps track of all created directories
    prefix = ''
    for component in path_components:
        prefix = os.path.join(prefix, component)
        if not os.path.exists(prefix):
            run_command(builder, 'mkdir', '-p', prefix, echo=echo)


def run_command(builder, *args, **kwargs):
    echo = kwargs.get('echo')
    if builder:
        (_, _, outputs_list) = builder.run(*args, echo=echo)
        return bool(outputs_list)
    else:
        subprocess.check_call(args)
        return True


def _split_all(path):
    if not path:
        return []

    head, tail = os.path.split(path)
    if head == path:
        return [head]

    parts = _split_all(head)
    parts.append(tail)
    return parts


