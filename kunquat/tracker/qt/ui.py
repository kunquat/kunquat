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

import sys

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class TestWindow(QMainWindow):

    def __init__(self):
        QMainWindow.__init__(self)
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def __del__(self):
        pass


class Ui():

    def __init__(self):
        self._app = QApplication(sys.argv)
        self._mainwindow = TestWindow()
        self._qp = None
        self._qp_timer = QTimer()
        self._driver_switch_timer = QTimer()
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._mainwindow.set_ui_model(ui_model)

    def set_queue_processor(self, qp):
        assert not self._qp
        self._qp = qp
        QObject.connect(
                self._qp_timer,
                SIGNAL('timeout()'),
                self._qp)
        self._qp_timer.start(20)

    def select_random_driver(self):
        import random
        driver_manager = self._ui_model.get_driver_manager()
        drivers = driver_manager.get_drivers()
        one = random.choice(drivers)
        print 'driver: %s' % one.get_id()
        driver_manager.select_audio_driver(one)

    def _start_driver_randomizer(self):
        QObject.connect(
                self._driver_switch_timer,
                SIGNAL('timeout()'),
                self.select_random_driver)
        self._driver_switch_timer.start(3000)

    def halt(self):
        self._app.exit()

    def show(self):
        self._mainwindow.show()

    def run(self):
        self._start_driver_randomizer()
        self._app.exec_()
        self._qp_timer.stop()


