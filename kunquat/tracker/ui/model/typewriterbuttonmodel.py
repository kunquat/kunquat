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

class TypewriterButtonModel():

    def __init__(self, row, index):
        self._controller = None
        self._ui_model = None
        self._ui_manager = None
        self._typewriter_manager = None
        self._notation_manager = None

        self._row = row
        self._index = index

    def set_controller(self, controller):
        self._controller = controller

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._ui_manager = ui_model.get_ui_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()
        self._notation_manager = ui_model.get_notation_manager()

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

    def press(self):
        pitch = self.get_pitch()
        if pitch == None:
            return

        selected_control = self._ui_manager.get_selected_control()
        selected_control.set_active_note(0, pitch)

    def release(self):
        selected_control = self._ui_manager.get_selected_control()
        selected_control.set_rest(0)


