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

from kunquat.extras.pulseaudio import Simple

class Pushaudio():

    def __init__(self):
        self._audio_source = None
        self._pa = Simple('Kunquat Tracker',
                          'Editor output')

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def put_audio(self, audio):
        (left, right) = audio
        if len(left) > 0:
            self._pa.write(left, right)
        self._next()

    def _next(self):
        nframes = 2048
        if self._audio_source != None:
            self._audio_source.generate_audio(nframes)

    def start(self):
        self._next()

    def stop(self):
        pass

    def close(self):
        self.stop()


