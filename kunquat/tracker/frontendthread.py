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

import Queue
import sys
import threading

from PyQt4.QtCore import SIGNAL
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from command import Command
from frontend.frontend import Frontend


class TestWindow(QMainWindow):

    def __init__(self):
        QMainWindow.__init__(self)

        self._process_queue_timer = QTimer(self)
        self._qp = None

    def set_queue_processor(self, qp):
        self._qp = qp
        QObject.connect(
                self._process_queue_timer,
                SIGNAL('timeout()'),
                self._qp_proc)
        self._process_queue_timer.start(20)

    def _qp_proc(self):
        self._qp()

    def __del__(self):
        self._process_queue_timer.stop()


class FrontendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = Queue.Queue()
        self._frontend = Frontend()

    def set_command_processor(self, cp):
        self._frontend.set_command_processor(cp)

    def queue_event(self, event):
        self._q.put(event)

    def halt(self):
        self.queue_event(Command('halt', None))

    def process_queue(self):
        count_estimate = self._q.qsize()
        for _ in range(count_estimate):
            try:
                event = self._q.get_nowait()
                self._frontend.process_event(event)
            except Queue.Empty:
                break

    def run(self):
        app = QApplication(sys.argv)
        tracker = TestWindow()
        tracker.set_queue_processor(self.process_queue)
        tracker.show()
        app.exec_()

        """
        while event.name != 'halt':
            self.process_queue()
        """


