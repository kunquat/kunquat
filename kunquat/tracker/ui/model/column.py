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
from .trigger import Trigger
from .triggerposition import TriggerPosition
from . import tstamp


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

    def flush_cache(self):
        self._trigger_rows = None

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

    def get_edit_replace_or_insert_trigger(self, row_ts, trigger_index, trigger):
        if not self.has_trigger(row_ts, trigger_index):
            return self.get_edit_insert_trigger(row_ts, trigger_index, trigger)

        trows = self._copy_trigger_rows(self._trigger_rows)

        trows[row_ts][trigger_index] = trigger

        raw_data = self._make_raw_data(trows)
        transaction = { self._get_key(): raw_data }
        return transaction

    def get_edit_insert_trigger(self, row_ts, trigger_index, trigger):
        self._build_trigger_rows()

        trows = self._copy_trigger_rows(self._trigger_rows)

        if row_ts not in trows:
            trows[row_ts] = []

        trows[row_ts].insert(trigger_index, trigger)

        raw_data = self._make_raw_data(trows)
        transaction = { self._get_key(): raw_data }
        return transaction

    def get_edit_remove_trigger(self, row_ts, trigger_index):
        self._build_trigger_rows()
        assert self.has_trigger(row_ts, trigger_index)

        trows = self._copy_trigger_rows(self._trigger_rows)

        del trows[row_ts][trigger_index]

        raw_data = self._make_raw_data(trows)
        transaction = { self._get_key(): raw_data }
        return transaction

    def _copy_trigger_rows(self, trows):
        new_trows = {}

        for row_ts, row in trows.iteritems():
            new_row = []
            for trigger in row:
                new_row.append(Trigger(trigger))
            new_trows[row_ts] = new_row

        return new_trows

    def get_edit_replace_trigger_row_slice(
            self, row_ts, start_index, stop_index, triggers):
        self._build_trigger_rows()

        trows = self._copy_trigger_rows(self._trigger_rows)

        trows[row_ts][start_index:stop_index] = triggers

        key = self._get_key()
        raw_data = self._make_raw_data(trows)
        transaction = { key: raw_data }
        return transaction

    def get_edit_insert_trigger_row_slice(self, row_ts, trigger_index, triggers):
        self._build_trigger_rows()

        trows = self._copy_trigger_rows(self._trigger_rows)

        if row_ts not in trows:
            trows[row_ts] = []

        trows[row_ts][trigger_index:trigger_index] = triggers

        key = self._get_key()
        raw_data = self._make_raw_data(trows)
        transaction = { key: raw_data }
        return transaction

    def get_edit_remove_trigger_row_slice(self, row_ts, start_index, stop_index):
        self._build_trigger_rows()
        assert self.has_trigger(row_ts, 0)

        trows = self._copy_trigger_rows(self._trigger_rows)

        if (start_index == 0) and (stop_index >= len(trows[row_ts])):
            del trows[row_ts]
        else:
            del trows[row_ts][start_index:stop_index]

        key = self._get_key()
        raw_data = self._make_raw_data(trows)
        transaction = { key: raw_data }
        return transaction

    def _get_length(self):
        track_num, system_num = self._get_col_location()
        album = self._ui_model.get_module().get_album()
        song = album.get_song_by_track(track_num)
        pinst = song.get_pattern_instance(system_num)
        return pinst.get_pattern().get_length()

    def get_edit_replace_trigger_rows(self, start_ts, stop_ts, new_trows):
        self._build_trigger_rows()

        all_trows = self._copy_trigger_rows(self._trigger_rows)

        old_trow_positions = self.get_trigger_row_positions_in_range(start_ts, stop_ts)
        for row_ts in old_trow_positions:
            del all_trows[row_ts]

        length = self._get_length()

        for row_ts, row in new_trows.iteritems():
            abs_ts = start_ts + row_ts
            if abs_ts > length:
                continue
            all_trows[abs_ts] = row

        key = self._get_key()
        raw_data = self._make_raw_data(all_trows)
        transaction = { key: raw_data }
        return transaction

    def get_edit_remove_trigger_rows(self, start_ts, stop_ts):
        self._build_trigger_rows()

        all_trows = self._copy_trigger_rows(self._trigger_rows)

        trow_positions = self.get_trigger_row_positions_in_range(start_ts, stop_ts)
        for row_ts in trow_positions:
            del all_trows[row_ts]

        raw_data = self._make_raw_data(all_trows)
        key = self._get_key()
        transaction = { key: raw_data }
        return transaction

    def _get_validated_tstamp(self, ts_pair):
        if (type(ts_pair) != list) or (len(ts_pair) != 2):
            return None
        if not all(type(t) == int for t in ts_pair):
            return None
        if (ts_pair[0] < 0) or not (0 <= ts_pair[1] < tstamp.BEAT):
            return None
        ts = tstamp.Tstamp(ts_pair)
        return ts

    def _get_overlay_grid_key(self):
        key = '{}/i_overlay_grids.json'.format(self._get_key_base_path())
        return key

    def _get_validated_overlay_grid_info(self):
        key = self._get_overlay_grid_key()

        unsafe_data = self._store.get(key, [])
        if type(unsafe_data) != list:
            return []

        result = []
        prev_ts = -tstamp.Tstamp(0, 1)
        try:
            for unsafe_item in unsafe_data:
                row, gp_id, offset = unsafe_item

                row = self._get_validated_tstamp(row)
                offset = self._get_validated_tstamp(offset)
                if (row == None) or (offset == None):
                    return []
                if row <= prev_ts:
                    return []
                if (gp_id != None) and (not isinstance(gp_id, str)):
                    return []
                prev_ts = row

                result.append((row, gp_id, offset))

        except ValueError:
            return []

        return result

    def _get_edit_set_overlay_grid_info(self, info):
        key = self._get_overlay_grid_key()

        if not info:
            return { key: None }

        raw_info = []
        for entry in info:
            row, gp_id, offset = entry
            raw_info.append((list(row), gp_id, list(offset)))

        return { key: raw_info }

    def get_overlay_grid_info_slice(self, start_ts, stop_ts):
        info = self._get_validated_overlay_grid_info()

        smaller = filter(lambda x: x[0] < start_ts, info)
        contained = filter(lambda x: start_ts <= x[0] < stop_ts, info)

        # See if contained can be used as-is
        if contained and contained[0][0] == start_ts:
            return contained

        first_gp_id = None
        first_offset = tstamp.Tstamp(0)
        if smaller:
            _, first_gp_id, first_offset = smaller[-1]

        info_slice = [(start_ts, first_gp_id, first_offset)]
        info_slice.extend(contained)
        return info_slice

    def get_edit_set_overlay_grid(self, start_ts, stop_ts, gp_id, offset):
        assert (gp_id == None) or isinstance(gp_id, str)
        info = self._get_validated_overlay_grid_info()

        smaller = filter(lambda x: x[0] < start_ts, info)
        contained = filter(lambda x: start_ts <= x[0] <= stop_ts, info)
        greater = filter(lambda x: x[0] > stop_ts, info)

        # Get active grid settings at stop_ts
        following_gp_id = None
        following_offset = tstamp.Tstamp(0)
        if contained:
            _, following_gp_id, following_offset = contained[-1]
        elif smaller:
            _, following_gp_id, following_offset = smaller[-1]

        # Construct the new settings list
        new_info = smaller
        new_info.append((start_ts, gp_id, offset))
        new_info.append((stop_ts, following_gp_id, following_offset))
        new_info.extend(greater)

        # Remove repeated settings
        final_new_info = []
        prev_gp_id = None
        prev_offset = tstamp.Tstamp(0)
        for entry in new_info:
            ts, gp_id, offset = entry
            if (gp_id, offset) == (prev_gp_id, prev_offset):
                continue
            final_new_info.append(entry)
            prev_gp_id, prev_offset = gp_id, offset

        return self._get_edit_set_overlay_grid_info(final_new_info)

    def get_edit_clear_overlay_grids(self):
        return self._get_edit_set_overlay_grid_info([])

    def _get_overlay_grids_with_start_index(self, row_ts):
        info = self._get_validated_overlay_grid_info()
        smaller_eq = filter(lambda x: x[0] <= row_ts, info)
        return info, len(smaller_eq) - 1

    def get_overlay_grid_info_at(self, row_ts):
        info, index = self._get_overlay_grids_with_start_index(row_ts)

        if 0 <= index < len(info):
            _, gp_id, offset = info[index]
        else:
            gp_id, offset = None, tstamp.Tstamp(0)

        return gp_id, offset

    def get_overlay_grid_range_at(self, row_ts):
        info, index = self._get_overlay_grids_with_start_index(row_ts)
        length = self._get_length()

        start_ts = tstamp.Tstamp(0)
        stop_ts = length + tstamp.Tstamp(0, 1)

        if 0 <= index < len(info):
            start_ts = info[index][0]
        if index < (len(info) - 1):
            stop_ts = info[index + 1][0]

        return start_ts, stop_ts

    def _get_col_location(self):
        album = self._ui_model.get_module().get_album()
        col_location = album.get_pattern_instance_location_by_nums(
                self._pattern_num, self._instance_num)
        assert col_location
        return col_location

    def _build_trigger_rows(self):
        if self._trigger_rows == None:
            # Find our location
            track_num, system_num = self._get_col_location()

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

    def _get_key_base_path(self):
        return 'pat_{:03x}/col_{:02x}'.format(self._pattern_num, self._col_num)

    def _get_key(self):
        key = '{}/p_triggers.json'.format(self._get_key_base_path())
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


