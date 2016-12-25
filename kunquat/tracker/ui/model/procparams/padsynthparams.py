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


# Algorithmically based on the FFTPACK C implementation,
# source: http://www.netlib.org/fftpack/fft.c
def rfft(data):
    def get_workspace(n):
        wa = [0.0] * n
        ifac = [0] * 32

        ntryh = [4, 2, 3, 5]
        nf = 0

        # Divide n into preferred set of factors
        test_index = 0
        left = n
        while left > 1:
            ntry = ntryh[test_index] if test_index < 4 else test_index * 2 - 1

            nq = left // ntry
            nr = left - ntry * nq
            if nr != 0:
                test_index += 1
                continue

            nf += 1
            ifac[nf + 1] = ntry
            left = nq

            if (ntry == 2) and (nf != 1):
                for i in range(1, nf):
                    ib = nf - i + 1
                    ifac[ib + 1] = ifac[ib]
                ifac[2] = 2

        ifac[0] = n
        ifac[1] = nf

        # Fill in complex roots of unity
        nfm1 = nf - 1

        if nfm1 == 0:
            return ([0] * n) + wa, ifac

        argh = math.pi * 2 / n
        is_ = 0
        l1 = 1

        for k1 in range(nfm1):
            ip = ifac[k1 + 2]
            ld = 0
            l2 = l1 * ip
            ido = n // l2
            ipm = ip - 1

            for j in range(ipm):
                ld += l1
                i = is_
                argld = ld * argh
                fi = 0.0
                for ii in range(2, ido, 2):
                    fi += 1.0
                    arg = fi * argld
                    wa[i] = math.cos(arg)
                    wa[i + 1] = math.sin(arg)
                    i += 2
                is_ += ido
            l1 = l2

        return wa, ifac

    def drftf1(n, c, ch, wa, ifac):

        def dradf4(ido, l1, cc, ch, wa1, wa1o, wa2, wa2o, wa3, wa3o):
            t0 = l1 * ido

            t1 = t0
            t4 = t1 << 1
            t2 = t1 + (t1 << 1)
            t3 = 0

            for k in range(l1):
                tr1 = cc[t1] + cc[t2]
                tr2 = cc[t3] + cc[t4]
                t5 = t3 << 2
                ch[t5] = tr1 + tr2
                ch[(ido << 2) + t5 - 1] = tr2 - tr1
                t5 += (ido << 1)
                ch[t5 - 1] = cc[t3] - cc[t4]
                ch[t5] = cc[t2] - cc[t1]

                t1 += ido
                t2 += ido
                t3 += ido
                t4 += ido

            if ido < 2:
                return

            if ido != 2:
                t1 = 0
                for k in range(l1):
                    t2 = t1
                    t4 = t1 << 2
                    t6 = ido << 1
                    t5 = t6 + t4
                    for i in range(2, ido, 2):
                        t2 += 2
                        t3 = t2
                        t4 += 2
                        t5 -= 2

                        t3 += t0
                        cr2 = wa1[wa1o + i - 2] * cc[t3 - 1] + wa1[wa1o + i - 1] * cc[t3]
                        ci2 = wa1[wa1o + i - 2] * cc[t3] - wa1[wa1o + i - 1] * cc[t3 - 1]
                        t3 += t0
                        cr3 = wa2[wa2o + i - 2] * cc[t3 - 1] + wa2[wa2o + i - 1] * cc[t3]
                        ci3 = wa2[wa2o + i - 2] * cc[t3] - wa2[wa2o + i - 1] * cc[t3 - 1]
                        t3 += t0
                        cr4 = wa3[wa3o + i - 2] * cc[t3 - 1] + wa3[wa3o + i - 1] * cc[t3]
                        ci4 = wa3[wa3o + i - 2] * cc[t3] - wa3[wa3o + i - 1] * cc[t3 - 1]

                        tr1 = cr2 + cr4;
                        tr4 = cr4 - cr2;
                        ti1 = ci2 + ci4;
                        ti4 = ci2 - ci4;
                        ti2 = cc[t2] + ci3;
                        ti3 = cc[t2] - ci3;
                        tr2 = cc[t2 - 1] + cr3;
                        tr3 = cc[t2 - 1] - cr3;

                        ch[t4 - 1] = tr1 + tr2;
                        ch[t4] = ti1 + ti2;

                        ch[t5 - 1] = tr3 - ti4;
                        ch[t5] = tr4 - ti3;

                        ch[t4 + t6 - 1] = ti4 + tr3;
                        ch[t4 + t6] = tr4 + ti3;

                        ch[t5 + t6 - 1] = tr2 - tr1;
                        ch[t5 + t6] = ti1 - ti2;

                    t1 += ido

                if ido % 2 == 1:
                    return

            t1 = t0 + ido - 1
            t2 = t1 + (t0 << 1)
            t3 = ido << 2
            t4 = ido
            t5 = ido << 1
            t6 = ido

            hsqt2 = 0.70710678118654752440084436210485;

            for k in range(l1):
                ti1 = -hsqt2 * (cc[t1] + cc[t2])
                tr1 = hsqt2 * (cc[t1] - cc[t2])
                ch[t4 - 1] = tr1 + cc[t6 - 1];
                ch[t4 + t5 - 1] = cc[t6 - 1] - tr1;
                ch[t4] = ti1 - cc[t1 + t0];
                ch[t4 + t5] = ti1 + cc[t1 + t0];
                t1 += ido;
                t2 += ido;
                t4 += t3;
                t6 += ido;


        nf = ifac[1]
        na = 1
        l2 = n
        iw = n

        for k1 in range(nf):
            kh = nf - k1
            ip = ifac[kh + 1]
            l1 = l2 // ip
            ido = n // l2
            idl1 = ido * l1
            iw -= (ip - 1) * ido
            na = 1 - na

            if ip == 4:
                ix2 = iw + ido
                ix3 = ix2 + ido
                if na != 0:
                    dradf4(ido, l1, ch, c, wa, iw - 1, wa, ix2 - 1, wa, ix3 - 1)
                else:
                    dradf4(ido, l1, c, ch, wa, iw - 1, wa, ix2 - 1, wa, ix3 - 1)
            elif ip == 2:
                assert False # we shouldn't need this with n == 4096
            else:
                assert False # we shouldn't need this with n == 4096

            l2 = l1

        if na == 1:
            return

        for i in range(n):
            c[i] = ch[i]

    wsave, ifac = get_workspace(len(data))

    n = len(data)
    drftf1(n, data, [0.0] * n, wsave, ifac)


class PadsynthParams(ProcParams):

    _MIN_SAMPLE_LENGTH = 16384
    _DEFAULT_SAMPLE_LENGTH = 262144
    _MAX_SAMPLE_LENGTH = 1048576

    _DEFAULT_AUDIO_RATE = 48000

    _DEFAULT_BANDWIDTH_BASE = 1
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
            'sample_count'   : 1,
            'pitch_range'    : [0, 0],
            'center_pitch'   : 0,
            'bandwidth_base' : self._DEFAULT_BANDWIDTH_BASE,
            'bandwidth_scale': self._DEFAULT_BANDWIDTH_SCALE,
            'harmonics'      : [[1, 1]],
        }
        stored = self._get_value('p_ps_params.json', {})
        ret.update(stored)
        return ret

    def get_allowed_sample_lengths(self):
        sl = self._MIN_SAMPLE_LENGTH
        lengths = []
        while sl <= self._MAX_SAMPLE_LENGTH:
            lengths.append(sl)
            sl <<= 1
        return lengths

    def get_sample_length(self):
        return self._get_value('i_sample_length.json', self._DEFAULT_SAMPLE_LENGTH)

    def set_sample_length(self, length):
        self._set_value('i_sample_length.json', length)

    def get_audio_rate(self):
        return self._get_value('i_audio_rate.json', self._DEFAULT_AUDIO_RATE)

    def set_audio_rate(self, rate):
        self._set_value('i_audio_rate.json', rate)

    def get_sample_count(self):
        return self._get_value('i_sample_count.json', 1)

    def set_sample_count(self, count):
        self._set_value('i_sample_count.json', count)

    def get_sample_pitch_range(self):
        return self._get_value('i_pitch_range.json', [0, 0])

    def set_sample_pitch_range(self, min_pitch, max_pitch):
        self._set_value('i_pitch_range.json', [min_pitch, max_pitch])

    def get_sample_center_pitch(self):
        return self._get_value('i_center_pitch.json', 0)

    def set_sample_center_pitch(self, pitch):
        self._set_value('i_center_pitch.json', pitch)

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
                    fr = waveform[i * 2 - 1]
                    fi = waveform[i * 2]
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
        return self._get_value('i_bandwidth_base.json', self._DEFAULT_BANDWIDTH_BASE)

    def set_bandwidth_base(self, cents):
        self._set_value('i_bandwidth_base.json', cents)
        self._update_harmonics()

    def get_bandwidth_scale(self):
        return self._get_value('i_bandwidth_scale.json', self._DEFAULT_BANDWIDTH_SCALE)

    def set_bandwidth_scale(self, scale):
        self._set_value('i_bandwidth_scale.json', scale)
        self._update_harmonics()

    def _get_config_params(self):
        return {
            'sample_length'  : self.get_sample_length(),
            'audio_rate'     : self.get_audio_rate(),
            'sample_count'   : self.get_sample_count(),
            'pitch_range'    : self.get_sample_pitch_range(),
            'center_pitch'   : self.get_sample_center_pitch(),
            'bandwidth_base' : self.get_bandwidth_base(),
            'bandwidth_scale': self.get_bandwidth_scale(),
            'harmonics'      : self._get_harmonics_data(),
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

    def get_stereo_enabled(self):
        return self._get_value('p_b_stereo.json', False)

    def set_stereo_enabled(self, enabled):
        self._set_value('p_b_stereo.json', enabled)


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


