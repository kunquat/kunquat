# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .triggerposition import TriggerPosition
from . import tstamp


class Selection():

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

    def set_location(self, trigger_position):
        self._session.set_selected_location(trigger_position)

        # Select default control ID of the new location
        sheet_manager = self._ui_model.get_sheet_manager()
        control_id = sheet_manager.get_inferred_active_control_id_at_location(
                self.get_location())
        control_manager = self._ui_model.get_control_manager()
        control_manager.set_selected_control_id(control_id)

        self._updater.signal_update('signal_selection')

    def get_location(self):
        location = self._session.get_selected_location()
        fallback = TriggerPosition(0, 0, 0, tstamp.Tstamp(0), 0)
        if not location:
            return fallback

        # Clamp to album limits
        module = self._ui_model.get_module()
        album = module.get_album()

        track = location.get_track()
        track_count = album.get_track_count()
        if track_count == 0:
            return fallback
        if track >= track_count:
            track = track_count - 1
            location.set_track(track)

        song = album.get_song_by_track(track)
        system = location.get_system()
        system_count = song.get_system_count()
        if system >= system_count:
            system = system_count - 1
            location.set_system(system)

        pinst = song.get_pattern_instance(system)
        pattern = pinst.get_pattern()
        row_ts = min(location.get_row_ts(), pattern.get_length())
        location.set_row_ts(row_ts)

        return location

    def _get_area_start(self):
        return self._session.get_selected_area_start()

    def _get_area_stop(self):
        return self._session.get_selected_area_stop()

    def try_set_area_start(self, location):
        if not self._get_area_start():
            self._session.set_selected_area_start(location)

    def set_area_stop(self, location):
        start = self._get_area_start()
        assert start
        assert start.get_track() == location.get_track()
        assert start.get_system() == location.get_system()

        self._session.set_selected_area_stop(location)

    def clear_area(self):
        self._session.set_selected_area_start(None)
        self._session.set_selected_area_stop(None)

    def has_area_start(self):
        return (self._get_area_start() != None)

    def get_area_start(self):
        assert self.has_area_start()
        return self._get_area_start()

    def has_area(self):
        start = self._get_area_start()
        stop = self._get_area_stop()
        return bool(start and stop and (start != stop))

    def has_trigger_row_slice(self):
        start = self._get_area_start()
        stop = self._get_area_stop()
        return (self.has_area() and
                (start.get_col_num() == stop.get_col_num()) and
                (start.get_row_ts() == stop.get_row_ts()))

    def has_rect_area(self):
        return self.has_area() and not self.has_trigger_row_slice()

    def get_area_top_left(self):
        assert self.has_area()
        start = self._get_area_start()
        stop = self._get_area_stop()

        col_num = min(start.get_col_num(), stop.get_col_num())
        row_ts = min(start.get_row_ts(), stop.get_row_ts())
        trigger_index = 0
        if self.has_trigger_row_slice():
            trigger_index = min(start.get_trigger_index(), stop.get_trigger_index())

        return TriggerPosition(
                start.get_track(), start.get_system(), col_num, row_ts, trigger_index)

    def get_area_bottom_right(self):
        assert self.has_area()
        start = self._get_area_start()
        stop = self._get_area_stop()

        col_num = max(start.get_col_num(), stop.get_col_num())
        row_ts = max(start.get_row_ts(), stop.get_row_ts())
        trigger_index = 0
        if self.has_trigger_row_slice():
            trigger_index = max(start.get_trigger_index(), stop.get_trigger_index())

        return TriggerPosition(
                start.get_track(), start.get_system(), col_num, row_ts, trigger_index)


