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

class DummyStatManager():

    def register_updater(self, updater):
        pass

    def get_import_progress_position(self):
        return 1

    def get_import_progress_steps(self):
        return 1

    def get_output_speed(self):
        return 48000

    def get_render_speed(self):
        return 50000

    def get_render_load(self):
        return 0.5

class DummyDriverManager():

    def register_updater(self, updater):
        pass

class DummyUiModel():

    def __init__(self):
        pass

    def set_ui(self, ui):
        pass

    def get_stat_manager(self):
        return DummyStatManager()

    def get_driver_manager(self):
        return DummyDriverManager()

    def perform_updates(self):
        pass

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


