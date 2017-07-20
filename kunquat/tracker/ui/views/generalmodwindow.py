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

from kunquat.tracker.ui.qt import *

from .generalmodeditor import GeneralModEditor
from .saverwindow import SaverWindow
from .updater import Updater


class GeneralModWindow(Updater, SaverWindow):

    def __init__(self):
        super().__init__()
        self.setWindowTitle('General module settings')

        self._editor = GeneralModEditor()
        self.add_to_updaters(self._editor)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._editor)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_general_module_settings()

    def sizeHint(self):
        return QSize(960, 768)


