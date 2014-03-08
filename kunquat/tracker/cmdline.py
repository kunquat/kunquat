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

from collections import defaultdict
import argparse
import sys


def parse_arguments():
    ap = argparse.ArgumentParser(add_help=False)
    ap.add_argument('-h', '--help', action='help',
            help='Show this help and exit')
    ap.add_argument('--experimental', action='store_true',
            help='Enable experimental features')
    ap.add_argument('kqtfile', type=str, nargs='?', default='',
            help='Use kqtfile as input')

    global _args
    _args = ap.parse_args()

def get_experimental():
    return _args.experimental

def get_kqt_file():
    return _args.kqtfile


