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

from .bindeditor import BindEditor
from .environmenteditor import EnvironmentEditor


class EnvBindWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._bind_editor = BindEditor()
        self._env_editor = EnvironmentEditor()

        self.setWindowTitle('Environment & bindings')

        h = QHBoxLayout()
        h.setMargin(4)
        h.setSpacing(4)
        h.addWidget(self._env_editor)
        h.addWidget(self._bind_editor)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._env_editor.set_ui_model(ui_model)
        self._bind_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._env_editor.unregister_updaters()
        self._bind_editor.unregister_updaters()

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_env_and_bindings()

    def sizeHint(self):
        return QSize(1024, 600)


