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

import time
import random
import unittest
import threading
from Queue import Queue
from threading import Thread

from audio_output import AudioOutput
from drivers.pulseaudio import Pulseaudio
from drivers.pushaudio import Pushaudio
from drivers.nullaudio import Nullaudio


class BrokenDriver():
    def __init__(self):
        raise Exception

class WorkingDriver():

    def __init__(self):
        self._audio_source = None

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def start(self):
        pass

    def close(self):
        pass

class TestAudioOutput(unittest.TestCase):

    def setUp(self):
        self._audio_output = AudioOutput()

    def test_select_driver_success(self):
        q = Queue()
        class DummyFrontend(Thread):
            def select_driver_success(self, driver):
                q.put(driver)
        class DummyBackend(Thread):
            def set_audio_output(self, audio_output):
                pass
            def acknowledge_audio(self):
                pass
            def update_selected_driver(self, DriverClass):
                pass
        dummy_backend = DummyBackend()
        dummy_frontend = DummyFrontend()
        self._audio_output.set_backend(dummy_backend)
        self._audio_output.set_frontend(dummy_frontend)
        self._audio_output.select_driver(WorkingDriver)
        selected = q.get()
        self.assertEqual(selected, WorkingDriver)

    def test_select_driver_error(self):
        q = Queue()
        class DummyFrontend(Thread):
            def select_driver_error(self, driver_class):
                q.put(driver_class)
            def select_driver_success(self, driver_class):
                pass
        class DummyBackend(Thread):
            def acknowledge_audio(self):
                pass
            def update_selected_driver(self, DriverClass):
                pass
        dummy_backend = DummyBackend()
        dummy_frontend = DummyFrontend()
        self._audio_output.set_backend(dummy_backend)
        self._audio_output.set_frontend(dummy_frontend)
        self._audio_output.select_driver(BrokenDriver)
        selected = q.get()
        self.assertEqual(selected, BrokenDriver)

    def test_actual_driver_selection(self):
        start = threading.active_count()
        driver_classes = [Nullaudio, Pulseaudio, Pushaudio]
        q = Queue()
        class DummyFrontend(Thread):
            def select_driver_success(self, driver):
                q.put(driver)
        class DummyBackend(Thread):
            def set_audio_output(self, audio_output):
                pass
            def acknowledge_audio(self):
                pass
            def update_selected_driver(self, DriverClass):
                pass
        dummy_backend = DummyBackend()
        dummy_frontend = DummyFrontend()
        self._audio_output.set_backend(dummy_backend)
        self._audio_output.set_frontend(dummy_frontend)
        some_drivers = 4 * driver_classes
        seed = (lambda:0.2)
        random.shuffle(some_drivers, seed)
        for driver in some_drivers:
            self._audio_output.select_driver(driver)
            selected = q.get()
            self.assertEqual(selected, driver)
        self._audio_output.select_driver(None)
        time.sleep(0.2)
        end = threading.active_count()
        self.assertEqual(start, end)


if __name__ == '__main__':
    unittest.main()


