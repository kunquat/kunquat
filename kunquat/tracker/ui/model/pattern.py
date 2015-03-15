# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value

from column import Column
import tstamp


COLUMNS_MAX = 64 # TODO: define in libkunquat interface


class Pattern():

    def __init__(self, pattern_id):
        assert pattern_id
        self._pattern_id = pattern_id
        self._store = None
        self._controller = None
        self._existence = None

    def __eq__(self, other):
        assert isinstance(other, Pattern)
        return self._pattern_id == other._pattern_id

    def __ne__(self, other):
        return not (self == other)

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_existence(self):
        key = '{}/p_manifest.json'.format(self._pattern_id)
        manifest = self._store[key]
        return (type(manifest) == type({}))

    def get_length(self):
        key = '{}/p_pattern.json'.format(self._pattern_id)
        try:
            header = self._store[key]
            length = header['length']
            return tstamp.Tstamp(length)
        except KeyError:
            default_header = get_default_value(key)
            return tstamp.Tstamp(default_header['length'])

    def set_length(self, length):
        key = '{}/p_pattern.json'.format(self._pattern_id)
        header = self._store.get(key, get_default_value(key))
        header['length'] = list(length)
        self._store[key] = header

    def get_column(self, column_index):
        assert 0 <= column_index < COLUMNS_MAX
        column = Column(self._pattern_id, column_index)
        column.set_controller(self._controller)
        return column

    def get_instance_ids(self):
        instance_ids = set()
        for key in self._store:
            start = '{}/instance_'.format(self._pattern_id)
            if key.startswith(start):
                instance_id = key.split('/')[1]
                manifest_key = '{}/{}/p_manifest.json'.format(
                        self._pattern_id, instance_id)
                if manifest_key in self._store:
                    instance_ids.add(instance_id)
        return instance_ids

    def get_name(self):
        key = '{}/m_name.json'.format(self._pattern_id)
        try:
            name = self._store[key]
        except KeyError:
            return None
        return name

    def get_edit_create_pattern(self):
        key = '{}/p_manifest.json'.format(self._pattern_id)
        edit = { key: {} }
        return edit

    def get_edit_remove_pattern(self):
        edit = {}
        start = '{}/'.format(self._pattern_id)
        for key in self._store:
            if key.startswith(start):
                edit[key] = None
        return edit


