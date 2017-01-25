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
from .composition import Composition
from .importprogress import ImportProgress
from .inputcontrols import InputControls
from .peakmeter import PeakMeter
from .portal import Portal
from .topcontrols import TopControls


class MainView(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._portal = Portal()
        self._top_controls = TopControls()
        self._composition = Composition()
        self._input_controls = InputControls()
        self._import_progress = ImportProgress()
        self._peak_meter = PeakMeter()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._portal)
        v.addWidget(self._top_controls)
        v.addSpacing(4)
        v.addWidget(self._composition)
        v.addWidget(self._input_controls)
        v.addWidget(self._import_progress)
        v.addSpacing(4)
        v.addWidget(self._peak_meter)
        self.setLayout(v)

        if not cmdline.get_experimental():
            self._import_progress.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._portal.set_ui_model(ui_model)
        self._top_controls.set_ui_model(ui_model)
        self._composition.set_ui_model(ui_model)
        self._input_controls.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._peak_meter.unregister_updaters()
        self._import_progress.unregister_updaters()
        self._input_controls.unregister_updaters()
        self._composition.unregister_updaters()
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


