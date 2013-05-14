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

import time

class Benchaudio():

    def __init__(self):
        self._started = False
        self._audio_source = None
        self._start = None
        self._previous = 0
        self._total = 0
        self._snapshot = 0

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def put_audio(self, audio_data):
        assert self._started
        current = time.time()
        if self._start == None:
            self._start = current
        elapsed = current - self._start
        (left, right) = audio_data
        samples = len(left)
        self._total += samples
        self._snapshot += samples
        if elapsed > 0 and current >= self._previous + 1:
            avg = int(self._total / elapsed)
            print '%s fps (%s avg)' % (self._snapshot, avg)
            self._snapshot = 0
            self._previous = current
        self._audio_source.acknowledge_audio()

    def start(self):
        self._started = True

    def stop(self):
        pass

    def close(self):
        self.stop()

    @classmethod
    def get_id(cls):
        return 'benchaudio'

    #'Silent benchmark driver'

