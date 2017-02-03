# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
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

import kunquat.tracker.cmdline as cmdline
from .playbackposition import PlaybackPosition
from .playbutton import PlayButton
from .playpatternbutton import PlayPatternButton
from .recordbutton import RecordButton
from .silencebutton import SilenceButton
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


class PlayFromCursorButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Play from Cursor')
        self.setToolTip('Play from Cursor (Alt + Comma)')
        self.setAutoRaise(True)

    def _on_setup(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('play_from_cursor')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        QObject.connect(self, SIGNAL('clicked()'), self._ui_model.play_from_cursor)


class InteractivityButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Interactivity')

    def _on_setup(self):
        QObject.connect(self, SIGNAL('clicked()'), self._show_ia_window)

    def _show_ia_window(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_interactivity_controls()


