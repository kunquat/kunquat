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

from launcher import QtLauncher


class DummyFrontend():

    def __init__(self):
        pass


class TestLauncher(unittest.TestCase):

    def _dummyQueueProcessor(self):
        if self._call_counter > 0:
            self._call_counter -= 1
            return  
        self._ui_launcher.halt_ui()

    def test_launcher(self):
        self._call_counter = 2
        self._ui_launcher = QtLauncher(show = False)
        self._ui_launcher.set_frontend(DummyFrontend())
        self._ui_launcher.set_queue_processor(self._dummyQueueProcessor)
        self._ui_launcher.run_ui()


if __name__ == '__main__':
    unittest.main()


