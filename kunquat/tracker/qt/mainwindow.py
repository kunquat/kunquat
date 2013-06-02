# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
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

from play_button import PlayButton
from type_writer import TypeWriter
from driver_select import DriverSelect
from instrument_select import InstrumentSelect
from render_stats import RenderStats
from import_progress import ImportProgress
from peak_meter import PeakMeter
#from active_notes import ActiveNotes

class MainWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._play_button = PlayButton()
        self._type_writer = TypeWriter()
        self._driver_select = DriverSelect()
        self._instrument_select = InstrumentSelect()
        self._import_progress = ImportProgress()
        self._render_stats = RenderStats()
        self._peak_meter = PeakMeter()
        #self._active_notes = ActiveNotes()

        v = QVBoxLayout()
        v.addWidget(self._play_button)
        v.addWidget(self._type_writer)
        v.addWidget(self._driver_select)
        v.addWidget(self._instrument_select)
        v.addWidget(self._import_progress)
        v.addWidget(self._render_stats)
        v.addWidget(self._peak_meter)
        #v.addWidget(self._active_notes)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._play_button.set_ui_model(ui_model)
        self._type_writer.set_ui_model(ui_model)
        self._driver_select.set_ui_model(ui_model)
        self._instrument_select.set_ui_model(ui_model)
        self._render_stats.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)
        #self._active_notes.set_ui_model(ui_model)

