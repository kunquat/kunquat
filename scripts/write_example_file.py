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

from distutils.spawn import find_executable
import os
import os.path
import sys


# Note: This hackety sax works around the issue of Fabricate
#       not tracking tar input files properly.


# Make Fabricate see our inputs
def look_at_files(in_files):
    for in_path in in_files:
        with open(in_path) as f:
            pass


def create_archive(out_file, examples_path, archive_root):
    tar_path = find_executable('tar')
    if not tar_path:
        print('tar not found.')
        sys.exit(1)

    args = (
        'tar',
        'cj', '--format=ustar',
        '-f', out_file,
        '--directory', examples_path,
        archive_root)
    #print('running tar {}'.format(' '.join(args)))
    os.execv(tar_path, args)


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


