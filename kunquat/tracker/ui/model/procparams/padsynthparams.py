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

import math
from copy import deepcopy

from kunquat.extras.sndfile import SndFileRMem, SndFileWMem

from .basewave import BaseWave
from .procparams import ProcParams


def bit_reversal_permute(data):
    bit_count = int(round(math.log(len(data), 2)))
    assert bit_count < 32

    def reverse_bits(n):
        n = ((n & 0xaaaaaaaa) >> 1) | ((n & 0x55555555) << 1)
        n = ((n & 0xcccccccc) >> 2) | ((n & 0x33333333) << 2)
        n = ((n & 0xf0f0f0f0) >> 4) | ((n & 0x0f0f0f0f) << 4)
        n = ((n & 0xff00ff00) >> 8) | ((n & 0x00ff00ff) << 8)
        n = ((n & 0xffff0000) >> 16) | ((n & 0x0000ffff) << 16)
        return n >> (32 - bit_count)

    for i in range(len(data)):
        rev_i = reverse_bits(i)
        if rev_i > i:
            data[i], data[rev_i] = data[rev_i], data[i]


def W(index, count):
    u = 2 * math.pi * index / count
    return complex(math.cos(u), math.sin(u))


def rfft(data):
    bit_reversal_permute(data)

    bit_count = int(round(math.log(len(data), 2)))
    for i in range(1, bit_count + 1):
        p_i = 1 << i
        p_im1 = p_i >> 1
        q_i = len(data) // p_i

        for b in range(q_i):
            ix0 = b * p_i
            ix1 = b * p_i + p_im1
            data[ix0], data[ix1] = data[ix0] + data[ix1], data[ix0] - data[ix1]

        for a in range(1, p_im1 >> 1):
            for b in range(q_i):
                bp_i = b * p_i

                z0 = complex(data[bp_i + a], data[bp_i + p_im1 - a])
                z1 = complex(data[bp_i + p_im1 + a], data[bp_i + p_i - a])
                Wa_p_iz1 = W(a, p_i) * z1
                t0 = z0 + Wa_p_iz1
                t1 = z0 - Wa_p_iz1
                data[bp_i + a], data[bp_i + p_i - a] = t0.real, t0.imag
                data[bp_i + p_im1 - a], data[bp_i + p_im1 + a] = t1.real, -t1.imag


class PadsynthParams(ProcParams):

    _MIN_SAMPLE_LENGTH = 64
    _DEFAULT_SAMPLE_LENGTH = 262144
    _MAX_SAMPLE_LENGTH = 1048576

    _DEFAULT_AUDIO_RATE = 48000

    _DEFAULT_BANDWIDTH_BASE = 0.1
    _DEFAULT_BANDWIDTH_SCALE = 1

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
            'sample_length'  : self._DEFAULT_SAMPLE_LENGTH,
            'audio_rate'     : self._DEFAULT_AUDIO_RATE,
            'bandwidth_base' : self._DEFAULT_BANDWIDTH_BASE,
            'bandwidth_scale': self._DEFAULT_BANDWIDTH_SCALE,
            'harmonics'      : [[1, 1]],
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

    def _get_harmonics_wave_def_data(self):
        key = 'i_harmonics_base.json'
        return self._get_value(key, None)

    def _get_harmonics_wave_data(self):
        key = 'i_harmonics_base.wav'
        wav_data = self._get_value(key, None)
        if not wav_data:
            return None

        sf = SndFileRMem(wav_data)
        channels = sf.read()
        waveform = channels[0]
        sf.close()

        return waveform

    def _set_harmonics_wave_data(self, def_data, wave_data):
        def_key = 'i_harmonics_base.json'
        self._set_value(def_key, def_data)

        waveform_key = 'i_harmonics_base.wav'
        sf = SndFileWMem(channels=1, use_float=True, bits=32)
        sf.write(wave_data)
        sf.close()
        self._set_value(waveform_key, bytes(sf.get_file_contents()))

        self._update_harmonics()

    def get_harmonics_wave(self):
        return BaseWave(
                self._get_harmonics_wave_def_data,
                self._get_harmonics_wave_data,
                self._set_harmonics_wave_data)

    def _update_harmonics(self):
        bw_base = self.get_bandwidth_base()
        bw_scale = self.get_bandwidth_scale()

        waveform = self._get_harmonics_wave_data()
        if waveform:
            rfft(waveform)
            hl = []
            for freq_mult, amp_mult in self._get_harmonic_scales_data():
                for i in range(1, len(waveform) // 2):
                    fr = waveform[i]
                    fi = waveform[-i]
                    amplitude = math.sqrt(fr * fr + fi * fi)
                    hl.append([i * freq_mult, amplitude * amp_mult])
        else:
            hl = []
            for freq_mult, amp_mult in self._get_harmonic_scales_data():
                hl.append([freq_mult, amp_mult])

        self._set_value('i_harmonics.json', hl)

    def _get_harmonics_data(self):
        hl = self._get_value('i_harmonics.json', [])
        if not hl:
            hl = [[1, 1]]
        return hl

    def _get_harmonic_scales_data(self):
        mults = self._get_value('i_harmonic_scales.json', None)
        if not mults:
            return [[1, 1]]
        return mults

    def _set_harmonic_scales_data(self, data):
        self._set_value('i_harmonic_scales.json', data)
        self._update_harmonics()

    def get_harmonic_scales(self):
        return HarmonicScales(
                self._get_harmonic_scales_data, self._set_harmonic_scales_data)

    def get_bandwidth_base(self):
        return self._get_value('i_bandwidth_base.json', 1)

    def set_bandwidth_base(self, cents):
        self._set_value('i_bandwidth_base.json', cents)
        self._update_harmonics()

    def get_bandwidth_scale(self):
        return self._get_value('i_bandwidth_scale.json', 1)

    def set_bandwidth_scale(self, scale):
        self._set_value('i_bandwidth_scale.json', scale)
        self._update_harmonics()

    def _get_config_params(self):
        return {
            'sample_length'  : self.get_sample_length(),
            'audio_rate'     : self.get_audio_rate(),
            'harmonics'      : self._get_harmonics_data(),
            'bandwidth_base' : self.get_bandwidth_base(),
            'bandwidth_scale': self.get_bandwidth_scale(),
        }

    def is_config_applied(self):
        applied_params = self._get_applied_params()
        config_params = self._get_config_params()
        return (applied_params == config_params)

    def apply_config(self):
        new_params = deepcopy(self._get_config_params())
        self._set_value('p_ps_params.json', new_params)

    def get_ramp_attack_enabled(self):
        return self._get_value('p_b_ramp_attack.json', True)

    def set_ramp_attack_enabled(self, enabled):
        self._set_value('p_b_ramp_attack.json', enabled)


class HarmonicScales():

    def __init__(self, get_data, set_data):
        self._get_data = get_data
        self._set_data = set_data

    def get_count(self):
        data = self._get_data()
        return len(data)

    def append_scale(self):
        data = self._get_data()
        data.append([len(data) + 1, 1])
        self._set_data(data)

    def remove_scale(self, index):
        data = self._get_data()
        del data[index]
        self._set_data(data)

    def _get_scale_data(self, index):
        data = self._get_data()
        return data[index]

    def _set_scale_data(self, index, hdata):
        data = self._get_data()
        data[index] = hdata
        self._set_data(data)

    def get_scale(self, index):
        return HarmonicScale(index, self._get_scale_data, self._set_scale_data)


class HarmonicScale():

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


