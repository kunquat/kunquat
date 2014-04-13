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

import kunquat.tracker.cmdline as cmdline
from delselectionbutton import DelSelectionButton
from zoombutton import ZoomButton


class Separator(QFrame):

    def __init__(self):
        QFrame.__init__(self)
        self.setFrameShape(QFrame.VLine)
        self.setFrameShadow(QFrame.Sunken)


class Toolbar(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._del_selection_button = DelSelectionButton()
        self._zoom_buttons = [
                ZoomButton('in'),
                ZoomButton('original'),
                ZoomButton('out')
            ]

        if cmdline.get_experimental():
            self._zoom_buttons.extend([
                    ZoomButton('expand_w'),
                    ZoomButton('original_w'),
                    ZoomButton('shrink_w'),
                ])

        h = QHBoxLayout()
        h.addWidget(self._del_selection_button)
        h.addWidget(Separator())
        for button in self._zoom_buttons:
            h.addWidget(button)
        h.addStretch(1)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._del_selection_button.set_ui_model(ui_model)
        for button in self._zoom_buttons:
            button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._del_selection_button.unregister_updaters()
        for button in self._zoom_buttons:
            button.unregister_updaters()


