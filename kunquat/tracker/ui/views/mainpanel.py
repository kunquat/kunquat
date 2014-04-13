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
from portal import Portal
from octaveselector import OctaveSelector
from typewriter import Typewriter
from instrumentselect import InstrumentSelect
from importprogress import ImportProgress
from peakmeter import PeakMeter
from profilecontrol import ProfileControl


class MainPanel(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._portal = Portal()
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()
        self._instrument_select = InstrumentSelect()
        self._import_progress = ImportProgress()
        self._peak_meter = PeakMeter()
        self._profile_control = ProfileControl()

        v = QVBoxLayout()
        v.addWidget(self._portal)
        v.addWidget(self._octave_selector)
        v.addWidget(self._typewriter)
        v.addWidget(self._instrument_select)
        v.addWidget(self._import_progress)
        v.addWidget(self._peak_meter)
        self.setLayout(v)

        self._typewriter.setFocus()

        if not cmdline.get_experimental():
            self._instrument_select.hide()
            self._import_progress.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._portal.set_ui_model(ui_model)
        self._octave_selector.set_ui_model(ui_model)
        self._typewriter.set_ui_model(ui_model)
        self._instrument_select.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)
        self._profile_control.set_ui_model(ui_model)

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            if cmdline.get_experimental():
                self._profile_control.show()

    def unregister_updaters(self):
        self._peak_meter.unregister_updaters()
        self._import_progress.unregister_updaters()
        self._instrument_select.unregister_updaters()
        self._typewriter.unregister_updaters()
        self._octave_selector.unregister_updaters()
        self._portal.unregister_updaters()

