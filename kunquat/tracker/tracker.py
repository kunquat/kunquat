# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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

import re
import sys
import time
import json
import tarfile

from views.mainwindow import MainWindow

from kunquat.tracker.model.driver_manager import DriverManager
from kunquat.tracker.model.stat_manager import StatManager
from kunquat.tracker.model.ui_manager import UiManager
from kunquat.tracker.model.playback_manager import PlaybackManager
from kunquat.tracker.model.module import Module
from kunquat.tracker.model.uimodel import UiModel
from kunquat.tracker.model.updater import Updater

from kunquat.tracker.backend.backend import Backend

from kunquat.tracker.audio.drivers.silentaudio import Silentaudio
from kunquat.tracker.audio.drivers.pulseaudio import Pulseaudio
from kunquat.tracker.audio.drivers.pushaudio import Pushaudio
from kunquat.tracker.audio.drivers.nullaudio import Nullaudio

class Tracker():

    def __init__(self):
        self.previous = 0
        self.updater = Updater()
        self._backend = Backend()

    def create_ui_model(self):
        drivers = [Nullaudio, Pulseaudio, Pushaudio, Silentaudio]
        driver_manager = DriverManager()
        driver_manager.set_drivers(drivers)
        stat_manager = StatManager()
        ui_manager = UiManager()
        playback_manager = PlaybackManager()
        module = Module()
        ui_model = UiModel()
        ui_model.set_driver_manager(driver_manager)
        ui_model.set_stat_manager(stat_manager)
        ui_model.set_ui_manager(ui_manager)
        ui_model.set_playback_manager(playback_manager)
        ui_model.set_module(module)
        ui_model.set_updater(self.updater)
        ui_model.set_backend(self._backend)
        return ui_model

    def update(self):
        self.current = time.time()
        s = self.current - self.previous
        ms = s * 1000
        lag = ms - 10
        if lag > 1:
            print lag
        self.previous = self.current
        self.updater.perform_updates()

    def run_heavy(self, runner):
        for _ in runner:
            QApplication.processEvents()

    def main(self):
        app = QApplication(sys.argv)
        main_window = MainWindow()
        ui_model = self.create_ui_model()

        update_timer = QTimer()
        QObject.connect(update_timer,
                        SIGNAL('timeout()'),
                        self.update)
        update_timer.start(10)

        if len(sys.argv) > 1:
            module_path = sys.argv[1]
            loader = self._backend.load_module(module_path)
            self.run_heavy(loader)

        main_window.set_ui_model(ui_model)
        main_window.show()
        app.exec_()

t = Tracker()
t.main()

