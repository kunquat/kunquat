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

import os
import re
import sys
import time
import json
import tarfile
from signal import SIGHUP, SIGKILL

from kunquat.tracker.ui.model.uimodel import create_ui_model
from kunquat.tracker.ui.model.updater import Updater

from kunquat.tracker.ui.views.mainwindow import MainWindow
from kunquat.tracker.ui.controller.controller import Controller
from kunquat.tracker.ui.controller.store import Store


class UiEngine():

    def __init__(self, show=True):
        self._show = show
        self.previous = 0
        self._updater = None
        self._controller = Controller()
        self._audio_engine = None
        self._queue_processor = None
        self._block = None
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def set_controller(self, controller):
        self._controller = controller

    def set_queue_processor(self, queue_processor, block):
        self._queue_processor = queue_processor
        self._block = block

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine

    def set_updater(self, updater):
        self._updater = updater

    def update(self):
        self.current = time.time()
        s = self.current - self.previous
        ms = s * 1000
        lag = ms - 10
        if lag > 1:
            print lag
        self.previous = self.current
        self._updater.perform_updates()

    def execute_task(self, task):
        for _ in task:
            QApplication.processEvents()

    def run_ui(self):
        app = QApplication(sys.argv)
        main_window = MainWindow()

        update_timer = QTimer()
        QObject.connect(update_timer,
                        SIGNAL('timeout()'),
                        self.update)
        update_timer.start(10)

        if len(sys.argv) > 1:
            module_path = sys.argv[1]
            load_task = self._controller.get_task_load_module(module_path)
            self.execute_task(load_task)

        main_window.set_ui_model(self._ui_model)
        if self._show:
            main_window.show()
        app.exec_()

    def halt_ui(self):
        pass

def create_ui_engine():
    updater = Updater()
    store = Store()
    ui_model = create_ui_model()
    ui_model.set_updater(updater)
    ui_model.set_store(store)
    controller = Controller()
    controller.set_store(store)
    controller.set_updater(updater)
    ui_engine = UiEngine()
    ui_engine.set_ui_model(ui_model)
    ui_engine.set_controller(controller)
    ui_engine.set_updater(updater)
    #ui_engine.set_instrument_class(Instrument)
    return ui_engine

