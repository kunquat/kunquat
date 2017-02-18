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

from kunquat.tracker.ui.views.saverwindow import SaverWindow
from kunquat.tracker.ui.views.updater import Updater
from .grideditor import GridEditor


class GridEditorWindow(Updater, SaverWindow):

    def __init__(self):
        super().__init__()
        self._grid_editor = GridEditor()

        self.add_to_updaters(self._grid_editor)

        self.setWindowTitle('Grid patterns')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._grid_editor)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_grid_editor()

    def sizeHint(self):
        return QSize(512, 768)


