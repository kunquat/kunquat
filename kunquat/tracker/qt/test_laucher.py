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

from launcher import QtLauncher


class DummyUiModel():

    def __init__(self):
        pass

    def set_ui(self, ui):
        pass

    def get_prog_position(self): # TODO: remove
        return 0

    def get_prog_last(self): # TODO: remove
        return 1


class TestLauncher(unittest.TestCase):

    def setUp(self):
        self._block_call_count = 0

    def _dummyQueueProcessor(self):
        #self.assertEqual(self._block_call_count, 1)
        self._block_call_count -= 1

        if self._call_counter > 0:
            self._call_counter -= 1
            return
        self._ui_launcher.halt_ui()

    def _dummyBlock(self):
        self._block_call_count += 1

    def test_launcher(self):
        self._call_counter = 2
        self._ui_launcher = QtLauncher(show = False)
        self._ui_launcher.set_ui_model(DummyUiModel())
        self._ui_launcher.set_queue_processor(self._dummyQueueProcessor, self._dummyBlock)
        self._ui_launcher.run_ui()


if __name__ == '__main__':
    unittest.main()


