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

from .renderstats import RenderStats
from .updater import Updater


class RenderStatsWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._render_stats = RenderStats()

        self.add_to_updaters(self._render_stats)

        self.setWindowTitle('System load')

        v = QVBoxLayout()
        v.addWidget(self._render_stats)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_render_stats()

    def sizeHint(self):
        return QSize(768, 384)


