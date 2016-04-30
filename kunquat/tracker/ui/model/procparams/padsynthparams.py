# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy

from .procparams import ProcParams


class PadsynthParams(ProcParams):

    _MIN_SAMPLE_LENGTH = 64
    _DEFAULT_SAMPLE_LENGTH = 262144
    _MAX_SAMPLE_LENGTH = 1048576

    _DEFAULT_AUDIO_RATE = 48000

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'pitch',
            'in_01':  'force',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def _get_applied_params(self):
        ret = {
            'sample_length': self._DEFAULT_SAMPLE_LENGTH,
            'audio_rate'   : self._DEFAULT_AUDIO_RATE,
            'harmonics'    : [[1, 1, 1]],
        }
        stored = self._get_value('p_ps_params.json', {})
        ret.update(stored)
        return ret

    def get_sample_length(self):
        return self._get_value('i_sample_length.json', self._DEFAULT_SAMPLE_LENGTH)

    def set_sample_length(self, length):
        self._set_value('i_sample_length.json', length)

    def get_audio_rate(self):
        return self._get_value('i_audio_rate.json', self._DEFAULT_AUDIO_RATE)

    def set_audio_rate(self, rate):
        self._set_value('i_audio_rate.json', rate)

    def _get_harmonics_data(self):
        hl = self._get_value('i_harmonics.json', [])
        if not hl:
            hl = [[1, 1, 1]]
        return hl

    def _set_harmonics_data(self, data):
        self._set_value('i_harmonics.json', data)

    def get_harmonics(self):
        return PadsynthHarmonics(self._get_harmonics_data, self._set_harmonics_data)

    def _get_config_params(self):
        return {
            'sample_length': self.get_sample_length(),
            'audio_rate'   : self.get_audio_rate(),
            'harmonics'    : self._get_harmonics_data(),
        }

    def is_config_applied(self):
        applied_params = self._get_applied_params()
        config_params = self._get_config_params()
        return (applied_params == config_params)

    def apply_config(self):
        new_params = deepcopy(self._get_config_params())
        self._set_value('p_ps_params.json', new_params)


class PadsynthHarmonics():

    def __init__(self, get_data, set_data):
        self._get_data = get_data
        self._set_data = set_data

    def get_count(self):
        data = self._get_data()
        return len(data)

    def append_harmonic(self):
        data = self._get_data()
        data.append([1, 1, 1])
        self._set_data(data)

    def remove_harmonic(self, index):
        data = self._get_data()
        del data[index]
        self._set_data(data)

    def _get_harmonic_data(self, index):
        data = self._get_data()
        return data[index]

    def _set_harmonic_data(self, index, hdata):
        data = self._get_data()
        data[index] = hdata
        self._set_data(data)

    def get_harmonic(self, index):
        return PadsynthHarmonic(index, self._get_harmonic_data, self._set_harmonic_data)


class PadsynthHarmonic():

    def __init__(self, index, get_data, set_data):
        self._index = index
        self._get_data = get_data
        self._set_data = set_data

    def get_freq_mul(self):
        return self._get_data(self._index)[0]

    def set_freq_mul(self, freq_mul):
        data = self._get_data(self._index)
        data[0] = freq_mul
        self._set_data(self._index, data)

    def get_amplitude(self):
        return self._get_data(self._index)[1]

    def set_amplitude(self, amplitude):
        data = self._get_data(self._index)
        data[1] = amplitude
        self._set_data(self._index, data)

    def get_bandwidth(self):
        return self._get_data(self._index)[2]

    def set_bandwidth(self, bandwidth):
        data = self._get_data(self._index)
        data[2] = bandwidth
        self._set_data(self._index, data)


