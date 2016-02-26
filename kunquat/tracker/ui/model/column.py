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

from kunquat.kunquat.limits import *
from trigger import Trigger
from triggerposition import TriggerPosition
import tstamp


class Column():

    def __init__(self, pattern_num, instance_num, col_num):
        assert 0 <= pattern_num < PATTERNS_MAX
        assert 0 <= instance_num < PAT_INSTANCES_MAX
        assert 0 <= col_num < COLUMNS_MAX
        self._pattern_num = pattern_num
        self._instance_num = instance_num
        self._col_num = col_num
        self._trigger_rows = None
        self._store = None
        self._controller = None
        self._ui_model = None

    def __eq__(self, other):
        assert isinstance(other, Column)
        return (self._col_num == other._col_num) and \
                (self._pattern_num == other._pattern_num)

    def __ne__(self, other):
        return not (self == other)

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_trigger_row_positions(self):
        self._build_trigger_rows()
        return self._trigger_rows.keys()

    def get_trigger_row_positions_in_range(self, start, stop):
        self._build_trigger_rows()
        return [ts for ts in self._trigger_rows.keys() if start <= ts < stop]

    def get_trigger_count_at_row(self, row_ts):
        self._build_trigger_rows()
        return len(self._trigger_rows[row_ts])

    def get_trigger(self, row_ts, trigger_index):
        self._build_trigger_rows()
        return self._trigger_rows[row_ts][trigger_index]

    def has_trigger(self, row_ts, trigger_index):
        try:
            self.get_trigger(row_ts, trigger_index)
            return True
        except (KeyError, IndexError):
            return False

    def insert_trigger(self, row_ts, trigger_index, trigger):
        self._build_trigger_rows()

        if row_ts not in self._trigger_rows:
            self._trigger_rows[row_ts] = []

        self._trigger_rows[row_ts].insert(trigger_index, trigger)

        raw_data = self._make_raw_data(self._trigger_rows)
        self._store[self._get_key()] = raw_data

    def remove_trigger(self, row_ts, trigger_index):
        self._build_trigger_rows()
        assert self.has_trigger(row_ts, trigger_index)

        del self._trigger_rows[row_ts][trigger_index]

        raw_data = self._make_raw_data(self._trigger_rows)
        self._store[self._get_key()] = raw_data

    def remove_trigger_row_slice(self, row_ts, start_index, stop_index):
        self._build_trigger_rows()
        assert self.has_trigger(row_ts, 0)

        if (start_index == 0) and (stop_index >= len(self._trigger_rows[row_ts])):
            del self._trigger_rows[row_ts]
        else:
            del self._trigger_rows[row_ts][start_index:stop_index]

        raw_data = self._make_raw_data(self._trigger_rows)
        self._store[self._get_key()] = raw_data

    def get_edit_remove_trigger_rows(self, start_ts, stop_ts):
        self._build_trigger_rows()

        trows = self.get_trigger_row_positions_in_range(start_ts, stop_ts)
        for row_ts in trows:
            del self._trigger_rows[row_ts]

        raw_data = self._make_raw_data(self._trigger_rows)
        key = self._get_key()
        transaction = { key: raw_data }
        return transaction

    def _build_trigger_rows(self):
        if self._trigger_rows == None:
            # Find our location
            album = self._ui_model.get_module().get_album()
            col_location = album.get_pattern_instance_location_by_nums(
                    self._pattern_num, self._instance_num)
            assert col_location
            track_num, system_num = col_location

            self._trigger_rows = {}
            trigger_list = self._get_raw_data()
            for ts_data, evspec in trigger_list:
                ts = tstamp.Tstamp(ts_data)
                if ts not in self._trigger_rows:
                    self._trigger_rows[ts] = []
                trigger_type, argument = evspec

                trigger_index = len(self._trigger_rows[ts])
                location = TriggerPosition(
                        track_num, system_num, self._col_num, ts, trigger_index)

                trigger = Trigger(trigger_type, argument, location)
                trigger.set_ui_model(self._ui_model)
                self._trigger_rows[ts].append(trigger)

    def _get_key(self):
        key = 'pat_{:03x}/col_{:02x}/p_triggers.json'.format(
                self._pattern_num, self._col_num)
        return key

    def _get_raw_data(self):
        key = self._get_key()
        try:
            triggers = self._store[key]
            return triggers
        except KeyError:
            return []

    def _make_raw_data(self, trigger_rows):
        raw_data = []
        for (ts, triggers) in trigger_rows.iteritems():
            for trigger in triggers:
                evspec = [trigger.get_type(), trigger.get_argument()]
                raw_data.append([tuple(ts), evspec])

        return raw_data


