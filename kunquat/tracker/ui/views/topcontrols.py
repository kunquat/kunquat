# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.tracker.cmdline as cmdline
from .playbackposition import PlaybackPosition
from .playbutton import PlayButton
from .playpatternbutton import PlayPatternButton
from .recordbutton import RecordButton
from .silencebutton import SilenceButton


class TopControls(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._play_button = PlayButton()
        self._play_pattern_button = PlayPatternButton()
        self._play_from_cursor_button = PlayFromCursorButton()
        self._record_button = RecordButton()
        self._silence_button = SilenceButton()
        self._playback_pos = PlaybackPosition()
        self._interactivity_button = InteractivityButton()

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

    def set_ui_model(self, ui_model):
        self._play_button.set_ui_model(ui_model)
        self._play_pattern_button.set_ui_model(ui_model)
        self._play_from_cursor_button.set_ui_model(ui_model)
        self._record_button.set_ui_model(ui_model)
        self._silence_button.set_ui_model(ui_model)
        self._playback_pos.set_ui_model(ui_model)
        self._interactivity_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._interactivity_button.unregister_updaters()
        self._playback_pos.unregister_updaters()
        self._silence_button.unregister_updaters()
        self._record_button.unregister_updaters()
        self._play_from_cursor_button.unregister_updaters()
        self._play_pattern_button.unregister_updaters()
        self._play_button.unregister_updaters()


class PlayFromCursorButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Play from Cursor')
        self.setToolTip('Play from Cursor (Alt + Comma)')
        self.setAutoRaise(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('play_from_cursor')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        QObject.connect(self, SIGNAL('clicked()'), self._ui_model.play_from_cursor)

    def unregister_updaters(self):
        pass


class InteractivityButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Interactivity')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        QObject.connect(self, SIGNAL('clicked()'), self._show_ia_window)

    def unregister_updaters(self):
        pass

    def _show_ia_window(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_interactivity_controls()


