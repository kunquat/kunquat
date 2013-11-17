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
        self._selected_instrument_id = None
        self._updater = None
        self._model = None

    def set_controller(self, controller):
        updater = controller.get_updater()
        self._updater = updater

    def set_model(self, model):
        self._model = model

    def get_selected_instrument_id(self):
        module = self._model.get_module()
        instrument_ids = module.get_instrument_ids()
        selected_id = self._selected_instrument_id
        if len(instrument_ids) < 1:
            return None
        if not selected_id in instrument_ids:
            some_id = list(instrument_ids)[0]
            return some_id
        return selected_id

    def set_selected_instrument_id(self, instrument_id):
        self._selected_instrument_id = instrument_id
        self._updater.signal_update()

    def get_selected_instrument(self):
        instrument_id = self.get_selected_instrument_id()
        if instrument_id == None:
            return None
        module = self._model.get_module()
        instrument = module.get_instrument(instrument_id)
        return instrument

