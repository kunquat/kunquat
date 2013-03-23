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

from kunquat.extras.pulseaudio import Simple

class Pushaudio():

    def __init__(self):
        self._ag = None
        self._pa = Simple('Kunquat Tracker',
                          'Editor output')

    def set_audio_generator(self, ag):
        self._ag = ag

    def put_audio(self, audio):
        (left, right) = audio
        self._pa.write(left, right)
        self.next()

    def next(self):
        nframes = 20
        if self._ag != None:
            self._ag.generate(nframes)

    def start(self):
        self.next()

    def stop(self):
        pass
