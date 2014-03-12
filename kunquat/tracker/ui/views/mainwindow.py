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
from playbutton import PlayButton
from aboutbutton import AboutButton
from eventlistbutton import EventListButton
from octaveselector import OctaveSelector
from typewriter import Typewriter
from instrumentselect import InstrumentSelect
from importprogress import ImportProgress
from peakmeter import PeakMeter
from sheet.sheet import Sheet
from profilecontrol import ProfileControl


class MainWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self.setWindowTitle('Kunquat Tracker')
        self._ui_model = None
        self.resize(800, 400)
        self._play_button = PlayButton()
        self._about_button = AboutButton()
        self._event_list_button = EventListButton()
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()
        self._instrument_select = InstrumentSelect()
        self._import_progress = ImportProgress()
        self._peak_meter = PeakMeter()
        self._sheet = Sheet()
        self._profile_control = ProfileControl()

        buttons = QHBoxLayout()
        buttons.addWidget(self._event_list_button)
        buttons.addWidget(self._about_button)

        v = QVBoxLayout()
        v.addWidget(self._play_button)
        v.addLayout(buttons)
        v.addWidget(self._octave_selector)
        v.addWidget(self._typewriter)
        v.addWidget(self._instrument_select)
        v.addWidget(self._import_progress)
        v.addWidget(self._peak_meter)
        v.addWidget(self._sheet)
        self.setLayout(v)

        if not cmdline.get_experimental():
            self._play_button.hide()
            self._sheet.hide()
            self._instrument_select.hide()
            self._import_progress.hide()

        self.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._play_button.set_ui_model(ui_model)
        self._about_button.set_ui_model(ui_model)
        self._event_list_button.set_ui_model(ui_model)
        self._octave_selector.set_ui_model(ui_model)
        self._typewriter.set_ui_model(ui_model)
        self._instrument_select.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)
        self._sheet.set_ui_model(ui_model)
        self._profile_control.set_ui_model(ui_model)

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            if cmdline.get_experimental():
                self._profile_control.show()

    def unregister_updaters(self):
        self._sheet.unregister_updaters()
        self._peak_meter.unregister_updaters()
        self._import_progress.unregister_updaters()
        self._instrument_select.unregister_updaters()
        self._typewriter.unregister_updaters()
        self._octave_selector.unregister_updaters()
        self._event_list_button.unregister_updaters()
        self._about_button.unregister_updaters()
        self._play_button.unregister_updaters()

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_main()


