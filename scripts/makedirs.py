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


def make_dirs(builder, path):
    path_components = _split_all(path)
    # Make sure that the builder keeps track of all created directories
    prefix = ''
    for component in path_components:
        prefix = os.path.join(prefix, component)
        builder.run('mkdir', '-p', prefix)


def _split_all(path):
    if not path:
        return []

    head, tail = os.path.split(path)
    if head == path:
        return [head]

    parts = _split_all(head)
    parts.append(tail)
    return parts


