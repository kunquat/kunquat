# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2016
#          Toni Ruottu, Finland 2014
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

from .about import About


class AboutWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._about = About()

        self.setWindowTitle('About Kunquat Tracker')

        v = QVBoxLayout()
        v.addWidget(self._about)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._about.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._about.unregister_updaters()

    def closeEvent(self, ev):
        ev.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_about()


