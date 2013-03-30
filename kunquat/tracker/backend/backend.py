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

from itertools import islice
import math
from kunquat import Kunquat


def gen_sine(rate):
    phase = 0
    while True:
        phase += 440 * 2 * math.pi / rate
        phase %= 2 * math.pi
        yield math.sin(phase) * 0.3


class Backend():

    def __init__(self):
        self._audio_output = None
        self._frontend = None
        self._kunquat = Kunquat()

        self._sine = gen_sine(48000)

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def set_frontend(self, frontend):
        self._frontend = frontend

    def set_data(self, key, value):
        self._kunquat.set_data(key, value)

    def update_selected_driver(self, name):
        pass

    def commit_data(self):
        self._kunquat.validate()

    def generate_audio(self, nframes):
        #data_mono = list(islice(self._sine, nframes))
        #audio_data = (data_mono, data_mono)
        audio_data = self._kunquat.mix(nframes)
        self._audio_output.put_audio(audio_data)


