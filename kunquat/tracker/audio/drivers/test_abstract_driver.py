# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from queue import Queue
import threading
import unittest
from time import sleep


class DummyAudioSource():
    def acknowledge_audio(self):
        pass


class TestAbstractDriver():

    def setUp(self):
        self._self._DriverClass = None
        self._cls_args = tuple()

    def test_threadleak_on_close(self):
        start = threading.active_count()
        driver = self._DriverClass(*self._cls_args)
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        sleep(0.2)
        driver.close()
        sleep(0.2)
        end = threading.active_count()
        self.assertEqual(start, end)

    def test_no_fake_acknowledge(self):
        driver = self._DriverClass(*self._cls_args)
        class AudioSourceWithQueue():
            def __init__(self):
                self.acks = 0
            def acknowledge_audio(self):
                self.acks += 1
        source = AudioSourceWithQueue()
        driver.set_audio_source(source)
        driver.start()
        sleep(0.2)
        self.assertEqual(source.acks, 0)
        driver.close()

    def test_audio_gets_acknowledged(self):
        q = Queue()
        driver = self._DriverClass(*self._cls_args)
        class AudioSourceWithQueue():
            def __init__(self):
                # in the real case there is no recursion
                # as the calls cross thread boundaries
                self.recursion_index = 0
                self.recursion_limit = 8
            def acknowledge_audio(self):
                if self.recursion_index > self.recursion_limit:
                    return
                self.recursion_index += 1
                q.put('foo')
                driver.put_audio([0, 0])
        driver.set_audio_source(AudioSourceWithQueue())
        driver.start()
        driver.put_audio([0, 0])
        for _ in range(8):
            q.get()
        driver.close()

    def test_emptypush(self):
        driver = self._DriverClass(*self._cls_args)
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        driver.put_audio([])
        driver.close()

    def test_quickpush(self):
        driver = self._DriverClass(*self._cls_args)
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        driver.put_audio([0, 0])
        driver.close()

    def test_prefeed(self):
        driver = self._DriverClass(*self._cls_args)
        driver.set_audio_source(DummyAudioSource())
        self.assertRaises(AssertionError, driver.put_audio, [0, 0])

    def _boot_driver(self):
        driver = self._DriverClass(*self._cls_args)
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        driver.close()

    def test_driver_cleanup(self):
        initial_threads = threading.active_count()
        for i in range(50):
            self._boot_driver()
        remaining_threads = threading.active_count()
        self.assertEqual(initial_threads, remaining_threads)

    def test_interrupt_driver(self):
        driver = self._DriverClass(*self._cls_args)
        driver.set_audio_source(DummyAudioSource())
        driver.start()
        sleep(0.2)
        driver.close()

    def test_get_id(self):
        self._DriverClass.get_id()


