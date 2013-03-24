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

from frontendthread import FrontendThread

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

class TestFrontendthread(unittest.TestCase):

    def test_halt(self):
        frontend_thread = FrontendThread()
        frontend_thread.set_ui_launcher(DummyLauncher())
        frontend_thread.halt()
        frontend_thread.run()


if __name__ == '__main__':
    unittest.main()


