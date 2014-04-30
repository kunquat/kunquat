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

from trigger import Trigger
from triggerposition import TriggerPosition


COLUMN_COUNT = 64 # TODO: define in kunquat interface


class TypewriterButtonModel():

    def __init__(self, row, index):
        self._controller = None
        self._session = None
        self._ui_model = None
        self._ui_manager = None
        self._typewriter_manager = None
        self._notation_manager = None
        self._sheet_manager = None

        self._row = row
        self._index = index

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._ui_manager = ui_model.get_ui_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()
        self._notation_manager = ui_model.get_notation_manager()
        self._sheet_manager = ui_model.get_sheet_manager()

    def get_name(self):
        pitch = self.get_pitch()
        if pitch == None:
            return None

        notation = self._notation_manager.get_notation()
        name = notation.get_full_name(pitch)
        return name

    def get_pitch(self):
        return self._typewriter_manager.get_button_pitch((self._row, self._index))

    def get_led_state(self):
        selected_control = self._ui_manager.get_selected_control()
        if selected_control == None:
            return None

        pitch = self.get_pitch()
        if pitch == None:
            return None

        (left_on, center_on, right_on) = 3 * [False]
        notes = selected_control.get_active_notes()
        for note in notes.itervalues():
            if self._typewriter_manager.get_closest_keymap_pitch(note) == pitch:
                if note < pitch:
                    left_on = True
                elif note == pitch:
                    center_on = True
                elif note > pitch:
                    right_on = True
                else:
                    assert False

        return (left_on, center_on, right_on)

    def start_tracked_note(self):
        pitch = self.get_pitch()
        if pitch == None:
            return

        if self._session.is_key_active(self._row, self._index):
            return

        selected_control = self._ui_manager.get_selected_control()
        if selected_control:
            note = selected_control.start_tracked_note(0, pitch)
            self._session.activate_key_with_note(self._row, self._index, note)

        if self._sheet_manager.is_editing_enabled():
            selection = self._ui_model.get_selection()
            orig_location = selection.get_location()

            trigger = Trigger('n+', unicode(pitch))
            self._sheet_manager.add_trigger(trigger)

            if self._session.get_chord_start() == None:
                self._session.set_chord_start(orig_location)

            cur_col_num = orig_location.get_col_num()
            if cur_col_num < COLUMN_COUNT - 1:
                new_col_num = cur_col_num + 1
                new_location = TriggerPosition(
                        orig_location.get_track(),
                        orig_location.get_system(),
                        new_col_num,
                        orig_location.get_row_ts(),
                        orig_location.get_trigger_index())
                selection.set_location(new_location)

    def stop_tracked_note(self):
        if not self._session.is_key_active(self._row, self._index):
            return

        note = self._session.get_active_note(self._row, self._index)
        note.set_rest()
        self._session.deactivate_key(self._row, self._index)

        chord_start = self._session.get_chord_start()
        if chord_start != None:
            selection = self._ui_model.get_selection()
            cur_location = selection.get_location()
            if cur_location.get_col_num() == chord_start.get_col_num():
                new_location = cur_location
            else:
                new_trigger_index = chord_start.get_trigger_index() + 1
                new_location = TriggerPosition(
                        chord_start.get_track(),
                        chord_start.get_system(),
                        chord_start.get_col_num(),
                        chord_start.get_row_ts(),
                        new_trigger_index)

            selection.set_location(new_location)
            self._session.set_chord_start(None)


