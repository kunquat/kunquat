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

from .hitmaptoggle import HitMapToggle
from .octaveselector import OctaveSelector
from .typewriter import Typewriter
from .notationselect import NotationSelect
from .updater import Updater


class TypewriterPanel(QFrame, Updater):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._notation_select = NotationSelect()
        self._hit_map_toggle = HitMapToggle()
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()

        self.add_to_updaters(
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


