# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2015
#          Toni Ruottu, Finland 2013-2014
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
from playbutton import PlayButton
from playpatternbutton import PlayPatternButton
from recordbutton import RecordButton
from silencebutton import SilenceButton
from eventlistbutton import EventListButton
from connectionsbutton import ConnectionsButton
from orderlistbutton import OrderlistButton
from notationselect import NotationSelect
from aboutbutton import AboutButton
from newbutton import NewButton
from openbutton import OpenButton
from savebutton import SaveButton


class Portal(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._play_button = PlayButton()
        self._play_pattern_button = PlayPatternButton()
        self._play_from_cursor_button = PlayFromCursorButton()
        self._record_button = RecordButton()
        self._silence_button = SilenceButton()
        self._event_list_button = EventListButton()
        self._connections_button = ConnectionsButton()
        self._orderlist_button = OrderlistButton()
        self._ch_defaults_button = ChDefaultsButton()
        self._notation_select = NotationSelect()
        self._about_button = AboutButton()
        self._new_button = NewButton()
        self._open_button = OpenButton()
        self._save_button = SaveButton()

        self.addWidget(self._play_button)
        self.addWidget(self._play_pattern_button)
        self.addWidget(self._play_from_cursor_button)
        if cmdline.get_experimental():
            self.addWidget(self._record_button)
        self.addWidget(self._silence_button)
        self.addSeparator()
        self.addWidget(self._event_list_button)
        self.addWidget(self._connections_button)
        self.addWidget(self._orderlist_button)
        self.addWidget(self._ch_defaults_button)
        self.addWidget(self._notation_select)
        self.addSeparator()
        self.addWidget(self._about_button)
        self.addSeparator()
        self.addWidget(self._new_button)
        self.addWidget(self._open_button)
        self.addWidget(self._save_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._play_button.set_ui_model(ui_model)
        self._play_pattern_button.set_ui_model(ui_model)
        self._play_from_cursor_button.set_ui_model(ui_model)
        self._record_button.set_ui_model(ui_model)
        self._silence_button.set_ui_model(ui_model)
        self._event_list_button.set_ui_model(ui_model)
        self._connections_button.set_ui_model(ui_model)
        self._orderlist_button.set_ui_model(ui_model)
        self._ch_defaults_button.set_ui_model(ui_model)
        self._notation_select.set_ui_model(ui_model)
        self._about_button.set_ui_model(ui_model)
        self._new_button.set_ui_model(ui_model)
        self._open_button.set_ui_model(ui_model)
        self._save_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._save_button.unregister_updaters()
        self._open_button.unregister_updaters()
        self._new_button.unregister_updaters()
        self._about_button.unregister_updaters()
        self._connections_button.unregister_updaters()
        self._orderlist_button.unregister_updaters()
        self._ch_defaults_button.unregister_updaters()
        self._notation_select.unregister_updaters()
        self._event_list_button.unregister_updaters()
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


class ChDefaultsButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Channel defaults')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_ch_defaults()


