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

from kunquat.tracker.audio.audio_pump import AudioPump
from kunquat.extras.pulseaudio import Simple

class Pushaudio():

    def __init__(self):
        self._audio_pump = AudioPump()
        self._audio_pump.set_write_method(self._put_audio)
        self._pa = Simple('Kunquat Tracker',
                          'Editor output')

    def __del__(self):
        if self._audio_pump.is_alive():
            self_audio_pump.halt()
            self_audio_pump.join()

    def set_audio_source(self, audio_source):
        self._audio_pump.set_audio_source(audio_source)

    def _put_audio(self, audio_data):
        (left, right) = audio_data
        if len(left) > 0:
            self._pa.write(left, right)

    def put_audio(self, audio_data):
        self._audio_pump.put_audio(audio_data)

    def start(self):
        self._audio_pump.start()

    def stop(self):
        self._audio_pump.halt()
        self._audio_pump.join()

    def close(self):
        self.stop()
        del self._pa

    @classmethod
    def get_id(cls):
        return 'pushaudio'

    #'PulseAudio synchronic push driver'

