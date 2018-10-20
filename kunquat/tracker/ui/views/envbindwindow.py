# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .bindeditor import BindEditor
from .environmenteditor import EnvironmentEditor
from .saverwindow import SaverWindow
from .updater import Updater
from .utils import get_abs_window_size


class EnvBindWindow(Updater, SaverWindow):

    def __init__(self):
        super().__init__()
        self._bind_editor = BindEditor()
        self._env_editor = EnvironmentEditor()

        self.add_to_updaters(self._bind_editor, self._env_editor)

        self.setWindowTitle('Environment & bindings')

        h = QHBoxLayout()
        h.setContentsMargins(4, 4, 4, 4)
        h.setSpacing(4)
        h.addWidget(self._env_editor)
        h.addWidget(self._bind_editor)
        self.setLayout(h)

    def closeEvent(self, event):
        event.ignore()
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.hide_env_and_bindings()

    def sizeHint(self):
        return get_abs_window_size(0.5, 0.7)


