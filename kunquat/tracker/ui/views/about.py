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
from PyQt4.QtSvg import QSvgRenderer

from renderstats import RenderStats


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

        self._logo = QLabel()
        self._logo.setGeometry(10, 10, 200, 100)

        self._render_stats = RenderStats()

        v = QVBoxLayout()
        v.addWidget(self._logo)
        v.addWidget(AboutMessage())
        v.addWidget(self._render_stats)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._render_stats.set_ui_model(ui_model)
        self._update_logo()

    def _update_logo(self):
        icon_bank = self._ui_model.get_icon_bank()
        logo_path = icon_bank.get_kunquat_logo_path()
        logo_image = QPixmap(200, 200)
        background_color = QColor(0, 0, 0, 0)
        logo_image.fill(background_color)
        logo_painter = QPainter(logo_image)
        logo_renderer = QSvgRenderer(logo_path)
        logo_renderer.render(logo_painter)
        self._logo.setPixmap(logo_image)

    def unregister_updaters(self):
        self._render_stats.unregister_updaters()


