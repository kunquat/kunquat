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
        self._frontend = None

    def set_frontend(self, frontend):
        self._frontend = frontend

    def __del__(self):
        pass


class Ui():

    def __init__(self):
        self._app = QApplication(sys.argv)
        self._mainwindow = TestWindow()
        self._qp = None
        self._qp_timer = QTimer()
        self._driver_switch_timer = QTimer()
        self._frontend = None

    def set_frontend(self, frontend):
        self._frontend = frontend
        self._mainwindow.set_frontend(frontend)

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
        drivers = self._frontend.get_drivers()
        driver_ids = drivers.get_ids()
        one = random.choice(driver_ids)
        print 'driver: %s' % one
        drivers.select_audio_driver(one)

    def start_driver_randomizer(self):
        QObject.connect(
                self._driver_switch_timer,
                SIGNAL('timeout()'),
                self.select_random_driver)
        self._driver_switch_timer.start(3000)

    def halt(self):
        self._app.exit()

    def run(self):
        self._mainwindow.show()
        self._app.exec_()
        self._qp_timer.stop()


