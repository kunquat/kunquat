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

import os
import re
import sys
import time
import json
import tarfile
from collections import deque
from signal import SIGHUP, SIGKILL

import kunquat.tracker.cmdline as cmdline
import kunquat.tracker.config as config
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
        self._tasks = deque([])
        self._task_timer = None

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

    def add_task(self, task):
        self._tasks.append(task)
        self._task_timer.start(0)

    def _execute_tasks(self):
        if self._tasks:
            cur_task = self._tasks.popleft()
            try:
                next(cur_task)
                self._tasks.appendleft(cur_task)
            except StopIteration:
                pass

        if self._tasks:
            self._task_timer.start(0)

    def run_ui(self):
        app = QApplication(sys.argv)
        error_dialog = ErrorDialog()
        root_view = RootView()
        root_view.set_task_executer(self.add_task)

        update_timer = QTimer()
        QObject.connect(update_timer, SIGNAL('timeout()'), self.update)
        update_timer.start(1000 * self.UI_DELTA)

        root_view.set_ui_model(self._ui_model)
        root_view.set_crash_dialog(error_dialog)

        self._event_pump_starter()

        self._task_timer = QTimer()
        self._task_timer.setSingleShot(True)
        QObject.connect(self._task_timer, SIGNAL('timeout()'), self._execute_tasks)

        if not self._show:
            visibility_manager = self._ui_model.get_visibility_manager()
            visibility_manager.run_hidden()

        root_view.show_main_window()
        root_view.setup_module()

        app.exec_()

        root_view.unregister_updaters()
        self._controller.get_updater().verify_ready_to_exit()

        config.try_save()

    def halt_ui(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_main()


def create_ui_launcher():
    controller = create_controller()
    ui_model = create_ui_model()
    controller.set_ui_model(ui_model)
    ui_model.set_controller(controller)
    ui_launcher = UiLauncher()
    ui_launcher.set_ui_model(ui_model)
    ui_launcher.set_controller(controller)
    return ui_launcher


