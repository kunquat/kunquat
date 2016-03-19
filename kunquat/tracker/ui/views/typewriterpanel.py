# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
#          Toni Ruottu, Finland 2013-2014
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
from octaveselector import OctaveSelector
from typewriter import Typewriter
from keymapselect import KeymapSelect
from profilecontrol import ProfileControl


class TypewriterPanel(QFrame):

    def __init__(self):
        QFrame.__init__(self)
        self._ui_model = None
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()
        self._keymap_select = KeymapSelect()
        self._profile_control = ProfileControl()

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 0)
        v.setSpacing(2)
        v.addWidget(self._octave_selector)
        v.addWidget(self._keymap_select)
        v.addWidget(self._typewriter)
        self.setLayout(v)

        self._typewriter.setFocus()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._octave_selector.set_ui_model(ui_model)
        self._typewriter.set_ui_model(ui_model)
        self._keymap_select.set_ui_model(ui_model)
        self._profile_control.set_ui_model(ui_model)

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            if cmdline.get_experimental():
                self._profile_control.show()

    def unregister_updaters(self):
        self._keymap_select.unregister_updaters()
        self._typewriter.unregister_updaters()
        self._octave_selector.unregister_updaters()


