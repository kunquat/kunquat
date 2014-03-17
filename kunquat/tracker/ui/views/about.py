# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
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

from renderstats import RenderStats
from logo import Logo


class AboutMessage(QLabel):

    def __init__(self):
        QLabel.__init__(self)
        self.setTextFormat(Qt.RichText)

        contents = """
            <h1>Kunquat Tracker</h1>
            """
        self.setText(contents)


class About(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None

        self._logo = Logo()
        self._render_stats = RenderStats()

        v = QVBoxLayout()
        v.addWidget(self._logo)
        v.addWidget(AboutMessage())
        v.addWidget(self._render_stats)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._logo.set_ui_model(ui_model)
        self._render_stats.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._logo.unregister_updaters()
        self._render_stats.unregister_updaters()


