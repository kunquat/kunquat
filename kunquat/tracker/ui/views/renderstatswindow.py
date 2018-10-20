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

from .renderstats import RenderStats
from .updater import Updater
from .utils import get_abs_window_size


class RenderStatsWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._render_stats = RenderStats()

        self.add_to_updaters(self._render_stats)

        self.setWindowTitle('System load')

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.addWidget(self._render_stats)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.hide_render_stats()

    def sizeHint(self):
        return get_abs_window_size(0.4, 0.5)


