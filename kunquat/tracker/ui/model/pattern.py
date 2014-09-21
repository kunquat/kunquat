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
        self._pattern_number = None
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

    def get_column(self, column_index):
        assert 0 <= column_index < COLUMNS_MAX
        column = Column(self._pattern_id, column_index)
        column.set_controller(self._controller)
        return column

    def get_name(self):
        key = '{}/m_name.json'.format(self._pattern_id)
        try:
            name = self._store[key]
        except KeyError:
            return None
        return name


