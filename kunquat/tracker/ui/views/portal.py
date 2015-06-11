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

from newbutton import NewButton
from openbutton import OpenButton
from savebutton import SaveButton
from connectionsbutton import ConnectionsButton
from orderlistbutton import OrderlistButton
from eventlistbutton import EventListButton
from aboutbutton import AboutButton


class Portal(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._new_button = NewButton()
        self._open_button = OpenButton()
        self._save_button = SaveButton()
        self._about_button = AboutButton()
        self._connections_button = ConnectionsButton()
        self._orderlist_button = OrderlistButton()
        self._ch_defaults_button = ChDefaultsButton()
        self._event_list_button = EventListButton()

        self.addWidget(self._new_button)
        self.addWidget(self._open_button)
        self.addWidget(self._save_button)
        self.addSeparator()
        self.addWidget(self._connections_button)
        self.addWidget(self._orderlist_button)
        self.addWidget(self._ch_defaults_button)
        self.addWidget(self._event_list_button)
        self.addSeparator()
        self.addWidget(self._about_button)

    def set_ui_model(self, ui_model):
        self._new_button.set_ui_model(ui_model)
        self._open_button.set_ui_model(ui_model)
        self._save_button.set_ui_model(ui_model)
        self._connections_button.set_ui_model(ui_model)
        self._orderlist_button.set_ui_model(ui_model)
        self._ch_defaults_button.set_ui_model(ui_model)
        self._event_list_button.set_ui_model(ui_model)
        self._about_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._about_button.unregister_updaters()
        self._event_list_button.unregister_updaters()
        self._ch_defaults_button.unregister_updaters()
        self._orderlist_button.unregister_updaters()
        self._connections_button.unregister_updaters()
        self._save_button.unregister_updaters()
        self._open_button.unregister_updaters()
        self._new_button.unregister_updaters()


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


