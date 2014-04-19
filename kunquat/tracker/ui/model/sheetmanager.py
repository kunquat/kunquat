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

from triggerposition import TriggerPosition


class SheetManager():

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
            pattern = song.get_pattern_instance(location.get_system()).get_pattern()
            column = pattern.get_column(location.get_col_num())

            cached = self._session.get_last_column()
            if cached and (cached == column):
                return cached

            self._session.set_last_column(column)
            return column

        return None

    def insert_trigger(self, trigger):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            return

        cur_column = self.get_column_at_location(location)
        row_ts = location.get_row_ts()
        index = location.get_trigger_index()

        cur_column.insert_trigger(row_ts, index, trigger)

        new_location = TriggerPosition(
                location.get_track(),
                location.get_system(),
                location.get_col_num(),
                location.get_row_ts(),
                location.get_trigger_index() + 1)
        selection.set_location(new_location)

        self._on_column_update()

    def try_remove_trigger(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            return

        cur_column = self.get_column_at_location(location)
        row_ts = location.get_row_ts()
        index = location.get_trigger_index()

        if cur_column.has_trigger(row_ts, index):
            cur_column.remove_trigger(row_ts, index)
            self._on_column_update()

    def _on_column_update(self):
        self._updater.signal_update(set(['signal_module']))

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

    def set_focus(self, has_focus):
        self._session.set_sheet_focus(has_focus)

    def get_focus(self):
        self._session.get_sheet_focus()


