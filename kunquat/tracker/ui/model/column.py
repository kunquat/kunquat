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

COLUMNS_MAX = 64 # TODO: define in libkunquat interface


class Column():

    def __init__(self, pattern_id, col_num):
        assert pattern_id
        assert 0 <= col_num < COLUMNS_MAX
        self._pattern_id = pattern_id
        self._col_num = col_num

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_triggers(self):
        key = '{}/col_{:02x}/p_triggers.json'.format(
                self._pattern_id, self._col_num)
        try:
            triggers = self._store[key]
            return triggers
        except KeyError:
            return []


