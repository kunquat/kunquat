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

import threading
import unittest
from time import sleep


class DummyAudioSource():
    def generate_audio(self, nframes):
        pass


class SilentAudioSource():

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def generate_audio(self, nframes):
        silence = ([0.1] * nframes, [0.1] * nframes)
        self._audio_output.put_audio(silence)


class TestAbstractDriver():

    def setUp(self):
        self._self._DriverClass = None

    def test_emptypush(self):
        driver = self._DriverClass()
        driver.put_audio(([],[]))

    def test_quickpush(self):
        driver = self._DriverClass()
        driver.put_audio(([0],[0]))

    def test_prefeed(self):
        driver = self._DriverClass()
        driver.set_audio_source(DummyAudioSource())
        driver.put_audio((10000*[0.1],10000*[0.1]))
        driver.start()
        sleep(0.2)
        driver.put_audio((10000*[0.1],10000*[0.1]))
        driver.stop()

    def _boot_driver(self):
        driver = self._DriverClass()
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        driver.close()

    def test_driver_cleanup(self):
        initial_threads = threading.active_count()
        for i in xrange(50):
            self._boot_driver()
        remaining_threads = threading.active_count()
        self.assertEqual(initial_threads, remaining_threads)

    def test_interrupt_driver(self):
        driver = self._DriverClass()
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        sleep(0.2)
        driver.close()

    def disabled_stress_test(self):
        for _ in range(100):
            driver = self._DriverClass()
            audio_source = SilentAudioSource()
            audio_source.set_audio_output(driver)
            driver.set_audio_source(audio_source)
            driver.start()
            sleep(0.2)
            driver.close()


