# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from . import tstamp


class TriggerPosition():

    def __init__(self, track, system, col_num, row_ts, trigger_index):
        self._track = track
        self._system = system
        self._col_num = col_num
        self._row_ts = row_ts
        self._trigger_index = trigger_index

    def __repr__(self):
        return 'TriggerPosition({}, {}, {}, {}, {})'.format(
                self._track,
                self._system,
                self._col_num,
                self._row_ts,
                self._trigger_index)

    def __eq__(self, other):
        if other == None:
            return False

        assert isinstance(other, TriggerPosition)
        return ((self._track == other._track) and
                (self._system == other._system) and
                (self._col_num == other._col_num) and
                (self._row_ts == other._row_ts) and
                (self._trigger_index == other._trigger_index))

    def __ne__(self, other):
        return not (self == other)

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


