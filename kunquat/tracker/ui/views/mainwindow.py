# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
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
from mainview import MainView


class MainWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self.setWindowTitle('Kunquat Tracker')
        self._ui_model = None

        self._main_view = MainView()
        layout = QVBoxLayout()
        layout.addWidget(self._main_view)
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)

        self.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._main_view.set_ui_model(ui_model)
        self.update_icon()

    def update_icon(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_kunquat_logo_path()
        icon = QIcon(icon_path)
        self.setWindowIcon(icon)

    def unregister_updaters(self):
        self._main_view.unregister_updaters()

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_main()

