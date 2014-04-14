# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
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
from instrumentselect import InstrumentSelect
from profilecontrol import ProfileControl


class TypewriterPanel(QFrame):

    def __init__(self):
        QFrame.__init__(self)
        self._ui_model = None
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()
        self._instrument_select = InstrumentSelect()
        self._profile_control = ProfileControl()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.addWidget(self._octave_selector)
        v.addWidget(self._instrument_select)
        v.addWidget(self._typewriter)
        self.setLayout(v)

        self._typewriter.setFocus()

        if not cmdline.get_experimental():
            self._instrument_select.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._octave_selector.set_ui_model(ui_model)
        self._typewriter.set_ui_model(ui_model)
        self._instrument_select.set_ui_model(ui_model)
        self._profile_control.set_ui_model(ui_model)

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            if cmdline.get_experimental():
                self._profile_control.show()

    def unregister_updaters(self):
        self._instrument_select.unregister_updaters()
        self._typewriter.unregister_updaters()
        self._octave_selector.unregister_updaters()

