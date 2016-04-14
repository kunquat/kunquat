# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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

from .notationeditor import NotationEditor


class NotationWindow(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self.setWindowTitle('Notations')

        self._editor = NotationEditor()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._editor.unregister_updaters()

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_notation_editor()

    def sizeHint(self):
        return QSize(1280, 768)


