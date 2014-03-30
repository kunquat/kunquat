# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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

from zoombutton import ZoomButton


class Toolbar(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._zoom_buttons = [
                ZoomButton('in'),
                ZoomButton('original'),
                ZoomButton('out')
            ]

        h = QHBoxLayout()
        for button in self._zoom_buttons:
            h.addWidget(button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        for button in self._zoom_buttons:
            button.set_ui_model(ui_model)

    def unregister_updaters(self):
        for button in self._zoom_buttons:
            button.unregister_updaters()


