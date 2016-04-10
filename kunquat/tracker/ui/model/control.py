# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value


class Control():

    def __init__(self, control_id):
        assert(control_id)
        self._control_id = control_id
        self._controller = None
        self._au_number = None
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

    def set_existence(self, existence):
        key = '{}/p_manifest.json'.format(self._control_id)
        if existence:
            self._store[key] = {}
        else:
            del self._store[key]

    def get_existence(self):
        key = '{}/p_manifest.json'.format(self._control_id)
        return (type(self._store.get(key)) == dict)

    def get_edit_remove_control(self):
        key = '{}/p_manifest.json'.format(self._control_id)
        return { key: None }

    def get_audio_unit(self):
        parts = self._control_id.split('_')
        second = parts[1]
        control_number = int(second, 16)
        cmap_key = 'p_control_map.json'
        input_map = self._store.get(cmap_key, get_default_value(cmap_key))
        controls = dict(input_map)
        au_number = controls[control_number]
        au_id = 'au_{0:02x}'.format(au_number)
        module = self._ui_model.get_module()
        au = module.get_audio_unit(au_id)
        return au

    def connect_to_au(self, au_id):
        control_num = int(self._control_id.split('_')[1], 16)
        au_num = int(au_id.split('_')[1], 16)

        cmap_key = 'p_control_map.json'
        control_map = self._store.get(cmap_key, get_default_value(cmap_key))
        controls = dict(control_map)
        controls[control_num] = au_num
        self._store[cmap_key] = list(controls.items())

        self._au_number = au_num

    def get_active_notes(self):
        notes = self._session.get_active_notes_by_control_id(self._control_id)
        return notes

    def get_active_note(self, channel_number):
        notes = self.get_active_notes()
        if channel_number not in notes:
            return None
        return notes[channel_number]

    def get_active_hits(self):
        hits = self._session.get_active_hits_by_control_id(self._control_id)
        return hits

    def start_tracked_note(self, channel_number, event_type, param):
        note = self._controller.start_tracked_note(
                channel_number, self._control_id, event_type, param)
        return note


