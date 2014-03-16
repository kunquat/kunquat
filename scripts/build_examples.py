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
import os.path


def build_examples(builder):
    build_dir_parts = ['build', 'examples']
    build_dir = ''
    for part in build_dir_parts:
        build_dir = os.path.join(build_dir, part)
        builder.run('mkdir', '-p', build_dir)

    example_dir = os.path.join('examples')

    packages = {
            'kqtc00': 'example.kqt.bz2',
            'kqti00': 'example_ins.kqti.bz2',
            'kqte00': 'example_effect.kqte.bz2',
            'kqts00': 'example_scale.kqts.bz2',
        }

    for src, dest in packages.iteritems():
        src_path = os.path.join(example_dir, src)
        dest_path = os.path.join(build_dir, dest)
        print('Building {}'.format(dest))
        builder.run('tar', 'cj', '--format=ustar', '-f', dest_path, src_path)


