# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import sys
import itertools

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.keymapmanager import KeyboardAction, KeyboardNoteAction
from .updater import Updater


# TODO: Define alternatives for different environments if/when needed

def _generate_keycode_map():
    # pylint: disable=line-too-long
    key_layout = [
        [Qt.Key_2, Qt.Key_3, Qt.Key_4, Qt.Key_5, Qt.Key_6, Qt.Key_7, Qt.Key_8, Qt.Key_9, Qt.Key_0],
        [Qt.Key_Q, Qt.Key_W, Qt.Key_E, Qt.Key_R, Qt.Key_T, Qt.Key_Y, Qt.Key_U, Qt.Key_I, Qt.Key_O, Qt.Key_P],
        [Qt.Key_S, Qt.Key_D, Qt.Key_F, Qt.Key_G, Qt.Key_H, Qt.Key_J, Qt.Key_K],
        [Qt.Key_Z, Qt.Key_X, Qt.Key_C, Qt.Key_V, Qt.Key_B, Qt.Key_N, Qt.Key_M],
    ]
    # pylint: enable=line-too-long
    for row_index, row in zip(itertools.count(), key_layout):
        for column_index, key in zip(itertools.count(), row):
            yield (key, (row_index, column_index))

_TYPEWRITER_KEYCODE_MAP = dict(_generate_keycode_map())


class KeyboardMapper(Updater):

    def __init__(self):
        super().__init__()
        self._sheet_mgr = None
        self._keymap_mgr = None
        self._typewriter_mgr = None

    def _on_setup(self):
        self._sheet_mgr = self._ui_model.get_sheet_manager()
        self._keymap_mgr = self._ui_model.get_keymap_manager()
        self._typewriter_mgr = self._ui_model.get_typewriter_manager()

    def process_typewriter_button_event(self, event):
        has_modifiers = (event.modifiers() != Qt.NoModifier)
        is_press = (event.type() == QEvent.KeyPress)
        is_release = (event.type() == QEvent.KeyRelease)

        # Note playback
        action = self.get_key_action(event)
        button = self._get_typewriter_button_model(action)
        if button and (not has_modifiers):
            if (not self._sheet_mgr.allow_note_autorepeat()) and event.isAutoRepeat():
                return True

            if is_press:
                button.start_tracked_note()
            elif is_release:
                button.stop_tracked_note()

            return True

        # Other actions that need to be handled
        if action:
            if action.action_type == KeyboardAction.OCTAVE_DOWN:
                if not has_modifiers:
                    if is_press:
                        cur_octave = self._typewriter_mgr.get_octave()
                        self._typewriter_mgr.set_octave(max(0, cur_octave - 1))
                    return True

            elif action.action_type == KeyboardAction.OCTAVE_UP:
                if not has_modifiers:
                    if is_press:
                        cur_octave = self._typewriter_mgr.get_octave()
                        octave_count = self._typewriter_mgr.get_octave_count()
                        self._typewriter_mgr.set_octave(
                                min(octave_count - 1, cur_octave + 1))
                    return True

            elif action.action_type == KeyboardAction.PLAY:
                if not has_modifiers:
                    if is_press:
                        self._ui_model.play()
                    return True
                elif (event.modifiers() == Qt.ControlModifier):
                    if is_press:
                        self._ui_model.play_pattern()
                    return True
                elif (event.modifiers() == Qt.AltModifier):
                    if is_press:
                        self._ui_model.play_from_cursor()
                    return True

            elif action.action_type == KeyboardAction.REST:
                pass # Skipped, only handled in the sheet view

            elif action.action_type == KeyboardAction.SILENCE:
                if not has_modifiers:
                    if is_press:
                        self._ui_model.silence()
                    return True

        if self.is_hit_keymap_toggle(event):
            if event.type() == QEvent.KeyPress:
                is_hit_keymap_active = self._keymap_mgr.is_hit_keymap_active()
                self._keymap_mgr.set_hit_keymap_active(not is_hit_keymap_active)
                self._updater.signal_update('signal_select_keymap')
            return True

        return False

    def get_key_action(self, event):
        if sys.platform.startswith('darwin'):
            pass # TODO
        else:
            code = event.nativeScanCode()
            loc = self._keymap_mgr.get_scancode_location(code)
            if loc:
                return self._keymap_mgr.get_key_action(loc)

        return None

    def _get_typewriter_button_model(self, action):
        if not isinstance(action, KeyboardNoteAction):
            return None

        row, index = action.row, action.index
        button = self._typewriter_mgr.get_button_model(row, index)
        return button

    def is_hit_keymap_toggle(self, event):
        return (event.key() == Qt.Key_H) and (event.modifiers() == Qt.ControlModifier)

    def is_handled_key(self, event):
        action = self.get_key_action(event)
        return (bool(action) or self.is_hit_keymap_toggle(event))


