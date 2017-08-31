# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2017
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy

from .notation import Notation


class NotationManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._share = None
        self._store = None
        self._ui_model = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._share = controller.get_share()
        self._store = controller.get_store()
        self._updater = controller.get_updater()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_selected_notation_id(self):
        notation_ids = self.get_all_notation_ids()
        selected_id = self._session.get_selected_notation_id()
        if len(notation_ids) < 1:
            return None
        if not selected_id in notation_ids:
            some_id = sorted(notation_ids)[0]
            return some_id
        return selected_id

    def get_notation(self, notation_id):
        notation = Notation(notation_id)
        notation.set_controller(self._controller)
        notation.set_ui_model(self._ui_model)
        return notation

    def get_selected_notation(self):
        notation_id = self.get_selected_notation_id()
        notation = self.get_notation(notation_id)
        return notation

    def get_shared_notation_ids(self):
        shared_notations = self._share.get_notations()
        is_shared = True
        shared_notation_ids = [(is_shared, k) for k in shared_notations.keys()]
        return shared_notation_ids

    def _get_custom_notation_data(self):
        return self._store.get('i_notations.json', None)

    def _set_custom_notation_data(self, data):
        self._store['i_notations.json'] = data

    def get_custom_notation_ids(self):
        custom_notations = self._get_custom_notation_data() or []
        is_shared = False
        custom_notation_ids = [(is_shared, i) for i in range(len(custom_notations))]
        return custom_notation_ids

    def get_all_notation_ids(self):
        return self.get_shared_notation_ids() + self.get_custom_notation_ids()

    def set_selected_notation_id(self, notation_id):
        self._session.set_selected_notation_id(notation_id)

    def set_editor_selected_notation_id(self, notation_id):
        self._session.set_notation_editor_selected_notation_id(notation_id)

    def get_editor_selected_notation_id(self):
        return self._session.get_notation_editor_selected_notation_id()

    def get_editor_selected_octave_id(self):
        return self._session.get_notation_editor_selected_octave_id()

    def set_editor_selected_octave_id(self, octave_id):
        self._session.set_notation_editor_selected_octave_id(octave_id)

    def get_editor_selected_note_index(self):
        return self._session.get_notation_editor_selected_note_index()

    def set_editor_selected_note_index(self, note_index):
        self._session.set_notation_editor_selected_note_index(note_index)

    def get_editor_selected_key_index(self):
        return self._session.get_notation_editor_selected_key_index()

    def set_editor_selected_key_index(self, key_index):
        self._session.set_notation_editor_selected_key_index(key_index)

    def get_editor_selected_template_note(self):
        return self._session.get_notation_editor_selected_template_note()

    def set_editor_selected_template_note(self, coords):
        self._session.set_notation_editor_selected_template_note(coords)

    def get_editor_selected_tuning_table_id(self):
        return self._session.get_notation_editor_selected_tuning_table_id()

    def set_editor_selected_tuning_table_id(self, table_id):
        self._session.set_notation_editor_selected_tuning_table_id(table_id)

    def get_editor_selected_notation(self):
        selected_id = self.get_editor_selected_notation_id()
        if not selected_id:
            return None
        return self.get_notation(selected_id)

    def add_custom_notation(self):
        data = deepcopy(self._get_custom_notation_data() or [])

        new_notation = deepcopy(self._share.get_notations()['12tetsharp'])
        new_notation['name'] = 'Notation {}'.format(len(data))

        data.append(new_notation)
        self._set_custom_notation_data(data)

    def remove_custom_notation(self, notation_id):
        is_shared, sub_id = notation_id
        assert not is_shared

        data = deepcopy(self._get_custom_notation_data())
        del data[sub_id]
        self._set_custom_notation_data(data)

        typewriter_mgr = self._ui_model.get_typewriter_manager()
        typewriter_mgr.notify_notation_changed(notation_id)


