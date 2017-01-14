# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2017
#          Toni Ruottu, Finland 2013-2014
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

import kunquat.tracker.cmdline as cmdline
from .hitmaptoggle import HitMapToggle
from .octaveselector import OctaveSelector
from .typewriter import Typewriter
from .notationselect import NotationSelect
from .profilecontrol import ProfileControl


class TypewriterPanel(QFrame):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._notation_select = NotationSelect()
        self._hit_map_toggle = HitMapToggle()
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()
        self._profile_control = ProfileControl()

        il = QHBoxLayout()
        il.setContentsMargins(0, 0, 0, 0)
        il.setSpacing(4)
        il.addWidget(self._notation_select)
        il.addWidget(self._hit_map_toggle)
        il.addStretch(1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 0)
        v.setSpacing(2)
        v.addLayout(il)
        v.addWidget(self._octave_selector)
        v.addWidget(self._typewriter)
        self.setLayout(v)

        self._typewriter.setFocus()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._notation_select.set_ui_model(ui_model)
        self._hit_map_toggle.set_ui_model(ui_model)
        self._octave_selector.set_ui_model(ui_model)
        self._typewriter.set_ui_model(ui_model)
        self._profile_control.set_ui_model(ui_model)

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            if cmdline.get_experimental():
                self._profile_control.show()

    def unregister_updaters(self):
        self._typewriter.unregister_updaters()
        self._octave_selector.unregister_updaters()
        self._hit_map_toggle.unregister_updaters()
        self._notation_select.unregister_updaters()


