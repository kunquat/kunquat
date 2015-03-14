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

import tstamp


class TriggerPosition():

    def __init__(self, track, system, col_num, row_ts, trigger_index):
        self._track = track
        self._system = system
        self._col_num = col_num
        self._row_ts = row_ts
        self._trigger_index = trigger_index

    def get_track(self):
        return self._track

    def set_track(self, track):
        self._track = track

    def get_system(self):
        return self._system

    def set_system(self, system):
        self._system = system

    def get_col_num(self):
        return self._col_num

    def get_row_ts(self):
        return self._row_ts

    def set_row_ts(self, row_ts):
        self._row_ts = row_ts

    def get_trigger_index(self):
        return self._trigger_index


