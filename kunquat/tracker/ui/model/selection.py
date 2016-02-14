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

from triggerposition import TriggerPosition
import tstamp


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

        self._updater.signal_update(set(['signal_selection']))

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


