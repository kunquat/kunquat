# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class Channel():

    def __init__(self):
        self._active_au_number = None
        self._selected_au_number = None

    def get_active_au_number(self):
        return self._active_au_number

    def get_selected_au_number(self):
        return self._selected_au_number


