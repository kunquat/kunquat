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

from triggerposition import TriggerPosition
import tstamp


class Selection():

    def __init__(self):
        self._controller = None
        self._session = None
        self._updater = None
        self._model = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._updater = controller.get_updater()

    def set_model(self, model):
        self._model = model

    def set_location(self, trigger_position):
        self._session.set_selected_location(trigger_position)
        self._updater.signal_update(set(['signal_selection']))

    def get_location(self):
        return (self._session.get_selected_location() or
                TriggerPosition(0, 0, 0, tstamp.Tstamp(0), 0))


