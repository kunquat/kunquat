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
        self._selected_slot_id = None
        self._updater = None
        self._model = None

    def set_controller(self, controller):
        updater = controller.get_updater()
        self._updater = updater

    def set_model(self, model):
        self._model = model

    def get_selected_slot_id(self):
        module = self._model.get_module()
        slot_ids = module.get_slot_ids()
        selected_id = self._selected_slot_id
        if len(slot_ids) < 1:
            return None
        if not selected_id in slot_ids:
            some_id = sorted(slot_ids)[0]
            return some_id
        return selected_id

    def set_selected_slot_id(self, slot_id):
        self._selected_slot_id = slot_id
        self._updater.signal_update()

    def get_selected_slot(self):
        slot_id = self.get_selected_slot_id()
        if slot_id == None:
            return None
        module = self._model.get_module()
        slot = module.get_slot(slot_id)
        return slot

