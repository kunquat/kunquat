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


class Audio():

    def __init__(self):
        self._backend = None
        self._driver = None

    # Audio generator interface

    def generate(self, nframes):
        self._backend.generate_audio(nframes)

    # Audio output interface

    def select_driver(self, name):
        if name == 'pulse':
            self._driver = Pulseaudio()
        else:
            assert False
        self._driver.set_audio_generator(self)
        self._driver.start()

    def set_backend(self, backend):
        self._backend = backend

    def put_audio(self, audio):
        self._driver.put_audio(audio)
            

