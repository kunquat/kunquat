# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
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

from .settings import Settings
from .updater import Updater


class SettingsWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self.setWindowTitle('Kunquat Tracker settings')

        self._settings = Settings()
        self.add_updating_child(self._settings)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._settings)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_settings()

    def sizeHint(self):
        return QSize(800, 512)


