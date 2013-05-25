# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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
        self._active_instrument = None
        self._selected_instrument = None

    def get_active_instrument(self):
        return self._active_instrument

    def update_active_instrument(self, instrument):
        self._active_instrument = instrument

    def get_selected_instrument(self):
        return self._selected_instrument

    def update_selected_instrument(self, instrument):
        self._selected_instrument = instrument

