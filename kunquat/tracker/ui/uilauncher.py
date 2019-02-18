# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2019
#          Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os
import re
import sys
import time
import json
from collections import deque

from kunquat.tracker.ui.qt import *

import kunquat.tracker.cmdline as cmdline
import kunquat.tracker.config as config
from kunquat.tracker.ui.controller.controller import create_controller
from kunquat.tracker.ui.model.uimodel import create_ui_model
# NOTE: Qt view modules must be imported after creating a QApplication; see run_ui


class UiLauncher():

    UI_FPS = 60
    UI_DELTA = 1.0 / float(UI_FPS)

    _UI_LOAD_FLUSH_INTERVAL = 1

    def __init__(self, show=True):
        self._show = show
        self._updater = None
        self._controller = None
        self._audio_engine = None
        self._queue_processor = None
        self._block = None
        self._ui_model = None
        self._event_queue_processor = None
        self._ui_loads = deque()
        self._ui_load_last_flush_time = None
        self._tasks = deque([])
        self._task_timer = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._ui_model.set_task_executor(self._add_task)

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

    def set_event_queue_processor(self, process_event_queue):
        self._process_event_queue = process_event_queue

    def update(self):
        start = time.time()
        self._process_event_queue()
        self._updater.perform_updates()
        end = time.time()

        # Report load on current update
        elapsed = end - start
        load = elapsed / self.UI_DELTA
        self._controller.update_ui_load(load)

        # Report statistics if we have gathered enough data
        if self._ui_load_last_flush_time == None:
            self._ui_load_last_flush_time = time.time()
        cur_time = time.time()
        flush_interval = self._UI_LOAD_FLUSH_INTERVAL
        if cur_time - self._ui_load_last_flush_time >= flush_interval:
            excess = cur_time - self._ui_load_last_flush_time - flush_interval
            if self._ui_loads:
                avg = sum(self._ui_loads) / float(len(self._ui_loads))
                self._controller.add_ui_load_average(avg)
                self._controller.add_ui_load_peak(max(self._ui_loads))
                self._ui_loads.clear()
            else:
                reported_load = (cur_time - self._ui_load_last_flush_time) * self.UI_FPS
                self._controller.add_ui_load_average(reported_load)
                self._controller.add_ui_load_peak(reported_load)

            self._ui_load_last_flush_time = cur_time + excess

        # Add to statistics
        self._ui_loads.append(load)

    def _add_task(self, task):
        assert (task != None)
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

        from kunquat.tracker.ui.errordialog import ErrorDialog
        from kunquat.tracker.ui.views.rootview import RootView
        import kunquat.tracker.ui.views.keyboardsetup as keyboardsetup

        keyboardsetup.setup(self._ui_model)

        error_dialog = ErrorDialog()
        root_view = RootView()

        update_timer = QTimer()
        update_timer.timeout.connect(self.update)
        update_timer.start(1000 * self.UI_DELTA)

        root_view.set_ui_model(self._ui_model)
        root_view.set_crash_dialog(error_dialog)

        self._task_timer = QTimer()
        self._task_timer.setSingleShot(True)
        self._task_timer.timeout.connect(self._execute_tasks)

        if not self._show:
            visibility_mgr = self._ui_model.get_visibility_manager()
            visibility_mgr.run_hidden()

        root_view.show_main_window()
        root_view.setup_module()

        app.exec_()

        root_view.unregister_updaters()
        self._controller.get_updater().verify_ready_to_exit()

        config.try_save()

    def halt_ui(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.hide_main()


def create_ui_launcher():
    controller = create_controller()
    ui_model = create_ui_model()
    controller.set_ui_model(ui_model)
    ui_model.set_controller(controller)
    ui_launcher = UiLauncher()
    ui_launcher.set_ui_model(ui_model)
    ui_launcher.set_controller(controller)
    return ui_launcher


