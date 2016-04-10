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

import json
import types

from kunquat.kunquat.events import trigger_events_by_name
from kunquat.kunquat.limits import *
from .grid import Grid
from .trigger import Trigger
from .triggerposition import TriggerPosition
from . import tstamp


class SheetManager():

    @staticmethod
    def get_column_signal_head():
        return 'signal_column'

    @staticmethod
    def encode_column_signal(track_num, system_num, col_num):
        return '_'.join((
            SheetManager.get_column_signal_head(),
            str(track_num),
            str(system_num),
            str(col_num)))

    @staticmethod
    def decode_column_signal(signal):
        head = SheetManager.get_column_signal_head()
        assert signal.startswith(head)
        numbers_part = signal[len(head):]
        str_parts = numbers_part.split('_')[1:]
        int_parts = [int(s) for s in str_parts]
        track_num, system_num, col_num = int_parts
        return (track_num, system_num, col_num)

    def __init__(self):
        self._controller = None
        self._session = None
        self._updater = None
        self._store = None
        self._ui_model = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._updater = controller.get_updater()
        self._store = controller.get_store()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def flush_latest_column(self):
        self._session.set_last_column(None)

    def get_column_at_location(self, location):
        module = self._ui_model.get_module()
        album = module.get_album()

        if album and album.get_track_count() > 0:
            song = album.get_song_by_track(location.get_track())
            pinst = song.get_pattern_instance(location.get_system())
            column = pinst.get_column(location.get_col_num())

            cached = self._session.get_last_column()
            if cached and (cached == column):
                return cached

            self._session.set_last_column(column)
            return column

        return None

    def get_clamped_location(self, location):
        column = self.get_column_at_location(location)
        if not column:
            return None

        row_ts = location.get_row_ts()
        if column.has_trigger(row_ts, 0):
            new_trigger_index = min(
                    location.get_trigger_index(),
                    column.get_trigger_count_at_row(row_ts))
        else:
            new_trigger_index = 0

        new_location = TriggerPosition(
                location.get_track(),
                location.get_system(),
                location.get_col_num(),
                location.get_row_ts(),
                new_trigger_index)
        return new_location

    def get_inferred_active_control_id_at_location(self, location):
        ret_id = 'control_00'
        clamped_loc = self.get_clamped_location(location)
        if not clamped_loc:
            return ret_id

        # Use channel default if we don't get any better ideas
        module = self._ui_model.get_module()
        ch_defs = module.get_channel_defaults()
        ret_id = ch_defs.get_default_control_id(clamped_loc.get_col_num())

        # See if there is an audio unit set event at the selected row
        column = self.get_column_at_location(clamped_loc)
        if column.has_trigger(clamped_loc.get_row_ts(), 0):
            triggers = [column.get_trigger(clamped_loc.get_row_ts(), i)
                    for i in range(clamped_loc.get_trigger_index())]
            for trigger in reversed(triggers):
                if trigger.get_type() == '.a':
                    try:
                        control_num = int(trigger.get_argument())
                        ret_id = 'control_{:02x}'.format(control_num)
                        return ret_id
                    except ValueError:
                        pass

        # See if there is an audio unit set event above the selection
        trow_positions = column.get_trigger_row_positions_in_range(
                tstamp.Tstamp(0), clamped_loc.get_row_ts())
        for row_ts in reversed(trow_positions):
            tr_count = column.get_trigger_count_at_row(row_ts)
            triggers = [column.get_trigger(row_ts, i) for i in range(tr_count)]
            for trigger in reversed(triggers):
                if trigger.get_type() == '.a':
                    try:
                        control_num = int(trigger.get_argument())
                        ret_id = 'control_{:02x}'.format(control_num)
                        return ret_id
                    except ValueError:
                        pass

        return ret_id

    def set_chord_mode(self, enabled):
        was_enabled = self.get_chord_mode()

        self._session.set_chord_mode(enabled)

        if enabled:
            if self._session.get_chord_start() == None:
                selection = self._ui_model.get_selection()
                location = selection.get_location()
                self._session.set_chord_start(location)
        else:
            chord_start = self._session.get_chord_start()
            if chord_start != None:
                selection = self._ui_model.get_selection()
                cur_location = selection.get_location()
                if cur_location.get_col_num() == chord_start.get_col_num():
                    new_location = cur_location
                else:
                    chord_next = TriggerPosition(
                            chord_start.get_track(),
                            chord_start.get_system(),
                            chord_start.get_col_num(),
                            chord_start.get_row_ts(),
                            chord_start.get_trigger_index() + 1)
                    new_location = self.get_clamped_location(chord_next)

                selection.set_location(new_location)
                self._session.set_chord_start(None)

            # Finish our undo step
            if was_enabled:
                history = self._ui_model.get_sheet_history()
                history.commit()

    def get_chord_mode(self):
        return self._session.get_chord_mode()

    def is_at_trigger(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            return False

        cur_column = self.get_column_at_location(location)
        if not cur_column:
            return False
        row_ts = location.get_row_ts()
        index = location.get_trigger_index()

        return cur_column.has_trigger(row_ts, index)

    def is_at_trigger_row(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            return False

        cur_column = self.get_column_at_location(location)
        if not cur_column:
            return False
        row_ts = location.get_row_ts()

        return row_ts in cur_column.get_trigger_row_positions()

    def get_selected_trigger(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        cur_column = self.get_column_at_location(location)
        row_ts = location.get_row_ts()
        index = location.get_trigger_index()

        return cur_column.get_trigger(row_ts, index)

    def _add_transaction(self, transaction, add_location=True, commit=None):
        location = None
        if add_location:
            location = self._ui_model.get_selection().get_location()
        history = self._ui_model.get_sheet_history()

        if commit == None:
            commit = not self.get_chord_mode()

        history.add_step(transaction, location, commit)

        self._store.put(transaction)

    def add_trigger(self, trigger, commit=None):
        if not self.is_editing_enabled():
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            return

        cur_column = self.get_column_at_location(location)
        if not cur_column:
            return
        row_ts = location.get_row_ts()
        index = location.get_trigger_index()

        if self.get_replace_mode():
            transaction = cur_column.get_edit_replace_or_insert_trigger(
                    row_ts, index, trigger)
        else:
            transaction = cur_column.get_edit_insert_trigger(row_ts, index, trigger)
        self._add_transaction(transaction, commit=commit)

        # This needs to be done before updating our location below
        self._on_column_update(location)

        cur_col_num = location.get_col_num()
        if self.get_chord_mode() and (cur_col_num < COLUMNS_MAX - 1):
            new_col_num = cur_col_num + 1
            new_location = TriggerPosition(
                    location.get_track(),
                    location.get_system(),
                    new_col_num,
                    location.get_row_ts(),
                    location.get_trigger_index())
        else:
            new_trigger_count = cur_column.get_trigger_count_at_row(row_ts)
            new_trigger_index = min(new_trigger_count, location.get_trigger_index() + 1)
            new_location = TriggerPosition(
                    location.get_track(),
                    location.get_system(),
                    location.get_col_num(),
                    location.get_row_ts(),
                    new_trigger_index)

        selection.set_location(new_location)

    def _get_edit_try_remove_trigger(self):
        if not self.is_editing_enabled():
            return {}

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            return {}

        cur_column = self.get_column_at_location(location)
        if not cur_column:
            return {}
        row_ts = location.get_row_ts()
        index = location.get_trigger_index()

        if not cur_column.has_trigger(row_ts, index):
            return {}

        transaction = cur_column.get_edit_remove_trigger(row_ts, index)
        return transaction

    def try_remove_trigger(self):
        transaction = self._get_edit_try_remove_trigger()
        if transaction:
            self._add_transaction(transaction)
            location = self._ui_model.get_selection().get_location()
            self._on_column_update(location)

    def try_remove_area(self):
        if not self.is_editing_enabled():
            return

        selection = self._ui_model.get_selection()
        if not selection.has_area():
            return

        top_left = selection.get_area_top_left()
        bottom_right = selection.get_area_bottom_right()

        if selection.has_trigger_row_slice():
            start_index = top_left.get_trigger_index()
            stop_index = bottom_right.get_trigger_index()
            cur_column = self.get_column_at_location(top_left)
            transaction = cur_column.get_edit_remove_trigger_row_slice(
                    top_left.get_row_ts(), start_index, stop_index)
            self._add_transaction(transaction)
            selection.set_location(top_left)
            self._on_column_update(top_left)

        elif selection.has_rect_area():
            start_col = top_left.get_col_num()
            stop_col = bottom_right.get_col_num() + 1
            start_ts = top_left.get_row_ts()
            stop_ts = bottom_right.get_row_ts()

            transaction = {}
            for col_num in range(start_col, stop_col):
                cur_location = TriggerPosition(
                    top_left.get_track(), top_left.get_system(), col_num, start_ts, 0)
                cur_column = self.get_column_at_location(cur_location)
                edit = cur_column.get_edit_remove_trigger_rows(start_ts, stop_ts)
                transaction.update(edit)
                self._on_column_update(cur_location)

            self._add_transaction(transaction)

        else:
            assert False

        selection.clear_area()

    @staticmethod
    def get_serialised_area_type():
        return 'application/json'

    def _get_col_key(self, col_index):
        assert 0 <= col_index < COLUMNS_MAX
        return 'col_{:02x}'.format(col_index)

    def get_serialised_area(self):
        selection = self._ui_model.get_selection()
        assert selection.has_area()

        top_left = selection.get_area_top_left()
        bottom_right = selection.get_area_bottom_right()

        area_info = {}

        if selection.has_trigger_row_slice():
            area_info['type'] = 'trow_slice'

            start_index = top_left.get_trigger_index()
            stop_index = bottom_right.get_trigger_index()
            column = self.get_column_at_location(top_left)
            row_ts = top_left.get_row_ts()
            triggers = (column.get_trigger(row_ts, i)
                    for i in range(start_index, stop_index))
            trigger_tuples = [(t.get_type(), t.get_argument()) for t in triggers]

            area_info['triggers'] = trigger_tuples

        elif selection.has_rect_area():
            area_info['type'] = 'rect'

            start_col = top_left.get_col_num()
            stop_col = bottom_right.get_col_num() + 1
            start_ts = top_left.get_row_ts()
            stop_ts = bottom_right.get_row_ts()

            area_info['width'] = stop_col - start_col
            area_info['height'] = tuple(stop_ts - start_ts)

            # Extract triggers with relative locations
            for col_index in range(start_col, stop_col):
                cur_location = TriggerPosition(
                    top_left.get_track(), top_left.get_system(), col_index, start_ts, 0)
                cur_column = self.get_column_at_location(cur_location)
                col_area_data = {}
                for row_ts in cur_column.get_trigger_row_positions_in_range(
                        start_ts, stop_ts):
                    trigger_count = cur_column.get_trigger_count_at_row(row_ts)
                    triggers = []
                    for trigger_index in range(trigger_count):
                        trigger = cur_column.get_trigger(row_ts, trigger_index)
                        triggers.append((trigger.get_type(), trigger.get_argument()))

                    rel_ts = row_ts - start_ts
                    col_area_data[str(tuple(rel_ts))] = triggers

                rel_col_index = col_index - start_col
                area_info[self._get_col_key(rel_col_index)] = col_area_data

        else:
            assert False

        return json.dumps(area_info)

    def _is_trigger_valid(self, unsafe_trigger):
        if (type(unsafe_trigger) != list) or (len(unsafe_trigger) != 2):
            return False
        tr_type, tr_arg = unsafe_trigger
        if ((type(tr_type) != str) or
                (tr_type not in trigger_events_by_name)):
            return False
        if type(tr_arg) not in (str, types.NoneType):
            return False
        if ((type(tr_arg) == None) !=
                (trigger_events_by_name[tr_type] == None)):
            return False

        return True

    def _unpack_tstamp_str(self, ts_str):
        parts = ts_str.strip('()').split(',')
        try:
            beats_str, rem_str = parts
            beats = int(beats_str)
            rem = int(rem_str)
            if beats < 0:
                return None
            if not 0 <= rem < tstamp.BEAT:
                return None
            return tstamp.Tstamp(beats, rem)
        except ValueError:
            return None
        assert False

    def _get_validated_area_info(self, unsafe_area_info):
        area_info = {}
        try:
            if unsafe_area_info['type'] == 'trow_slice':
                area_info['type'] = unsafe_area_info['type']

                triggers = unsafe_area_info['triggers']
                if type(triggers) != list:
                    return None
                if not all(self._is_trigger_valid(t) for t in triggers):
                    return None
                area_info['triggers'] = [Trigger(t[0], t[1]) for t in triggers]

            elif unsafe_area_info['type'] == 'rect':
                area_info['type'] = unsafe_area_info['type']

                width = unsafe_area_info['width']
                if (type(width) != int) or not (1 <= width <= COLUMNS_MAX):
                    return None
                height = unsafe_area_info['height']
                if (type(height) != list) or (len(height) != 2):
                    return None
                if not all(type(n) == int for n in height):
                    return None
                height_beats, height_rem = height
                if height_beats < 0:
                    return None
                if not 0 <= height_rem < tstamp.BEAT:
                    return None
                area_info['width'] = width
                area_info['height'] = tstamp.Tstamp(height_beats, height_rem)

                for col_index in range(COLUMNS_MAX):
                    col_key = self._get_col_key(col_index)
                    if col_key in unsafe_area_info:
                        col_area_data = unsafe_area_info[col_key]
                        col_data = {}
                        for ts_str, triggers in col_area_data.items():
                            row_ts = self._unpack_tstamp_str(ts_str)
                            if row_ts == None:
                                return None
                            if row_ts >= area_info['height']:
                                return None
                            if type(triggers) != list:
                                return None
                            if not all(self._is_trigger_valid(t) for t in triggers):
                                return None
                            col_data[row_ts] = [Trigger(t[0], t[1]) for t in triggers]
                        area_info[col_key] = col_data

            else:
                return None
        except KeyError:
            return None

        return area_info

    def is_area_data_valid(self, unsafe_area_data):
        unsafe_area_info = json.loads(unsafe_area_data)
        area_info = self._get_validated_area_info(unsafe_area_info)
        return area_info != None

    def try_paste_serialised_area(self, unsafe_area_data):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            return

        unsafe_area_info = json.loads(unsafe_area_data)
        area_info = self._get_validated_area_info(unsafe_area_info)
        if area_info == None:
            return

        if area_info['type'] == 'trow_slice':
            column = self.get_column_at_location(location)
            triggers = area_info['triggers']

            if selection.has_trigger_row_slice():
                top_left = selection.get_area_top_left()
                bottom_right = selection.get_area_bottom_right()
                start_index = top_left.get_trigger_index()
                stop_index = bottom_right.get_trigger_index()
                transaction = column.get_edit_replace_trigger_row_slice(
                        location.get_row_ts(), start_index, stop_index, triggers)
            else:
                start_index = location.get_trigger_index()
                transaction = column.get_edit_insert_trigger_row_slice(
                        location.get_row_ts(), start_index, triggers)
            new_location = TriggerPosition(
                    location.get_track(),
                    location.get_system(),
                    location.get_col_num(),
                    location.get_row_ts(),
                    start_index + len(triggers))

            self._add_transaction(transaction)

            selection.set_location(new_location)
            self._on_column_update(location)

        elif area_info['type'] == 'rect':
            width = area_info['width']
            height = area_info['height']

            start_ts = location.get_row_ts()
            stop_ts = start_ts + height

            transaction = {}
            for rel_col_num in range(width):
                col_num = location.get_col_num() + rel_col_num
                if col_num >= COLUMNS_MAX:
                    break

                cur_location = TriggerPosition(
                    location.get_track(), location.get_system(), col_num, start_ts, 0)
                cur_column = self.get_column_at_location(cur_location)

                edit = cur_column.get_edit_replace_trigger_rows(
                        start_ts, stop_ts, area_info[self._get_col_key(rel_col_num)])
                transaction.update(edit)

                self._on_column_update(cur_location)

            self._add_transaction(transaction)

        else:
            assert False

    def set_pattern_length(self, pattern, new_length, is_final):
        transaction = pattern.get_edit_set_length(new_length)
        self._add_transaction(transaction, add_location=False, commit=is_final)

    def set_pattern_base_grid_pattern_id(self, pattern, gp_id, is_final):
        transaction = pattern.get_edit_set_base_grid_pattern_id(gp_id)
        self._add_transaction(transaction, add_location=False, commit=is_final)

    def set_overlay_grid(
            self, pinst, start_col, stop_col, start_ts, stop_ts, gp_id, offset):
        for col_num in range(start_col, stop_col):
            column = pinst.get_column(col_num)
            transaction = column.get_edit_set_overlay_grid(
                    start_ts, stop_ts, gp_id, offset)
            self._add_transaction(transaction, commit=(col_num == stop_col - 1))

    def clear_overlay_grids(self, pinst, start_col, stop_col):
        for col_num in range(start_col, stop_col):
            column = pinst.get_column(col_num)
            transaction = column.get_edit_clear_overlay_grids()
            self._add_transaction(transaction, commit=(col_num == stop_col - 1))

    def _on_column_update(self, location):
        track_num = location.get_track()
        system_num = location.get_system()
        col_num = location.get_col_num()
        signal = SheetManager.encode_column_signal(track_num, system_num, col_num)

        column = self.get_column_at_location(location)
        column.flush_cache()

        self._updater.signal_update(set([signal]))

        # Clear cached column data
        self._session.set_last_column(None)

    def set_zoom(self, zoom):
        old_zoom = self._session.get_sheet_zoom()
        self._session.set_sheet_zoom(zoom)
        if self._session.get_sheet_zoom() != old_zoom:
            self._updater.signal_update(set(['signal_sheet_zoom']))

    def set_zoom_range(self, minimum, maximum):
        old_zoom = self._session.get_sheet_zoom()
        self._session.set_sheet_zoom_range(minimum, maximum)
        signals = set(['signal_sheet_zoom_range'])
        if self._session.get_sheet_zoom() != old_zoom:
            signals.add('signal_sheet_zoom')
        self._updater.signal_update(signals)

    def get_zoom(self):
        return self._session.get_sheet_zoom()

    def get_zoom_range(self):
        return self._session.get_sheet_zoom_range()

    def set_column_width(self, width):
        old_width = self._session.get_sheet_column_width()
        self._session.set_sheet_column_width(width)
        if self._session.get_sheet_column_width() != old_width:
            self._updater.signal_update(set(['signal_sheet_column_width']))

    def set_column_width_range(self, minimum, maximum):
        old_width = self._session.get_sheet_column_width()
        self._session.set_sheet_column_width_range(minimum, maximum)
        if self._session.get_sheet_column_width() != old_width:
            self._updater.signal_update(set(['signal_sheet_column_width']))

    def get_column_width(self):
        return self._session.get_sheet_column_width()

    def get_column_width_range(self):
        return self._session.get_sheet_column_width_range()

    def set_edit_mode(self, enabled):
        self._session.set_edit_mode(enabled)
        self._updater.signal_update(set(['signal_edit_mode']))

    def get_edit_mode(self):
        return self._session.get_edit_mode()

    def set_typewriter_connected(self, connected):
        self._session.set_typewriter_connected(connected)
        self._updater.signal_update(set(['signal_edit_mode']))
        if not connected and self.get_replace_mode():
            self.set_replace_mode(False)

    def get_typewriter_connected(self):
        return self._session.get_typewriter_connected()

    def is_editing_enabled(self):
        return self.get_edit_mode() and self.get_typewriter_connected()

    def set_replace_mode(self, enabled):
        self._session.set_replace_mode(enabled)
        self._updater.signal_update(set(['signal_replace_mode']))
        if enabled and not self.get_typewriter_connected():
            self.set_typewriter_connected(True)

    def get_replace_mode(self):
        return self._session.get_replace_mode()

    def set_grid_enabled(self, enabled):
        self._session.set_grid_enabled(enabled)

    def is_grid_enabled(self):
        return self._session.is_grid_enabled()

    def get_grid(self):
        grid = Grid()
        grid.set_controller(self._controller)
        grid.set_ui_model(self._ui_model)
        return grid


