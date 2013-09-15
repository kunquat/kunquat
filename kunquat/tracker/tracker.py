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

from widgets.mainwindow import MainWindow

from kunquat.tracker.model.driver_manager import DriverManager
from kunquat.tracker.model.stat_manager import StatManager
from kunquat.tracker.model.ui_manager import UiManager
from kunquat.tracker.model.playback_manager import PlaybackManager
from kunquat.tracker.model.module import Module
from kunquat.tracker.model.uimodel import UiModel

def create_ui_model():
    #drivers = [Nullaudio, Pulseaudio, Pushaudio, Silentaudio]
    driver_manager = DriverManager()
    #driver_manager.set_drivers(drivers)
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
    return ui_model

def main():
    app = QApplication(sys.argv)
    main_window = MainWindow()
    ui_model = create_ui_model()
    main_window.set_ui_model(ui_model)
    main_window.show()
    app.exec_()

