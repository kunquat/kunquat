# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class UiManager():

    def __init__(self):
        self._selected_instrument = None
        self._updater = None

    def set_updater(self, updater):
        self._updater = updater

    def get_selected_instrument(self):
        return self._selected_instrument

    def set_selected_instrument(self, instrument):
        self._selected_instrument = instrument
        self._updater.signal_update()

