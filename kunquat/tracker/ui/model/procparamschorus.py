# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from procparams import ProcParams


class ProcParamsChorus(ProcParams):

    _VOICES_MAX = 32

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_max_voice_count(self):
        return self._VOICES_MAX

    def _get_voice_subkey(self, index, subkey):
        return 'voice_{:02x}/{}'.format(index, subkey)

    def _get_voice_value(self, index, subkey, default_value):
        voice_subkey = self._get_voice_subkey(index, subkey)
        return self._get_value(voice_subkey, default_value)

    def _set_voice_value(self, index, subkey, value):
        voice_subkey = self._get_voice_subkey(index, subkey)
        self._set_value(voice_subkey, value)

    def _get_voice_delay(self, index):
        default_delay = -1
        return self._get_voice_value(index, 'p_f_delay.json', default_delay)

    def _set_voice_delay(self, index, delay):
        self._set_voice_value(index, 'p_f_delay.json', delay)

    def _get_voice_range(self, index):
        default_range = 0
        return self._get_voice_value(index, 'p_f_range.json', default_range)

    def _set_voice_range(self, index, vrange):
        self._set_voice_value(index, 'p_f_range.json', vrange)

    def _get_voice_speed(self, index):
        default_speed = 0
        return self._get_voice_value(index, 'p_f_speed.json', default_speed)

    def _set_voice_speed(self, index, speed):
        self._set_voice_value(index, 'p_f_speed.json', speed)

    def _get_voice_volume(self, index):
        default_volume = 0
        return self._get_voice_value(index, 'p_f_volume.json', default_volume)

    def _set_voice_volume(self, index, volume):
        self._set_voice_value(index, 'p_f_volume.json', volume)

    def _set_all_voices(self, voices):
        # TODO: do this in one transaction
        for i, voice in enumerate(voices):
            delay, vrange, speed, volume = voice
            self._set_voice_delay(i, delay)
            self._set_voice_range(i, vrange)
            self._set_voice_speed(i, speed)
            self._set_voice_volume(i, volume)
        for i in xrange(len(voices), self._VOICES_MAX):
            self._remove_voice(i)

    def _get_voice_existence(self, index):
        has_delay = (self._get_voice_delay(index) >= 0)
        has_range = (self._get_voice_range(index) >= 0)
        has_speed = (self._get_voice_speed(index) >= 0)
        has_volume = not math.isinf(self._get_voice_volume(index))
        return (has_delay and has_range and has_speed and has_volume)

    def _get_voices_raw(self):
        voices_raw = []
        for i in xrange(self._VOICES_MAX):
            if self._get_voice_existence(i):
                delay = self._get_voice_delay(i)
                vrange = self._get_voice_range(i)
                speed = self._get_voice_speed(i)
                volume = self._get_voice_volume(i)
                voices_raw.append([delay, vrange, speed, volume])
            else:
                voices_raw.append(None)
        return voices_raw

    def _get_voices_and_packing_info(self):
        voices = self._get_voices_raw()
        has_holes = (None in voices) and (
                voices.index(None) < sum(1 for v in voices if v != None))
        voices = filter(lambda x: x != None, voices)
        return voices, has_holes

    def _get_voices(self):
        voices, _ = self._get_voices_and_packing_info()
        return voices

    def add_voice(self):
        voices, has_holes = self._get_voices_and_packing_info()
        if has_holes:
            voices.append([0, 0, 0, 0])
            self._set_all_voices(voices)
        else:
            new_index = len(voices)
            self._set_voice_delay(new_index, 0)
            self._set_voice_range(new_index, 0)
            self._set_voice_speed(new_index, 0)
            self._set_voice_volume(new_index, 0)

    def get_voice_count(self):
        return len(self._get_voices())

    def get_voice_delay(self, index):
        return self._get_voices()[index][0]

    def set_voice_delay(self, index, delay):
        voices, has_holes = self._get_voices_and_packing_info()
        if has_holes:
            voices[index][0] = delay
            self._set_all_voices(voices)
        else:
            self._set_voice_delay(index, delay)

    def get_voice_range(self, index):
        return self._get_voices()[index][1]

    def set_voice_range(self, index, vrange):
        voices, has_holes = self._get_voices_and_packing_info()
        if has_holes:
            voices[index][1] = vrange
            self._set_all_voices(voices)
        else:
            self._set_voice_range(index, vrange)

    def get_voice_speed(self, index):
        return self._get_voices()[index][2]

    def set_voice_speed(self, index, speed):
        voices, has_holes = self._get_voices_and_packing_info()
        if has_holes:
            voices[index][2] = speed
            self._set_all_voices(voices)
        else:
            self._set_voice_speed(index, speed)

    def get_voice_volume(self, index):
        return self._get_voices()[index][3]

    def set_voice_volume(self, index, volume):
        voices, has_holes = self._get_voices_and_packing_info()
        if has_holes:
            voices[index][3] = volume
            self._set_all_voices(voices)
        else:
            self._set_voice_volume(index, volume)

    def _remove_voice(self, index):
        self._set_voice_delay(index, None)
        self._set_voice_range(index, None)
        self._set_voice_speed(index, None)
        self._set_voice_volume(index, None)

    def remove_voice(self, index):
        voices, has_holes = self._get_voices_and_packing_info()
        if has_holes:
            del voices[index]
            self._set_all_voices(voice)
        else:
            self._remove_voice(index)


