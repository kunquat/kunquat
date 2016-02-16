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
from grid import Grid
from triggerposition import TriggerPosition
import tstamp


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
        self._ui_model = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._updater = controller.get_updater()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

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
                    for i in xrange(clamped_loc.get_trigger_index())]
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
            triggers = [column.get_trigger(row_ts, i) for i in xrange(tr_count)]
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

    def get_selected_trigger(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        cur_column = self.get_column_at_location(location)
        row_ts = location.get_row_ts()
        index = location.get_trigger_index()

        return cur_column.get_trigger(row_ts, index)

    def add_trigger(self, trigger):
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
            self.try_remove_trigger()
        cur_column.insert_trigger(row_ts, index, trigger)

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

        self._on_column_update(location)

    def try_remove_trigger(self):
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

        if cur_column.has_trigger(row_ts, index):
            cur_column.remove_trigger(row_ts, index)
            self._on_column_update(location)

    def _on_column_update(self, location):
        track_num = location.get_track()
        system_num = location.get_system()
        col_num = location.get_col_num()
        signal = SheetManager.encode_column_signal(track_num, system_num, col_num)

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


