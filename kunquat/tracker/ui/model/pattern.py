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

from column import Column


COLUMNS_MAX = 64 # TODO: define in libkunquat interface


class Pattern():

    def __init__(self, pattern_id):
        assert pattern_id
        self._pattern_id = pattern_id
        self._store = None
        self._controller = None
        self._pattern_number = None
        self._existence = None
        self._columns = [Column(pattern_id, i) for i in xrange(COLUMNS_MAX)]

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller
        for column in self._columns:
            column.set_controller(controller)

    def get_id(self):
        return self._pattern_id

    def get_existence(self):
        key = '{}/p_manifest.json'.format(self._pattern_id)
        manifest = self._store[key]
        return (type(manifest) == type({}))

    def get_length(self):
        key = '{}/p_pattern.json'.format(self._pattern_id)
        try:
            header = self._store[key]
            length = header['length']
            return length
        except KeyError:
            return [0, 0]

    def get_columns(self):
        return self._columns

    def get_name(self):
        key = '{}/m_name.json'.format(self._pattern_id)
        try:
            name = self._store[key]
        except KeyError:
            return None
        return name


