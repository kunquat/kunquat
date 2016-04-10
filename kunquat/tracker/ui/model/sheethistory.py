# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .triggerposition import TriggerPosition


class SheetHistory():

    def __init__(self):
        self._controller = None
        self._session = None
        self._store = None
        self._ui_model = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._store = controller.get_store()

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
            cur_step = Step(self._store, self._ui_model)
            self._session.set_sheet_cur_step(cur_step)
        cur_step.add_transaction(transaction, location)

        # Clear our future
        future = self._session.get_sheet_future()
        future[:] = []

        if commit:
            self.commit()

    def undo(self):
        self.commit()

        past = self._session.get_sheet_past()
        if not past:
            return

        prev_entry = past.pop()
        prev_entry.apply_old_data()
        future = self._session.get_sheet_future()
        future.append(prev_entry)

    def redo(self):
        future = self._session.get_sheet_future()
        if not future:
            return
        assert not self._session.get_sheet_cur_step()

        next_entry = future.pop()
        next_entry.apply_new_data()
        past = self._session.get_sheet_past()
        past.append(next_entry)

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

    def __init__(self, store, ui_model):
        self._store = store
        self._ui_model = ui_model

        self._old_data = {}
        self._new_data = {}
        self._location = None
        self._pinst = None

    def add_transaction(self, transaction, location):
        for k in transaction.iterkeys():
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

    def apply_old_data(self):
        for k, v in self._old_data.iteritems():
            self._store[k] = v

        new_location = self._get_new_location()
        if new_location:
            selection = self._ui_model.get_selection()
            selection.set_location(new_location)

    def apply_new_data(self):
        for k, v in self._new_data.iteritems():
            self._store[k] = v

        new_location = self._get_new_location()
        if new_location:
            selection = self._ui_model.get_selection()
            selection.set_location(new_location)

    def has_changes_with_prefix(self, prefix):
        return any(k.startswith(prefix) for k in self._new_data.iterkeys())


