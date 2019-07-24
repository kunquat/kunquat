# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.extras import processors
from kunquat.kunquat.limits import *

import argparse
import math
import multiprocessing
import os.path


def parse_arguments():
    ap = argparse.ArgumentParser(add_help=False)
    ap.add_argument('-h', '--help', action='help',
            help='Show this help and exit')
    ap.add_argument('--experimental', action='store_true',
            help='Enable experimental features')
    ap.add_argument('--install-prefix', default=_find_install_prefix(),
            help=argparse.SUPPRESS)
    ap.add_argument('-r', '--rate', type=int, default=48000, metavar='r',
            help='Set audio rate to %(metavar)s frames/second (default: %(default)s)')
    ap.add_argument('--audio-latency', type=float, default=60, metavar='t',
            help='Set audio latency to %(metavar)s milliseconds (default: %(default)s)')
    ap.add_argument(
            '--threads', type=int, default=get_default_thread_count(), metavar='n',
            help='Use %(metavar)s threads for audio rendering (default: %(default)s)')
    ap.add_argument('kqtfile', type=str, nargs='?', default='',
            help='Use kqtfile as input')

    global _args
    _args = ap.parse_args()

def get_experimental():
    return _args.experimental

def get_kqt_file():
    return _args.kqtfile

def get_install_prefix():
    path = os.path.abspath(os.path.expanduser(_args.install_prefix))
    return path

def get_audio_rate():
    return min(max(2000, _args.rate), 192000)

def get_audio_latency():
    return min(max(1, _args.audio_latency), 2000)

def get_thread_count():
    return min(max(1, _args.threads), THREADS_MAX)


def _find_install_prefix():
    module_path = os.path.realpath(__file__)
    dir_name = os.path.dirname(module_path)

    all_parts = _split_all(dir_name)
    kunquat_parts = ['kunquat', 'tracker']
    for i, suf in enumerate(reversed(kunquat_parts)):
        assert all_parts[-1 - i] == suf
    python_base_parts = all_parts[:-len(kunquat_parts)]

    install_prefix_parts = python_base_parts

    # See if we have been installed
    if 'lib' in python_base_parts:
        lib_index = _rindex(python_base_parts, 'lib')
        lib_parts = python_base_parts[lib_index:]
        if len(lib_parts) > 1 and lib_parts[1].startswith('python'):
            # We are probably an installed Python module
            install_prefix_parts = python_base_parts[:lib_index]

    install_prefix = os.path.join(*install_prefix_parts)
    return install_prefix

def _split_all(path):
    if not path:
        return []

    head, tail = os.path.split(path)
    if head == path:
        return [head]

    parts = _split_all(head)
    parts.append(tail)
    return parts

def _rindex(ls, elem):
    return -1 - list(reversed(ls)).index(elem)

def get_default_thread_count():
    return processors.get_core_count()


