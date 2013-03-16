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
        yield math.sin(phase)


class Backend():

    def __init__(self):
        self._ap = None
        self._ep = None

        self._sine = gen_sine(48000)

    def process_command(self, cmd):
        if cmd.name == 'generate':
            data_mono = list(islice(self._sine, cmd.arg))
            data = (data_mono, data_mono)
            self._ap(Command('audio', data))

    def set_audio_processor(self, ap):
        self._ap = ap

    def set_event_processor(self, ep):
        """storage, environment and tracker events"""
        self._ep = ep


