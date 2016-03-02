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


class SheetHistory():

    def __init__(self):
        self._controller = None
        self._session = None
        self._store = None
        self._ui_model = None

        self._cur_step = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._store = controller.get_store()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def commit(self):
        if self._cur_step:
            past = self._session.get_sheet_past()
            past.append(self._cur_step)
            self._cur_step = None

    def add_step(self, transaction, location, commit=True):
        if not self._cur_step:
            self._cur_step = Step(self._store, self._ui_model)
        self._cur_step.add_transaction(transaction, location)

        # Clear our future
        future = self._session.get_sheet_future()
        future *= 0

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
        assert not self._cur_step

        next_entry = future.pop()
        next_entry.apply_new_data()
        past = self._session.get_sheet_past()
        past.append(next_entry)

    def has_past_changes(self):
        return bool(self._session.get_sheet_past())

    def has_future_changes(self):
        return bool(self._session.get_sheet_future())

    def remove_pattern_changes(self, pattern_num):
        prefix = 'pat_{:03x}'.format(pattern_num)

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

    def add_transaction(self, transaction, location):
        for k in transaction.iterkeys():
            if k not in self._old_data:
                self._old_data[k] = self._store.get(k, None)
        self._new_data.update(transaction)
        assert set(self._old_data.keys()) == set(self._new_data.keys())

        if not self._location:
            self._location = location

    def apply_old_data(self):
        for k, v in self._old_data.iteritems():
            self._store[k] = v

        selection = self._ui_model.get_selection()
        selection.set_location(self._location)

    def apply_new_data(self):
        for k, v in self._new_data.iteritems():
            self._store[k] = v

        selection = self._ui_model.get_selection()
        selection.set_location(self._location)

    def has_changes_with_prefix(self, prefix):
        return any(k.startswith(prefix) for k in self._new_data.iterkeys())


