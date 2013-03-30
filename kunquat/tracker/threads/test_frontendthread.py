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

import unittest
import threading

from kunquat.tracker.frontend.frontend import Frontend
from frontendthread import FrontendThread
from test_abstractthread import TestAbstractThread

class DummyLauncher():

    def __init__(self):
        self._running = None
        self._queue_processor = None

    def set_frontend(self, frontend):
        pass

    def set_queue_processor(self, queue_processor):
        self._queue_processor = queue_processor

    def halt_ui(self):
        self._running = False

    def run_ui(self):
        self._running = True
        while(self._running == True):
            self._queue_processor()


class DummyBackend(threading.Thread):
    pass


class DummyAudio(threading.Thread):
    pass


class TestFrontendthread(TestAbstractThread, unittest.TestCase):

    def setUp(self):
        self._test_calls = [
          ('set_audio_output', (DummyAudio(),), {}),
          ('set_backend', (DummyBackend(),), {}),
          ('update_drivers', ({'coolsound':{'name': 'coolsound test driver'}},), {})
        ]
        self._InterfaceClass = Frontend
        self._TestClass = FrontendThread
        self._thread = self._TestClass()
        self._thread.set_ui_launcher(DummyLauncher())


if __name__ == '__main__':
    unittest.main()

