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

from updater import Updater

class UiManager(Updater):

    def __init__(self):
        super(UiManager, self).__init__()
        self._selected_instrument = None

    def get_selected_instrument(self):
        return self._selected_instrument

    def set_selected_instrument(self, instrument):
        self._selected_instrument = instrument
        self._signal_update()

