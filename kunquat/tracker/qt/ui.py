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
        self._fe = None

    def set_frontend(self, fe):
        self._fe = fe

    def __del__(self):
        pass


class Ui():

    def __init__(self):
        self._app = QApplication(sys.argv)
        self._mainwindow = TestWindow()
        self._qp = None
        self._qp_timer = QTimer()

    def set_frontend(self, fe):
        self._mainwindow.set_frontend(fe)

    def set_queue_processor(self, qp):
        assert not self._qp
        self._qp = qp
        QObject.connect(
                self._qp_timer,
                SIGNAL('timeout()'),
                self._qp)
        self._qp_timer.start(20)

    def halt(self):
        self._app.exit()

    def run(self):
        self._mainwindow.show()
        self._app.exec_()
        self._qp_timer.stop()


