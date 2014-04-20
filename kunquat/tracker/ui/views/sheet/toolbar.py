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
from restbutton import RestButton
from delselectionbutton import DelSelectionButton
from zoombutton import ZoomButton


class Toolbar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._rest_button = RestButton()
        self._del_selection_button = DelSelectionButton()
        self._zoom_buttons = [
                ZoomButton('out'),
                ZoomButton('original'),
                ZoomButton('in'),
            ]

        if cmdline.get_experimental():
            self._zoom_buttons.extend([
                    ZoomButton('shrink_w'),
                    ZoomButton('original_w'),
                    ZoomButton('expand_w'),
                ])

        self.addWidget(self._rest_button)
        self.addSeparator()
        self.addWidget(self._del_selection_button)
        self.addSeparator()
        for button in self._zoom_buttons:
            self.addWidget(button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._rest_button.set_ui_model(ui_model)
        self._del_selection_button.set_ui_model(ui_model)
        for button in self._zoom_buttons:
            button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._rest_button.unregister_updaters()
        self._del_selection_button.unregister_updaters()
        for button in self._zoom_buttons:
            button.unregister_updaters()


