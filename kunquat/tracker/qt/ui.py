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

from mainwindow import MainWindow


class EventPump(QThread):

    process_queue = pyqtSignal(name='process_queue')

    def __init__(self):
        QThread.__init__(self)

    def set_block(self, block):
        self._block = block

    def run(self):
        while True:
            self._block()
            QObject.emit(self, SIGNAL('process_queue()'))

class Ui():

    def __init__(self):
        self._app = QApplication(sys.argv)
        self._mainwindow = MainWindow()
        self._qp = None
        self._event_pump = EventPump()
        self._update_timer = QTimer()
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._mainwindow.set_ui_model(ui_model)
        from kunquat.tracker.audio.drivers.pushaudio import Pushaudio
        driver_manager = self._ui_model.get_driver_manager()
        driver_manager.set_selected_driver(Pushaudio)

    def set_queue_processor(self, qp, block):
        assert not self._qp
        self._qp = qp
        self._event_pump.set_block(block)
        QObject.connect(
                self._event_pump,
                SIGNAL('process_queue()'),
                self._qp)
        self._event_pump.start()

    def _start_updater(self):
        QObject.connect(
                self._update_timer,
                SIGNAL('timeout()'),
                self._ui_model.perform_updates)
        self._update_timer.start(10)

    def halt(self):
        self._event_pump.terminate()
        self._app.exit()

    def show(self):
        self._mainwindow.show()

    def run(self):
        self._start_updater()
        self._ui_model.load_module()
        self._app.exec_()


