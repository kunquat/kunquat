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

import os
import re
import sys
import time
import json
import tarfile
from collections import deque
from signal import SIGHUP, SIGKILL

import kunquat.tracker.cmdline as cmdline
from kunquat.tracker.ui.model.uimodel import create_ui_model
from kunquat.tracker.ui.errordialog import ErrorDialog
from kunquat.tracker.ui.views.rootview import RootView
from kunquat.tracker.ui.controller.controller import create_controller


class UiLauncher():

    UI_FPS = 60
    UI_DELTA = 1.0 / float(UI_FPS)

    def __init__(self, show=True):
        self._show = show
        self._updater = None
        self._controller = None
        self._audio_engine = None
        self._queue_processor = None
        self._block = None
        self._ui_model = None
        self._event_pump_starter = None
        self._lag_times = deque([], 20)

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
        start = time.time()
        self._updater.perform_updates()
        end = time.time()

        s = end - start
        lag = s - self.UI_DELTA
        self._lag_times.append(lag)
        avg = sum(lag for lag in self._lag_times) / float(len(self._lag_times))
        self._controller.update_ui_lag(avg * 1000)

    def execute_task(self, task):
        for _ in task:
            QApplication.processEvents()

    def run_ui(self):
        app = QApplication(sys.argv)
        error_dialog = ErrorDialog()
        root_view = RootView()

        update_timer = QTimer()
        QObject.connect(update_timer,
                        SIGNAL('timeout()'),
                        self.update)
        update_timer.start(1000 * self.UI_DELTA)
        root_view.set_ui_model(self._ui_model)

        self._event_pump_starter()

        if not self._show:
            visibility_manager = self._ui_model.get_visibility_manager()
            visibility_manager.run_hidden()

        root_view.show_main_window()

        if cmdline.get_kqt_file():
            module_path = cmdline.get_kqt_file()
            load_task = self._controller.get_task_load_module(module_path)
            self.execute_task(load_task)
        else:
            kqtifile = self._controller.get_share().get_default_instrument()
            load_task = self._controller.get_task_load_instrument(kqtifile)
            self.execute_task(load_task)
        app.exec_()

        root_view.unregister_updaters()
        self._controller.get_updater().verify_ready_to_exit()

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


