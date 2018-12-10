# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import re

from kunquat.kunquat.limits import *
from .sheetmanager import SheetManager
from .triggerposition import TriggerPosition


class SheetHistory():

    def __init__(self):
        self._controller = None
        self._session = None
        self._store = None
        self._ui_model = None
        self._updater = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._store = controller.get_store()
        self._updater = controller.get_updater()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def commit(self):
        cur_step = self._session.get_sheet_cur_step()
        if cur_step:
            past = self._session.get_sheet_past()
            past.append(cur_step)
            self._session.set_sheet_cur_step(None)

    def add_step(self, transaction, location, commit=True):
        cur_step = self._session.get_sheet_cur_step()
        if not cur_step:
            cur_step = Step(self._store, self._session, self._ui_model)
            self._session.set_sheet_cur_step(cur_step)
        cur_step.add_transaction(transaction, location)

        # Clear our future
        future = self._session.get_sheet_future()
        future[:] = []

        if commit:
            self.commit()

    def _allow_sheet_changes(self):
        playback_mgr = self._ui_model.get_playback_manager()
        return (not playback_mgr.follow_playback_cursor() or playback_mgr.is_recording())

    def undo(self):
        if not self._allow_sheet_changes():
            return

        self.commit()

        past = self._session.get_sheet_past()
        if not past:
            return

        prev_entry = past.pop()
        prev_entry.apply_old_data()
        future = self._session.get_sheet_future()
        future.append(prev_entry)

        self._ui_model.get_sheet_manager().flush_latest_column()
        self._updater.signal_update('signal_sheet_undo')

    def redo(self):
        if not self._allow_sheet_changes():
            return

        future = self._session.get_sheet_future()
        if not future:
            return
        assert not self._session.get_sheet_cur_step()

        next_entry = future.pop()
        next_entry.apply_new_data()
        past = self._session.get_sheet_past()
        past.append(next_entry)

        self._ui_model.get_sheet_manager().flush_latest_column()
        self._updater.signal_update('signal_sheet_redo')

    def has_past_changes(self):
        return bool(self._session.get_sheet_past() or self._session.get_sheet_cur_step())

    def has_future_changes(self):
        return bool(self._session.get_sheet_future())

    def remove_pattern_changes(self, pattern):
        prefix = pattern.get_id()

        past = self._session.get_sheet_past()
        new_past = [s for s in past if not s.has_changes_with_prefix(prefix)]
        past[:] = new_past

        future = self._session.get_sheet_future()
        new_future = [s for s in future if not s.has_changes_with_prefix(prefix)]
        future[:] = new_future


class Step():

    def __init__(self, store, session, ui_model):
        self._store = store
        self._session = session
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        self._old_data = {}
        self._new_data = {}
        self._location = None
        self._pinst = None

    def add_transaction(self, transaction, location):
        for k in transaction.keys():
            if k not in self._old_data:
                self._old_data[k] = self._store.get(k, None)
        self._new_data.update(transaction)
        assert set(self._old_data.keys()) == set(self._new_data.keys())

        if (not self._location) and location:
            self._location = location

            # Find our pattern instance in case it gets moved later
            album = self._ui_model.get_module().get_album()
            assert album
            song = album.get_song_by_track(location.get_track())
            assert song
            self._pinst = song.get_pattern_instance(location.get_system())

    def _get_new_location(self):
        if not self._location:
            return None

        album = self._ui_model.get_module().get_album()
        if not album:
            return None
        pinst_loc = album.get_pattern_instance_location(self._pinst)
        if not pinst_loc:
            return None
        track, system = pinst_loc

        return TriggerPosition(
                track,
                system,
                self._location.get_col_num(),
                self._location.get_row_ts(),
                self._location.get_trigger_index())

    def _signal_change(self, key):
        if re.match('pat_[0-9a-f]{3}/col_[0-9a-f]{2}/p_triggers.json', key):
            parts = key.split('/')

            album = self._ui_model.get_module().get_album()
            if album.get_existence():
                pat_num = int(parts[0].split('_')[1], 16)
                col_num = int(parts[1].split('_')[1], 16)

                for i in range(PAT_INSTANCES_MAX):
                    key = '{}/instance_{:03x}/p_manifest.json'.format(parts[0], i)
                    if key in self._store.keys():
                        inst_num = i
                        break
                else:
                    return

                track_num, system_num = album.get_pattern_instance_location_by_nums(
                        pat_num, inst_num)

                self._session.add_column_update(track_num, system_num, col_num)
                self._updater.signal_update('signal_column_updated')
                return

        elif re.match('pat_[0-9a-f]{3}/col_[0-9a-f]{2}/i_overlay_grids.json', key):
            self._updater.signal_update('signal_grid')
            return

        elif re.match('pat_[0-9a-f]{3}/p_length.json', key):
            self._updater.signal_update('signal_pattern_length')
            return

        elif re.match('pat_[0-9a-f]{3}/i_base_grid.json', key):
            self._updater.signal_update('signal_grid')
            return

        assert False

    def apply_old_data(self):
        for k, v in self._old_data.items():
            self._store[k] = v
            self._signal_change(k)

        new_location = self._get_new_location()
        if new_location:
            selection = self._ui_model.get_selection()
            selection.set_location(new_location)

    def apply_new_data(self):
        for k, v in self._new_data.items():
            self._store[k] = v
            self._signal_change(k)

        new_location = self._get_new_location()
        if new_location:
            selection = self._ui_model.get_selection()
            selection.set_location(new_location)

    def has_changes_with_prefix(self, prefix):
        return any(k.startswith(prefix) for k in self._new_data.keys())


