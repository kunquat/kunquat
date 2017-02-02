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
from .updatingview import UpdatingView


class TypewriterPanel(QFrame, UpdatingView):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._notation_select = NotationSelect()
        self._hit_map_toggle = HitMapToggle()
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()
        self._profile_control = ProfileControl()

        self.add_updating_child(
                self._notation_select,
                self._hit_map_toggle,
                self._octave_selector,
                self._typewriter)

        il = QHBoxLayout()
        il.setContentsMargins(0, 0, 0, 0)
        il.setSpacing(8)
        il.addWidget(self._notation_select)
        il.addWidget(self._hit_map_toggle)
        il.addStretch(1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 0, 4, 0)
        v.setSpacing(6)
        v.addLayout(il)
        v.addWidget(self._octave_selector)
        v.addWidget(self._typewriter)
        self.setLayout(v)

        self._typewriter.setFocus()

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            if cmdline.get_experimental():
                self._profile_control.show()


