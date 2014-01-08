# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
#          Toni Ruottu, Finland 2013
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

from crashtests import CrashTests
from playbutton import PlayButton
from octaveselector import OctaveSelector
from typewriter import TypeWriter
from instrumentselect import InstrumentSelect
from renderstats import RenderStats
from importprogress import ImportProgress
from peakmeter import PeakMeter
from sheet.sheet import Sheet

class MainWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self.resize(800, 600)
        self._crash_tests = CrashTests()
        self._play_button = PlayButton()
        self._octave_selector = OctaveSelector()
        self._type_writer = TypeWriter()
        self._instrument_select = InstrumentSelect()
        self._import_progress = ImportProgress()
        self._render_stats = RenderStats()
        self._peak_meter = PeakMeter()
        #self._sheet = Sheet()

        v = QVBoxLayout()
        v.addWidget(self._crash_tests)
        v.addWidget(self._play_button)
        v.addWidget(self._octave_selector)
        v.addWidget(self._type_writer)
        v.addWidget(self._instrument_select)
        v.addWidget(self._import_progress)
        v.addWidget(self._render_stats)
        v.addWidget(self._peak_meter)
        #v.addWidget(self._sheet)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._crash_tests.set_ui_model(ui_model)
        self._play_button.set_ui_model(ui_model)
        self._octave_selector.set_ui_model(ui_model)
        self._type_writer.set_ui_model(ui_model)
        self._instrument_select.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._render_stats.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)
        #self._sheet.set_ui_model(ui_model)

