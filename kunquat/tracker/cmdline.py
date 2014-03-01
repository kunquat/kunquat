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
import getopt
import sys


# Public interface

def get_help():
    return _args['help']

def get_experimental():
    return _args['experimental']

def get_kqt_file():
    return _args['kqt_file']

def print_help():
    print('Usage: kunquat-tracker [options] [kqt_file]')
    print('\nOptions:')
    print('  -h, --help       Show this help and exit')
    print('  --experimental   Enable experimental features')
    print()


# Parse command line arguments

_args = defaultdict(lambda: None)


def _parse_cmd_args():
    long_opts = [
            'help',
            'experimental',
        ]
    try:
        opts, paths = getopt.getopt(sys.argv[1:], 'h', long_opts)
    except getopt.GetoptError as e:
        _option_error(e.msg)

    short_to_long = {
            '-h': '--help',
        }

    for option, value in opts:
        long_opt = option if option.startswith('--') else short_to_long[option]
        key = long_opt[2:]
        stored_value = value if value else True
        _args[key] = stored_value

    if paths:
        _args['kqt_file'] = paths[0]


def _option_error(msg):
    sys.exit(msg + '\nUse -h for help.')


_parse_cmd_args()


