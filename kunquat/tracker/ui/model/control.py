# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class Control():

    def __init__(self, control_id):
        assert(control_id)
        self._control_id = control_id
        self._controller = None
        self._instrument_number = None
        self._existence = None
        self._session = None
        self._store = None
        self._ui_model = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._session = controller.get_session()
        self._controller = controller

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_instrument(self):
        parts = self._control_id.split('_')
        second = parts[1]
        control_number = int(second, 16)
        try:
            input_map = self._store['p_control_map.json']
        except KeyError:
            input_map = []
        controls = dict(input_map)
        instrument_number = controls[control_number]
        instrument_id = 'au_{0:02x}'.format(instrument_number)
        module = self._ui_model.get_module()
        instrument = module.get_instrument(instrument_id)
        return instrument

    def get_active_notes(self):
        notes = self._session.get_active_notes_by_control_id(self._control_id)
        return notes

    def get_active_note(self, channel_number):
        notes = self.get_active_notes()
        if channel_number not in notes:
            return None
        return notes[channel_number]

    def start_tracked_note(self, channel_number, pitch):
        note = self._controller.start_tracked_note(
                channel_number, self._control_id, pitch)
        return note


