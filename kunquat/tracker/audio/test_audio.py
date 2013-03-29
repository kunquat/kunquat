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
import random

from audio import Audio


class DummyBackend():

    def __init__(self):
        self._selected = None

    def update_selected_driver(self, driver):
        self._selected = driver

    def generate_audio(self, nframes):
        return ([],[])


class DummyFrontend():

    def __init__(self):
        self._drivers = None

    def update_drivers(self, drivers):
        self._drivers = drivers


class TestAudio(unittest.TestCase):

    def test_driver_selection(self):
        audio_output = Audio()
        dummy_backend = DummyBackend()
        dummy_frontend = DummyFrontend()
        audio_output.set_backend(dummy_backend)
        audio_output.set_frontend(dummy_frontend)
        audio_output.request_update()
        driver_ids = dummy_frontend._drivers.keys()
        test_ids = 4 * driver_ids
        seed = (lambda:0.2)
        random.shuffle(test_ids, seed)
        for driver_id in test_ids:
            audio_output.select_driver(driver_id)
            self.assertEqual(dummy_backend._selected, driver_id)


if __name__ == '__main__':
    unittest.main()


