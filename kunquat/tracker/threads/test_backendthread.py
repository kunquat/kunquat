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

from kunquat.tracker.backend.backend import Backend
from kunquat.tracker.backend.test_backend import TestBackend
from backendthread import BackendThread
from test_abstractthread import TestAbstractThread


class DummyFrontend(threading.Thread):
    pass

class TestBackendthread(TestAbstractThread, unittest.TestCase):

    def setUp(self):
        self._thread = BackendThread()


class TestThreadedBackend(TestBackend):

    def setUp(self):
        handler = Backend()
        self._backend = BackendThread()
        self._backend.set_handler(handler)
        self._backend.start()

    def tearDown(self):
        self._backend.halt()
        self._backend.join()


if __name__ == '__main__':
    unittest.main()


