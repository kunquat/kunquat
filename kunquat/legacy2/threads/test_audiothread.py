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
import threading

from kunquat.tracker.audio.audio_output import AudioOutput
from kunquat.tracker.audio.test_audio_output import TestAudioOutput
from audiothread import AudioThread
from test_abstractthread import TestAbstractThread


class DummyFrontend(threading.Thread):
    pass


class DummyBackend(threading.Thread):

    def update_selected_driver(self):
        pass


class TestAudiothread(TestAbstractThread, unittest.TestCase):

    def setUp(self):
        self._thread = AudioThread()


class TestThreadedAudio(TestAudioOutput):

    def setUp(self):
        handler = AudioOutput()
        self._audio_output = AudioThread()
        self._audio_output.set_handler(handler)
        self._audio_output.start()

    def tearDown(self):
        self._audio_output.halt()
        self._audio_output.join()


if __name__ == '__main__':
    unittest.main()


