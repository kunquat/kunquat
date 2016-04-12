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
from .portal import Portal
from .topcontrols import TopControls
from .mainsplitter import MainSplitter
from .importprogress import ImportProgress
from .peakmeter import PeakMeter


class MainView(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._portal = Portal()
        self._top_controls = TopControls()
        self._main_splitter = MainSplitter()
        self._import_progress = ImportProgress()
        self._peak_meter = PeakMeter()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._portal)
        v.addWidget(self._top_controls)
        v.addSpacing(4)
        v.addWidget(self._main_splitter)
        v.addWidget(self._import_progress)
        v.addSpacing(4)
        v.addWidget(self._peak_meter)
        self.setLayout(v)

        if not cmdline.get_experimental():
            self._import_progress.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._portal.set_ui_model(ui_model)
        self._top_controls.set_ui_model(ui_model)
        self._main_splitter.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._peak_meter.unregister_updaters()
        self._import_progress.unregister_updaters()
        self._main_splitter.unregister_updaters()
        self._top_controls.unregister_updaters()
        self._portal.unregister_updaters()

    def keyPressEvent(self, event):
        if event.modifiers() == Qt.NoModifier:
            if event.key() == Qt.Key_Comma:
                self._ui_model.play()
            elif event.key() == Qt.Key_Period:
                self._ui_model.silence()
            else:
                event.ignore()
        elif event.modifiers() == Qt.ControlModifier:
            if event.key() == Qt.Key_Comma:
                self._ui_model.play_pattern()
            else:
                event.ignore()
        elif event.modifiers() == Qt.AltModifier:
            if event.key() == Qt.Key_Comma:
                self._ui_model.play_from_cursor()
            else:
                event.ignore()
        else:
            event.ignore()


