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

import os
import re
import sys
import time
import json
import tarfile
from signal import SIGHUP, SIGKILL

from kunquat.tracker.ui.model.uimodel import create_ui_model

from kunquat.tracker.ui.errordialog import ErrorDialog
from kunquat.tracker.ui.views.mainwindow import MainWindow
from kunquat.tracker.ui.views.rootview import RootView
from kunquat.tracker.ui.controller.controller import create_controller


class UiLauncher():

    def __init__(self, show=True):
        self._show = show
        self.previous = 0
        self._updater = None
        self._controller = None
        self._audio_engine = None
        self._queue_processor = None
        self._block = None
        self._ui_model = None
        self._event_pump_starter = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def set_controller(self, controller):
        self._controller = controller
        updater = controller.get_updater()
        self._updater = updater

    def get_controller(self):
        return self._controller

    def set_queue_processor(self, queue_processor, block):
        self._queue_processor = queue_processor
        self._block = block

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine
        self._controller.set_audio_engine(audio_engine)

    def set_event_pump_starter(self, event_pump_starter):
        self._event_pump_starter = event_pump_starter

    def update(self):
        self.current = time.time()
        s = self.current - self.previous
        ms = s * 1000
        lag = ms - 10
        self._controller.update_ui_lag(lag)
        self.previous = self.current
        self._updater.perform_updates()

    def execute_task(self, task):
        for _ in task:
            QApplication.processEvents()

    def run_ui(self):
        app = QApplication(sys.argv)
        error_dialog = ErrorDialog()
        main_window = MainWindow()
        root_view = RootView()

        update_timer = QTimer()
        QObject.connect(update_timer,
                        SIGNAL('timeout()'),
                        self.update)
        update_timer.start(10)
        root_view.set_ui_model(self._ui_model)

        self._event_pump_starter()

        #if not self._show:
        #    visibility_manager = self._ui_model.get_visibility_manager()
        #    visibility_manager.run_hidden()

        root_view.show_main_window()

        if len(sys.argv) > 1:
            module_path = sys.argv[1]
            load_task = self._controller.get_task_load_module(module_path)
            self.execute_task(load_task)
        app.exec_()


    def halt_ui(self):
        pass

def create_ui_launcher():
    controller = create_controller()
    ui_model = create_ui_model()
    ui_model.set_controller(controller)
    ui_launcher = UiLauncher()
    ui_launcher.set_ui_model(ui_model)
    ui_launcher.set_controller(controller)
    #ui_launcher.set_instrument_class(Instrument)
    return ui_launcher

