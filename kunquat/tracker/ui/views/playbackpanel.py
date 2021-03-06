# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

import kunquat.tracker.cmdline as cmdline
from kunquat.tracker.ui.model.keymapmanager import KeyboardAction
from .iconbutton import IconButton
from .playbackposition import PlaybackPosition
from .updater import Updater


class PlaybackPanel(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._play_button = PlayButton()
        self._play_pattern_button = PlayPatternButton()
        self._play_from_cursor_button = PlayFromCursorButton()
        self._record_button = RecordButton()
        self._silence_button = SilenceButton()
        self._playback_pos = PlaybackPosition()
        self._interactivity_button = InteractivityButton()

        self.add_to_updaters(
                self._play_button,
                self._play_pattern_button,
                self._play_from_cursor_button,
                self._record_button,
                self._silence_button,
                self._playback_pos,
                self._interactivity_button)

        self.addWidget(self._play_button)
        self.addWidget(self._play_pattern_button)
        self.addWidget(self._play_from_cursor_button)
        if cmdline.get_experimental():
            self.addWidget(self._record_button)
        self.addWidget(self._silence_button)
        self.addSeparator()
        self.addWidget(self._playback_pos)
        self.addSeparator()
        self.addWidget(self._interactivity_button)

    def addWidget(self, widget):
        if isinstance(widget, QToolButton):
            widget.setFocusPolicy(Qt.NoFocus)
        super().addWidget(widget)


class PlayButton(IconButton):

    def __init__(self):
        super().__init__(flat=True)
        self.setToolTip('Play')

    def _on_setup(self):
        super()._on_setup()
        self.set_icon('play')

        keymap_mgr = self._ui_model.get_keymap_manager()
        key_name = keymap_mgr.get_key_name(
                keymap_mgr.get_action_location(KeyboardAction.PLAY))
        if key_name:
            self.setToolTip('Play ({})'.format(key_name))

        self.clicked.connect(self._ui_model.play)


class PlayPatternButton(IconButton):

    def __init__(self):
        super().__init__(flat=True)
        self.setToolTip('Play Pattern')

    def _on_setup(self):
        super()._on_setup()
        self.set_icon('play_pattern')

        keymap_mgr = self._ui_model.get_keymap_manager()
        key_name = keymap_mgr.get_key_name(
                keymap_mgr.get_action_location(KeyboardAction.PLAY))
        if key_name:
            self.setToolTip('Play Pattern (Ctrl + {})'.format(key_name))

        self.clicked.connect(self._ui_model.play_pattern)


class PlayFromCursorButton(IconButton):

    def __init__(self):
        super().__init__(flat=True)
        self.setToolTip('Play from Cursor')

    def _on_setup(self):
        super()._on_setup()
        self.set_icon('play_from_cursor')

        keymap_mgr = self._ui_model.get_keymap_manager()
        key_name = keymap_mgr.get_key_name(
                keymap_mgr.get_action_location(KeyboardAction.PLAY))
        if key_name:
            self.setToolTip('Play from Cursor (Alt + {})'.format(key_name))

        self.clicked.connect(self._ui_model.play_from_cursor)


class RecordButton(IconButton):

    def __init__(self):
        super().__init__(flat=True)
        self._sheet_mgr = None
        self._playback_mgr = None

        self.setCheckable(True)

    def _on_setup(self):
        super()._on_setup()

        self.set_icon('record')

        self.register_action('signal_record_mode', self._update_checked)

        self._sheet_mgr = self._ui_model.get_sheet_manager()
        self._playback_mgr = self._ui_model.get_playback_manager()

        self.clicked.connect(self._clicked)

    def _update_checked(self):
        old_block = self.blockSignals(True)
        is_checked = self._playback_mgr.is_recording()
        self.setChecked(is_checked)
        self.blockSignals(old_block)

    def _clicked(self):
        if self._playback_mgr.is_recording():
            self._playback_mgr.stop_recording()
        else:
            self._playback_mgr.start_recording()
            self._sheet_mgr.set_typewriter_connected(True)
            self._ui_model.play()


class SilenceButton(IconButton):

    def __init__(self):
        super().__init__(flat=True)
        self._playback_mgr = None

        self.setToolTip('Silence')

    def _on_setup(self):
        super()._on_setup()
        self._playback_mgr = self._ui_model.get_playback_manager()

        self.set_icon('silence')

        keymap_mgr = self._ui_model.get_keymap_manager()
        key_name = keymap_mgr.get_key_name(
                keymap_mgr.get_action_location(KeyboardAction.SILENCE))
        if key_name:
            self.setToolTip('Silence ({})'.format(key_name))

        self.clicked.connect(self._clicked)

    def _clicked(self):
        self._playback_mgr.stop_recording()
        self._ui_model.silence()


class InteractivityButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Interactivity')

    def _on_setup(self):
        self.clicked.connect(self._show_ia_window)

    def _show_ia_window(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.show_interactivity_controls()


