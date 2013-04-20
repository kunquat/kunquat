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

from Queue import Queue
import threading
import unittest
from time import sleep


class DummyAudioSource():
    def generate_audio(self, nframes):
        pass


class TestAbstractDriver():

    def setUp(self):
        self._self._DriverClass = None

    def test_threadleak_on_close(self):
        start = threading.active_count()
        driver = self._DriverClass()
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        sleep(0.2)
        driver.close()
        sleep(0.2)
        end = threading.active_count()
        self.assertEqual(start, end)

    def test_audio_gets_requested(self):
        q = Queue()
        driver = self._DriverClass()
        class AudioSourceWithQueue():
            def generate_audio(self, nframes):
                driver.put_audio(([0],[0]))
                q.put('foo')
        driver.set_audio_source(AudioSourceWithQueue())
        driver.start()
        for _ in range(8):
            q.get()
        driver.close()

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

    def test_get_id(self):
        self._DriverClass.get_id()

