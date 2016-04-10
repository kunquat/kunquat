# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .kunquat import get_limit_info


"""Kunquat limit constants.

"""


# Define libkunquat limit constants in the module namespace
for (limit_name, limit_value) in get_limit_info().items():
    globals()[limit_name] = limit_value


