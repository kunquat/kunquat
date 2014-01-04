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

from playbutton import PlayButton
from aboutbutton import AboutButton
from octaveselector import OctaveSelector
from typewriter import TypeWriter
from instrumentselect import InstrumentSelect
from about import About
from importprogress import ImportProgress
from peakmeter import PeakMeter
from sheet.sheet import Sheet

class MainWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self.resize(800, 600)
        self._play_button = PlayButton()
        self._about_button = AboutButton()
        self._octave_selector = OctaveSelector()
        self._type_writer = TypeWriter()
        self._instrument_select = InstrumentSelect()
        self._import_progress = ImportProgress()
        self._about = About()
        self._peak_meter = PeakMeter()
        #self._sheet = Sheet()

        v = QVBoxLayout()
        v.addWidget(self._play_button)
        v.addWidget(self._about_button)
        v.addWidget(self._octave_selector)
        v.addWidget(self._type_writer)
        v.addWidget(self._instrument_select)
        v.addWidget(self._import_progress)
        v.addWidget(self._about)
        v.addWidget(self._peak_meter)
        #v.addWidget(self._sheet)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._play_button.set_ui_model(ui_model)
        self._about_button.set_ui_model(ui_model)
        self._octave_selector.set_ui_model(ui_model)
        self._type_writer.set_ui_model(ui_model)
        self._instrument_select.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._about.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)
        #self._sheet.set_ui_model(ui_model)

