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

import sys
import time

from views.mainwindow import MainWindow

from kunquat.tracker.model.driver_manager import DriverManager
from kunquat.tracker.model.stat_manager import StatManager
from kunquat.tracker.model.ui_manager import UiManager
from kunquat.tracker.model.playback_manager import PlaybackManager
from kunquat.tracker.model.module import Module
from kunquat.tracker.model.uimodel import UiModel
from kunquat.tracker.model.updater import Updater

from kunquat.tracker.audio.drivers.silentaudio import Silentaudio
from kunquat.tracker.audio.drivers.pulseaudio import Pulseaudio
from kunquat.tracker.audio.drivers.pushaudio import Pushaudio
from kunquat.tracker.audio.drivers.nullaudio import Nullaudio

class Tracker():

    def __init__(self):
        self.previous = 0

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
        updater = Updater()
        ui_model.set_updater(updater)
        return ui_model

    def update(self):
        self.current = time.time()
        s = self.current - self.previous
        ms = s * 1000
        lag = ms - 10
        if lag > 1:
            print lag
        self.previous = self.current

    def heavy(self):
        print 'hs'
        for _ in range(10000):
            QApplication.processEvents()
            for _ in range(10000):
                pass
        print 'he'

    def main(self):
        app = QApplication(sys.argv)
        main_window = MainWindow()
        ui_model = self.create_ui_model()

        update_timer = QTimer()
        QObject.connect(update_timer,
                        SIGNAL('timeout()'),
                        self.update)
                        #ui_model.perform_updates)
        update_timer.start(10)

        update_timer2 = QTimer()
        QObject.connect(update_timer2,
                        SIGNAL('timeout()'),
                        self.heavy)
        update_timer2.start(1000)

        main_window.set_ui_model(ui_model)
        main_window.show()
        app.exec_()

t = Tracker()
t.main()

