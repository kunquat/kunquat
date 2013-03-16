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

import time

from kunquat.extras.pulseaudio_async import Async

class Pulseaudio():

    def __init__(self):
        self._ag = None
        self._pa = Async(
                'Kunquat Tracker',
                'Editor output',
                self.get_audio)
        self._pa.init()

    def set_audio_generator(self, ag):
        self._ag = ag

    def get_audio(self, nframes):
        audio_data = self._ag(nframes)
        return audio_data or ([], [])

    def start(self):
        time.sleep(1)
        self._pa.play()


