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

from PySide.QtCore import *
from PySide.QtGui import *


# TODO: Define alternatives for different environments if/when needed
_TYPEWRITER_MAP = {}

_TYPEWRITER_MAP.update(dict((11 + x, (0, x)) for x in range(9)))
_TYPEWRITER_MAP.update(dict((24 + x, (1, x)) for x in range(10)))
_TYPEWRITER_MAP.update(dict((39 + x, (2, x)) for x in range(7)))
_TYPEWRITER_MAP.update(dict((52 + x, (3, x)) for x in range(7)))


class KeyboardMapper():

    def __init__(self):
        self._ui_model = None
        self._updater = None
        self._typewriter_manager = None
        self._typewriter_map = _TYPEWRITER_MAP

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._typewriter_manager = ui_model.get_typewriter_manager()

    def unregister_updaters(self):
        pass

    def process_typewriter_button_event(self, event):
        # Note playback
        scancode = event.nativeScanCode()
        button = self.get_typewriter_button_model(scancode)
        if button and event.modifiers() == Qt.NoModifier:
            if event.isAutoRepeat():
                return True
            if event.type() == QEvent.KeyPress:
                button.start_tracked_note()
            elif event.type() == QEvent.KeyRelease:
                button.stop_tracked_note()
            return True

        # Octave selection and hit keymap toggle
        if self.is_octave_down(event):
            if event.type() == QEvent.KeyPress:
                cur_octave = self._typewriter_manager.get_octave()
                self._typewriter_manager.set_octave(max(0, cur_octave - 1))
            return True
        elif self.is_octave_up(event):
            if event.type() == QEvent.KeyPress:
                cur_octave = self._typewriter_manager.get_octave()
                octave_count = self._typewriter_manager.get_octave_count()
                self._typewriter_manager.set_octave(min(octave_count - 1, cur_octave + 1))
            return True
        elif self.is_hit_keymap_toggle(event):
            if event.type() == QEvent.KeyPress:
                keymap_manager = self._ui_model.get_keymap_manager()
                is_hit_keymap_active = keymap_manager.is_hit_keymap_active()
                keymap_manager.set_hit_keymap_active(not is_hit_keymap_active)
                self._updater.signal_update('signal_select_keymap')
            return True
        return False

    def get_typewriter_button_model(self, scancode):
        try:
            row, index = self._typewriter_map[scancode]
        except KeyError:
            return None

        button = self._typewriter_manager.get_button_model(row, index)
        return button

    def is_octave_up(self, event):
        return event.key() == Qt.Key_O and event.modifiers() == Qt.ControlModifier

    def is_octave_down(self, event):
        return event.key() == Qt.Key_O and (
                event.modifiers() == (Qt.ControlModifier | Qt.ShiftModifier))

    def is_hit_keymap_toggle(self, event):
        return (event.key() == Qt.Key_H) and (event.modifiers() == Qt.ControlModifier)

    def is_handled_key(self, event):
        scancode = event.nativeScanCode()
        button = self.get_typewriter_button_model(scancode)
        return (bool(button) or
                self.is_octave_up(event) or
                self.is_octave_down(event) or
                self.is_hit_keymap_toggle(event))


