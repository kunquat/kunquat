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

from kunquat.tracker.command import Command


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

        self._sine = gen_sine(48000)

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def set_frontend(self, frontend):
        self._frontend = frontend

    def generate_audio(self, nframes):
        data_mono = list(islice(self._sine, nframes))
        data = (data_mono, data_mono)
        self._audio_output.put_audio(data)


