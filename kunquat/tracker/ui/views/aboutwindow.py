# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2017
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .about import About
from .updater import Updater


class AboutWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._about = About()

        self.add_to_updaters(self._about)

        self.setWindowTitle('About Kunquat Tracker')

        v = QVBoxLayout()
        v.addWidget(self._about)
        self.setLayout(v)

    def closeEvent(self, ev):
        ev.ignore()
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.hide_about()


