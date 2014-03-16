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

import os.path


def make_dirs(builder, path_components):
    # Make sure that the builder keeps track of all created directories
    path = ''
    for component in path_components:
        path = os.path.join(path, component)
        builder.run('mkdir', '-p', path)


