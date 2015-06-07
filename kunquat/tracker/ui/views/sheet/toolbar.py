# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
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
from editbutton import EditButton
from replacebutton import ReplaceButton
from restbutton import RestButton
from delselectionbutton import DelSelectionButton
from zoombutton import ZoomButton
from lengtheditor import LengthEditor


class Toolbar(QWidget):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None

        self._edit_button = EditButton()
        self._replace_button = ReplaceButton()
        self._rest_button = RestButton()
        self._del_selection_button = DelSelectionButton()
        self._zoom_buttons = [
                ZoomButton('out'),
                ZoomButton('original'),
                ZoomButton('in'),
            ]
        self._length_editor = LengthEditor()

        if cmdline.get_experimental():
            self._zoom_buttons.extend([
                    ZoomButton('shrink_w'),
                    ZoomButton('original_w'),
                    ZoomButton('expand_w'),
                ])

        h = QHBoxLayout()
        h.setContentsMargins(4, 0, 4, 4)
        h.setSpacing(5)

        h.addWidget(self._edit_button)
        h.addWidget(self._replace_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._rest_button)
        if cmdline.get_experimental():
            h.addWidget(self._del_selection_button)
        h.addWidget(HackSeparator())
        for button in self._zoom_buttons:
            h.addWidget(button)

        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        h.addWidget(spacer)

        h.addWidget(self._length_editor)

        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._edit_button.set_ui_model(ui_model)
        self._replace_button.set_ui_model(ui_model)
        self._rest_button.set_ui_model(ui_model)
        self._del_selection_button.set_ui_model(ui_model)
        for button in self._zoom_buttons:
            button.set_ui_model(ui_model)
        self._length_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._edit_button.unregister_updaters()
        self._replace_button.unregister_updaters()
        self._rest_button.unregister_updaters()
        self._del_selection_button.unregister_updaters()
        for button in self._zoom_buttons:
            button.unregister_updaters()
        self._length_editor.unregister_updaters()


class HackSeparator(QFrame):

    def __init__(self):
        QFrame.__init__(self)
        self.setFrameShape(QFrame.VLine)
        self.setFrameShadow(QFrame.Sunken)


