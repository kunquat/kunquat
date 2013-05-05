# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import unittest
import threading

from kunquat.tracker.frontend.test_frontend import TestFrontend
from kunquat.tracker.frontend.frontend import Frontend
from frontendthread import FrontendThread
from test_abstractthread import TestAbstractThread

class DummyLauncher():

    def __init__(self):
        self._running = None
        self._queue_processor = None
        self._block = None

    def set_frontend(self, frontend):
        pass

    def set_queue_processor(self, queue_processor, block):
        self._queue_processor = queue_processor
        self._block = block

    def halt_ui(self):
        self._running = False

    def run_ui(self):
        self._running = True
        while(self._running == True):
            self._block()
            self._queue_processor()


class DummyBackend(threading.Thread):
    pass


class DummyAudio(threading.Thread):
    pass


class TestFrontendthread(TestAbstractThread, unittest.TestCase):

    def setUp(self):
        self._thread = FrontendThread()
        self._thread.set_ui_launcher(DummyLauncher())


class TestThreadedFrontend(TestFrontend):

    def setUp(self):
        TestFrontend.setUp(self)
        handler = self._frontend
        self._frontend = FrontendThread()
        self._frontend.set_handler(handler)
        self._frontend.set_ui_launcher(DummyLauncher())
        self._frontend.start()

    def tearDown(self):
        self._frontend.halt()
        self._frontend.join()


if __name__ == '__main__':
    unittest.main()

