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

        self._progressBar = QProgressBar(self)
        self._progressBar.setGeometry(QRect(30, 70, 481, 23))
        self._progressBar.setObjectName("progressBar")
        self._progressBar.setValue(1)
        self._progressBar.setMaximum(1)
        self._progressBar.setMinimum(0)

        self._render_load = QLabel(self)

    def update_progress(self):
        position = self._ui_model.get_prog_position()
        last = self._ui_model.get_prog_last()
        self._progressBar.setMaximum(last)
        self._progressBar.setValue(position)

    def update_render_load(self):
        ratio = self._ui_model.get_render_load()
        self._render_load.setText('{} %'.format(int(ratio * 100)))

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def run(self):
        self.update_progress()

    def __del__(self):
        pass


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

class Updater():

    def __init__(self):
        self._signals = set()
        self._updaters = {}

    def queue_update(self, signal):
        self._signals.add(signal)

    def register_updater(self, signal, updater):
        if not signal in self._updaters:
            self._updaters[signal] = set()
        self._updaters[signal].add(updater)

    def _run_updaters(self, signal):
        if not signal in self._updaters:
            return
        for updater in self._updaters[signal]:
            updater()

    def perform_updates(self):
        for signal in self._signals:
            self._run_updaters(signal)
        self._signals = set()

class Ui():

    def __init__(self):
        self._app = QApplication(sys.argv)
        self._mainwindow = TestWindow()
        self._qp = None
        self._event_pump = EventPump()
        self._driver_switch_timer = QTimer()
        self._update_timer = QTimer()
        self._ui_model = None
        self._asdfasdf = False
        self._updater = Updater()
        self._updater.register_updater('progress', self._mainwindow.update_progress)
        self._updater.register_updater('render_load', self._mainwindow.update_render_load)

    def update_progress(self):
        self._updater.queue_update('progress')

    def update_render_load(self):
        self._updater.queue_update('render_load')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._mainwindow.set_ui_model(ui_model)

    def set_queue_processor(self, qp, block):
        assert not self._qp
        self._qp = qp
        self._event_pump.set_block(block)
        QObject.connect(
                self._event_pump,
                SIGNAL('process_queue()'),
                self._qp)
        self._event_pump.start()

    def select_random_driver(self):
        if self._asdfasdf:
            return
        self._asdfasdf = True
        from kunquat.tracker.audio.drivers.pushaudio import Pushaudio
        from kunquat.tracker.audio.drivers.nullaudio import Nullaudio
        from kunquat.tracker.audio.drivers.silentaudio import Silentaudio
        one = Pushaudio
        print 'driver: %s' % one.get_id()
        driver_manager = self._ui_model.get_driver_manager()
        driver_manager.select_driver(one)
        self._ui_model.load_module()

        #import random
        #driver_manager = self._ui_model.get_driver_manager()
        #drivers = driver_manager.get_drivers()
        #one = random.choice(drivers)

    def _start_driver_randomizer(self):
        QObject.connect(
                self._driver_switch_timer,
                SIGNAL('timeout()'),
                self.select_random_driver)
        self._driver_switch_timer.start(3000)

    def _start_updater(self):
        QObject.connect(
                self._update_timer,
                SIGNAL('timeout()'),
                self._updater.perform_updates)
        self._update_timer.start(10)

    def halt(self):
        self._event_pump.terminate()
        self._app.exit()

    def show(self):
        self._mainwindow.show()

    def run(self):
        self._mainwindow.run()
        self._start_updater()
        self._start_driver_randomizer()
        self._app.exec_()


