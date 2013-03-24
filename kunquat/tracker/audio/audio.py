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

from drivers.pulseaudio import Pulseaudio
from drivers.pushaudio import Pushaudio

D_NONE = 'none'
D_PULSE = 'pulse'
D_PUSH = 'push'

drivers = {
  D_NONE: {'name': 'no driver'},
  D_PULSE: {'name': 'PulseAudio asynchronic pull driver'},
  D_PUSH: {'name': 'PulseAudio synchronic push driver'}
}

class Audio():

    def __init__(self):
        self._backend = None
        self._frontend = None
        self._driver = None

    def init(self):
        self._frontend.update_drivers(drivers)

    # Audio generator interface

    def generate(self, nframes):
        self._backend.generate_audio(nframes)

    # Audio output interface

    def select_driver(self, name):
        if self._driver:
            self._driver.close()
        if name == D_NONE:
            self._driver = None
        elif name == D_PULSE:
            self._driver = Pulseaudio()
        elif name == D_PUSH:
            self._driver = Pushaudio()
        else:
            assert False
        if self._driver:
            self._driver.set_audio_generator(self)
            self._driver.start()

    def set_backend(self, backend):
        self._backend = backend

    def set_frontend(self, frontend):
        self._frontend = frontend

    def put_audio(self, audio):
        if self._driver:
            self._driver.put_audio(audio)
            

