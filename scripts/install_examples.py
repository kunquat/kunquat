# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import glob
import os.path

from . import command


def install_examples(builder, install_prefix):
    install_examples_dir = os.path.join(
            install_prefix, 'share', 'doc', 'kunquat', 'examples')

    build_dir = os.path.join('build', 'examples')

    names = [
        'example.kqt.bz2',
        'example_ins.kqti.bz2',
    ]

    for name in names:
        in_path = os.path.join(build_dir, name)
        out_path = os.path.join(install_examples_dir, name)
        command.copy(builder, in_path, out_path)


