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

from frontend import Frontend
from Queue import Queue


class DummyDriver():
    pass


class TestFrontend(unittest.TestCase):

    def setUp(self):
        q = Queue()
        self._q = q
        class DummyDriverManager():
            def select_driver_success(self, driver_class):
                q.put(driver_class)
        class DummyUiModel():
            def get_driver_manager(self):
                driver_manager_dummy = DummyDriverManager()
                return driver_manager_dummy
        ui_model_dummy = DummyUiModel()
        self._frontend = Frontend()
        self._frontend.set_ui_model(ui_model_dummy)

    def test_select_driver_success(self):
        self._frontend.select_driver_success(DummyDriver)
        selected = self._q.get()
        self.assertEqual(selected, DummyDriver)
        

if __name__ == '__main__':
    unittest.main()


