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

from kunquat.kunquat.kunquat import get_default_value

from .patterninstance import PatternInstance


class Song():

    def __init__(self, song_id, track_num):
        assert song_id
        self._song_id = song_id
        self._track_num = track_num
        self._store = None
        self._controller = None
        self._ui_model = None

    def __eq__(self, other):
        if not isinstance(other, Song):
            return False
        return (self._song_id == other._song_id)

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_number(self):
        parts = self._song_id.split('_')
        return int(parts[1], 16)

    def get_existence(self):
        key = '{}/p_manifest.json'.format(self._song_id)
        manifest = self._store.get(key)
        return (type(manifest) == type({}))

    def get_system_count(self):
        return len(self._get_order_list())

    def get_pattern_instance(self, system_num):
        order_list = self._get_order_list()
        pattern_num, instance_num = order_list[system_num]
        pattern_instance = PatternInstance(pattern_num, instance_num)
        pattern_instance.set_controller(self._controller)
        pattern_instance.set_ui_model(self._ui_model)
        return pattern_instance

    def _get_order_list_key(self):
        return '{}/p_order_list.json'.format(self._song_id)

    def _get_order_list(self):
        key = self._get_order_list_key()
        try:
            order_list = self._store[key]
        except KeyError:
            order_list = get_default_value(key)
        return order_list or []

    def get_containing_track_number(self):
        return self._track_num

    def set_name(self, name):
        assert isinstance(name, str)
        key = '{}/m_name.json'.format(self._song_id)
        self._store[key] = name

    def get_name(self):
        key = '{}/m_name.json'.format(self._song_id)
        return self._store.get(key, '')

    def get_edit_create_song(self):
        key = '{}/p_manifest.json'.format(self._song_id)
        edit = { key: {} }
        return edit

    def get_edit_remove_song(self):
        edit = {}
        start = '{}/'.format(self._song_id)
        for key in self._store:
            if key.startswith(start):
                edit[key] = None
        return edit

    def get_edit_remove_song_and_pattern_instances(self):
        edit = self.get_edit_remove_song()
        edit[self._get_order_list_key()] = None
        return edit

    def get_edit_insert_pattern_instance(self, index, pattern_instance):
        order_list = self._get_order_list()
        pattern_num = pattern_instance.get_pattern_num()
        instance_num = pattern_instance.get_instance_num()
        order_list.insert(index, [pattern_num, instance_num])

        edit = { self._get_order_list_key(): order_list }
        return edit

    def get_edit_remove_pattern_instance(self, index):
        order_list = self._get_order_list()
        del order_list[index]

        edit = { self._get_order_list_key(): order_list }
        return edit

    def get_edit_move_pattern_instance(self, from_index, to_index):
        order_list = self._get_order_list()
        pinst = order_list[from_index]
        del order_list[from_index]
        order_list.insert(to_index, pinst)

        edit = { self._get_order_list_key(): order_list }
        return edit

    def set_initial_tempo(self, tempo):
        key = '{}/p_tempo.json'.format(self._song_id)
        self._store[key] = tempo

    def get_initial_tempo(self):
        key = '{}/p_tempo.json'.format(self._song_id)
        return self._store.get(key, get_default_value(key))


