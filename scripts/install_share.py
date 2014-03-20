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
import glob
import os
import os.path

import command


def install_share(builder, install_prefix):
    install_share_dir = os.path.join(install_prefix, 'share', 'kunquat')
    share_dir = os.path.join('share', 'kunquat')

    for root, _, names in os.walk(share_dir):
        rel_dir = os.path.relpath(root, share_dir)
        for name in names:
            in_path = os.path.join(share_dir, rel_dir, name)
            out_dir = os.path.join(install_share_dir, rel_dir)
            command.make_dirs(builder, out_dir)
            out_path = os.path.join(out_dir, name)
            builder.run('cp', '--no-dereference', in_path, out_path)


