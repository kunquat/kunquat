# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy

from kunquat.kunquat.kunquat import get_default_value


class TuningTable():

    def __init__(self, table_id):
        self._controller = None
        self._store = None
        self._session = None

        self._table_id = table_id

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()
        self._session = controller.get_session()

    def _get_key(self, subkey):
        return '{}/{}'.format(self._table_id, subkey)

    def get_name(self):
        key = self._get_key('m_name.json')
        return self._store.get(key, None)

    def set_name(self, name):
        key = self._get_key('m_name.json')
        self._store[key] = name

    def _get_table(self):
        key = self._get_key('p_tuning_table.json')
        return self._store.get(key, get_default_value(key))

    def _set_table(self, table):
        key = self._get_key('p_tuning_table.json')
        self._store[key] = table

    def get_ref_note_index(self):
        return self._get_table()['ref_note']

    def set_ref_note_index(self, index):
        table = deepcopy(self._get_table())
        table['ref_note'] = index
        self._set_table(table)

    def get_ref_pitch(self):
        return self._get_table()['ref_pitch']

    def set_ref_pitch(self, pitch):
        table = deepcopy(self._get_table())
        table['ref_pitch'] = pitch
        self._set_table(table)

    def get_pitch_offset(self):
        return self._get_table()['pitch_offset']

    def set_pitch_offset(self, offset):
        table = deepcopy(self._get_table())
        table['pitch_offset'] = offset
        self._set_table(table)

    def get_octave_width(self):
        return self._get_table()['octave_width']

    def set_octave_width(self, octave_width):
        table = deepcopy(self._get_table())
        table['octave_width'] = octave_width
        self._set_table(table)

    def get_note_count(self):
        return len(self._get_table()['notes'])

    def _get_note_names(self):
        key = self._get_key('m_note_names.json')
        return self._store[key]

    def _set_note_names(self, names):
        key = self._get_key('m_note_names.json')
        self._store[key] = names

    def set_selected_note(self, index):
        self._session.set_tuning_table_selected_note(self._table_id, index)

    def get_selected_note(self):
        return self._session.get_tuning_table_selected_note(self._table_id)

    def add_note(self):
        table = deepcopy(self._get_table())
        table['notes'].append(0)
        note_count = len(table['notes'])

        names = deepcopy(self._get_note_names())
        if len(names) < note_count:
            names.extend([''] * (note_count - len(names)))
        names[note_count - 1] = '(n)'

        self._set_table(table)
        self._set_note_names(names)

    def remove_note(self, index):
        table = deepcopy(self._get_table())
        del table['notes'][index]

        if table['ref_note'] >= index:
            table['ref_note'] = max(0, table['ref_note'] - 1)

        names = deepcopy(self._get_note_names())
        if index < len(names):
            del names[index]

        self._set_table(table)
        self._set_note_names(names)

    def get_note_pitch(self, index):
        return self._get_table()['notes'][index]

    def set_note_pitch(self, index, pitch):
        table = deepcopy(self._get_table())
        table['notes'][index] = pitch
        self._set_table(table)

    def get_note_name(self, index):
        names = self._get_note_names()
        if index >= len(names):
            return u''
        return names[index]

    def set_note_name(self, index, name):
        names = deepcopy(self._get_note_names())

        if len(names) <= index:
            names.extend([''] * (index + 1 - len(names)))
        names[index] = name

        self._set_note_names(names)

    def remove(self):
        self.set_selected_note(None)

        key_prefix = '{}/'.format(self._table_id)
        transaction = {}
        for key in self._store:
            if key.startswith(key_prefix):
                transaction[key] = None
        self._store.put(transaction)


