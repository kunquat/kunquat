# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.extras.pulseaudio import Simple


class Pushaudio():

    def __init__(self, audio_rate, latency):
        self._started = False
        self._audio_source = None
        self._pa = Simple(
                'Kunquat Tracker', 'Editor output', rate=audio_rate, latency=latency)

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def put_audio(self, audio_data):
        assert self._started
        (left, right) = audio_data
        if len(left) > 0:
            self._pa.write(left, right)
        self._audio_source.acknowledge_audio()

    def start(self):
        self._started = True

    def stop(self):
        pass

    def close(self):
        self.stop()
        del self._pa

    @classmethod
    def get_id(cls):
        return 'pushaudio'

    #'PulseAudio synchronous push driver'


