# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
import sys
import time
import Queue

from kunquat.extras.pulseaudio_async import Async

class Pulseaudio():

    def __init__(self):
        self._ag = None
        self._pa = Async(
                'Kunquat Tracker',
                'Editor output',
                self._pa_callback)
        self._buffer = Queue.Queue()
        self._pa.init()

    def set_audio_generator(self, ag):
        self._ag = ag

    def put_audio(self, audio):
        self._buffer.put(audio)

    def _pa_callback(self, nframes):
        self._ag.generate(nframes)
        audio_data = self._buffer.get()
        return audio_data

    def start(self):
        self._pa.play()

    def stop(self):
        self._pa.stop()

