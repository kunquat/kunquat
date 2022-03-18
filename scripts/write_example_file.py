#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2022
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os
import os.path
import shutil
import sys


# Note: This hackety sax works around the issue of Fabricate
#       not tracking zip input files properly.


# Make Fabricate see our inputs
def look_at_files(in_files):
    for in_path in in_files:
        with open(in_path) as f:
            pass


def create_archive(out_file, examples_path, archive_root):
    exec_path = shutil.which('python3')
    if not exec_path:
        print('python3 not found.')
        sys.exit(1)

    args = (
        'python3',
        '-m', 'zipfile',
        '-c',
        out_file,
        os.path.join(examples_path, archive_root))
    os.execv(exec_path, args)


def main():
    if len(sys.argv) < 4:
        print('Usage: {} <out_file> <in_dir> <in_file> [<in_file> [...]]'.format(
            sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    out_file = sys.argv[1]
    in_dir = sys.argv[2]
    in_files = sys.argv[3:]

    look_at_files(in_files)

    examples_path, archive_root = os.path.split(in_dir)
    create_archive(out_file, examples_path, archive_root)


if __name__ == '__main__':
    main()


