# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

class Command(tuple):

    def __new__(cls, name, arg):
        return tuple.__new__(cls, (name, arg))

    @property
    def name(self):
        return self[0]

    @property
    def arg(self):
        return self[1]


