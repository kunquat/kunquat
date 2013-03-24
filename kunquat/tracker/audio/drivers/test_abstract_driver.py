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

import threading
import unittest
from time import sleep

class TestAbstractDriver(unittest.TestCase):

    def setUp(self):
        pass

    def _quickpush(self, DriverClass):
        driver = DriverClass()
        driver.put_audio(([0],[0]))

    def _prefeed(self, DriverClass):
        driver = DriverClass()
        class gen():
            def generate(self,foo):
                pass
        driver.set_audio_generator(gen())
        driver.put_audio((10000*[0.1],10000*[0.1]))
        driver.start()
        sleep(1)
        driver.put_audio((10000*[0.1],10000*[0.1]))
        driver.stop()

    def _boot_driver(self, DriverClass):
        driver = DriverClass()
        class gen():
            def generate(self,foo):
                pass
        driver.set_audio_generator(gen())
        driver.start()
        driver.close()

    def _driver_cleanup(self, DriverClass):
        initial_threads = threading.active_count()
        for i in xrange(50):
            self._boot_driver(DriverClass)
        remaining_threads = threading.active_count()
        self.assertEqual(initial_threads, remaining_threads)

    def run_tests(self, DriverClass):
        self._prefeed(DriverClass)
        self._quickpush(DriverClass)
        self._driver_cleanup(DriverClass)


