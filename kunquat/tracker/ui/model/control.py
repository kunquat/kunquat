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


class Control():

    def __init__(self, control_id):
        assert(control_id)
        self._control_id = control_id
        self._controller = None
        self._instrument_number = None
        self._existence = None
        self._session = None
        self._store = None
        self._model = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._session = controller.get_session()
        self._controller = controller

    def set_model(self, model):
        self._model = model

    def get_instrument(self):
        control_id = self.get_id()
        parts = control_id.split('_')
        second = parts[1]
        control_number = int(second, 16)
        try:
            input_map = self._store['p_control_map.json']
        except KeyError:
            input_map = []
        controls = dict(input_map)
        instrument_number = controls[control_number]
        instrument_id = 'ins_{0:02x}'.format(instrument_number)
        module = self._model.get_module()
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

    def set_active_note(self, channel_number, pitch):
        control_id = self.get_id()
        self._controller.set_active_note(channel_number, control_id, pitch)

    def set_rest(self, channel_number):
        self._controller.set_rest(channel_number)

    def get_id(self):
        return self._control_id

