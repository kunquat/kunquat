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
        # Note playback
        button = self._get_typewriter_button_model(event)
        if button and event.modifiers() == Qt.NoModifier:
            if (not self._sheet_mgr.allow_note_autorepeat()) and event.isAutoRepeat():
                return True

            if event.type() == QEvent.KeyPress:
                button.start_tracked_note()
            elif event.type() == QEvent.KeyRelease:
                button.stop_tracked_note()

            return True

        # Octave selection and hit keymap toggle
        if self.is_octave_down(event):
            if event.type() == QEvent.KeyPress:
                cur_octave = self._typewriter_mgr.get_octave()
                self._typewriter_mgr.set_octave(max(0, cur_octave - 1))
            return True
        elif self.is_octave_up(event):
            if event.type() == QEvent.KeyPress:
                cur_octave = self._typewriter_mgr.get_octave()
                octave_count = self._typewriter_mgr.get_octave_count()
                self._typewriter_mgr.set_octave(min(octave_count - 1, cur_octave + 1))
            return True
        elif self.is_hit_keymap_toggle(event):
            if event.type() == QEvent.KeyPress:
                is_hit_keymap_active = self._keymap_mgr.is_hit_keymap_active()
                self._keymap_mgr.set_hit_keymap_active(not is_hit_keymap_active)
                self._updater.signal_update('signal_select_keymap')
            return True

        return False

    def _get_key_action(self, event):
        if sys.platform.startswith('darwin'):
            pass # TODO
        else:
            code = event.nativeScanCode()
            loc = self._keymap_mgr.get_scancode_location(code)
            if loc:
                return self._keymap_mgr.get_key_action(loc)

        return None

    def _get_typewriter_button_model(self, event):
        if sys.platform.startswith('darwin'):
            typewriter_map = _TYPEWRITER_KEYCODE_MAP
            code = event.key()
            try:
                row, index = typewriter_map[code]
            except KeyError:
                return None
        else:
            code = event.nativeScanCode()
            loc = self._keymap_mgr.get_scancode_location(code)
            if not loc:
                return None
            action = self._keymap_mgr.get_key_action(loc)
            if not isinstance(action, KeyboardNoteAction):
                return None
            row, index = action.row, action.index

        button = self._typewriter_mgr.get_button_model(row, index)
        return button

    def is_octave_up(self, event):
        action = self._get_key_action(event)
        return action and (action.action_type == KeyboardAction.OCTAVE_UP)

    def is_octave_down(self, event):
        action = self._get_key_action(event)
        return action and (action.action_type == KeyboardAction.OCTAVE_DOWN)

    def is_hit_keymap_toggle(self, event):
        return (event.key() == Qt.Key_H) and (event.modifiers() == Qt.ControlModifier)

    def is_handled_key(self, event):
        button = self._get_typewriter_button_model(event)
        return (bool(button) or
                self.is_octave_up(event) or
                self.is_octave_down(event) or
                self.is_hit_keymap_toggle(event))


