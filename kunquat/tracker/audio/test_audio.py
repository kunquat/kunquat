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

import random
import unittest
from Queue import Queue
from threading import Thread

from audio import Audio


class TestAudio(unittest.TestCase):

    def setUp(self):
        self._audio_output = Audio()

    def test_driver_selection(self):
        q = Queue()
        class DummyFrontend(Thread):
            def update_drivers(self, drivers):
                q.put(drivers)
        class DummyBackend(Thread):
            def __init__(self):
                self._audio_output = None
            def update_selected_driver(self, driver):
                q.put(driver)
            def set_audio_output(self, audio_output):
                self._audio_output = audio_output
            def generate_audio(self, nframes):
                audio_data = ([],[])
                if self._audio_output != None:
                    self._audio_output.put_audio(audio_data)
        dummy_backend = DummyBackend()
        dummy_frontend = DummyFrontend()
        self._audio_output.set_backend(dummy_backend)
        self._audio_output.set_frontend(dummy_frontend)
        self._audio_output.request_update()
        drivers = q.get()
        driver_ids = drivers.keys()
        test_ids = 4 * driver_ids
        seed = (lambda:0.2)
        random.shuffle(test_ids, seed)
        for driver_id in test_ids:
            self._audio_output.select_driver(driver_id)
            selected = q.get()
            self.assertEqual(selected, driver_id)


if __name__ == '__main__':
    unittest.main()


