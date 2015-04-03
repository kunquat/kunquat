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

import kunquat.kunquat.events as events
from trigger import Trigger
from triggerposition import TriggerPosition


class TypewriterButtonModel():

    def __init__(self, row, index):
        self._controller = None
        self._session = None
        self._ui_model = None
        self._control_manager = None
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
        self._control_manager = ui_model.get_control_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()
        self._notation_manager = ui_model.get_notation_manager()
        self._sheet_manager = ui_model.get_sheet_manager()

    def get_name(self):
        pitch = self.get_pitch()
        if pitch == None:
            return None

        notation = self._notation_manager.get_selected_notation()
        name = notation.get_full_name(pitch)
        return name

    def get_pitch(self):
        return self._typewriter_manager.get_button_pitch((self._row, self._index))

    def get_led_state(self):
        selected_control = self._control_manager.get_selected_control()
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

        selected_control = self._control_manager.get_selected_control()
        if selected_control:
            note = selected_control.start_tracked_note(0, pitch)
            self._session.activate_key_with_note(self._row, self._index, note)

        if self._sheet_manager.is_editing_enabled():
            self._sheet_manager.set_chord_mode(True)
            self._session.set_chord_note(pitch, True)

            if (self._sheet_manager.get_replace_mode() and
                    self._sheet_manager.is_at_trigger()):
                trigger = self._sheet_manager.get_selected_trigger()
                if trigger.get_argument_type() == events.EVENT_ARG_PITCH:
                    new_trigger = Trigger(trigger.get_type(), unicode(pitch))
                    self._sheet_manager.add_trigger(new_trigger)
            else:
                trigger = Trigger('n+', unicode(pitch))
                self._sheet_manager.add_trigger(trigger)

    def stop_tracked_note(self):
        if not self._session.is_key_active(self._row, self._index):
            return

        note = self._session.get_active_note(self._row, self._index)
        note.set_rest()
        self._session.deactivate_key(self._row, self._index)

        self._session.set_chord_note(self.get_pitch(), False)
        if not self._session.are_chord_notes_down():
            self._sheet_manager.set_chord_mode(False)


