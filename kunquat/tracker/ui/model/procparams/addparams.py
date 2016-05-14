# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from kunquat.extras.sndfile import SndFileRMem, SndFileWMem

from .basewave import BaseWave
from .procparams import ProcParams


class AddParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'pitch',
            'in_01':  'force',
            'in_02':  'phmod L',
            'in_03':  'phmod R',
            'out_00': 'audio L',
            'out_01': 'audio R'
        }

    _TONES_MAX = 32

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_ramp_attack_enabled(self):
        return self._get_value('p_b_ramp_attack.json', True)

    def set_ramp_attack_enabled(self, enabled):
        self._set_value('p_b_ramp_attack.json', enabled)

    def _get_waveform_def_data(self):
        key = 'i_base.json'
        return self._get_value(key, None)

    def _get_waveform_data(self):
        key = 'p_base.wav'
        wav_data = self._get_value(key, None)
        if not wav_data:
            return None

        sf = SndFileRMem(wav_data)
        channels = sf.read()
        waveform = channels[0]
        sf.close()

        return waveform

    def _set_waveform_data(self, def_data, wave_data):
        def_key = 'i_base.json'
        self._set_value(def_key, def_data)

        waveform_key = 'p_base.wav'
        sf = SndFileWMem(channels=1, use_float=True, bits=32)
        sf.write(wave_data)
        sf.close()
        self._set_value(waveform_key, bytes(sf.get_file_contents()))

    def get_base_wave(self):
        return BaseWave(
                self._get_waveform_def_data,
                self._get_waveform_data,
                self._set_waveform_data)

    def _get_tone_subkey(self, index, subkey):
        return 'tone_{:02x}/{}'.format(index, subkey)

    def _get_tone_value(self, index, subkey, default_value):
        tone_subkey = self._get_tone_subkey(index, subkey)
        return self._get_value(tone_subkey, default_value)

    def _set_tone_value(self, index, subkey, value):
        tone_subkey = self._get_tone_subkey(index, subkey)
        self._set_value(tone_subkey, value)

    def _get_tone_pitch(self, index):
        default_pitch = 1 if (index == 0) else 0
        return self._get_tone_value(index, 'p_f_pitch.json', default_pitch)

    def _set_tone_pitch(self, index, pitch):
        self._set_tone_value(index, 'p_f_pitch.json', pitch)

    def _get_tone_volume(self, index):
        default_volume = 0 if (index == 0) else float('-inf')
        return self._get_tone_value(index, 'p_f_volume.json', default_volume)

    def _set_tone_volume(self, index, volume):
        self._set_tone_value(index, 'p_f_volume.json', volume)

    def _get_tone_panning(self, index):
        return self._get_tone_value(index, 'p_f_pan.json', 0)

    def _set_tone_panning(self, index, panning):
        self._set_tone_value(index, 'p_f_pan.json', panning)

    def _remove_tone(self, index):
        if index == 0:
            self._set_tone_pitch(index, 0)
        else:
            self._set_tone_pitch(index, None)

        self._set_tone_volume(index, None)
        self._set_tone_panning(index, None)

    def _set_all_tones(self, tones):
        # TODO: do this in one transaction
        for i, tone in enumerate(tones):
            pitch, volume, panning = tone
            self._set_tone_pitch(i, pitch)
            self._set_tone_volume(i, volume)
            self._set_tone_panning(i, panning)
        for i in range(len(tones), self._TONES_MAX):
            self._remove_tone(i)

    def _get_tone_existence(self, index):
        has_pitch = (self._get_tone_pitch(index) > 0)
        has_volume = not math.isinf(self._get_tone_volume(index))
        return (has_pitch and has_volume)

    def _get_tones_raw(self):
        tones_raw = []
        for i in range(self._TONES_MAX):
            if self._get_tone_existence(i):
                pitch = self._get_tone_pitch(i)
                volume = self._get_tone_volume(i)
                panning = self._get_tone_panning(i)
                tones_raw.append([pitch, volume, panning])
            else:
                tones_raw.append(None)
        return tones_raw

    def _get_tones_and_packing_info(self):
        tones = self._get_tones_raw()
        has_holes = (None in tones) and (
                tones.index(None) < sum(1 for t in tones if t != None))
        tones = [x for x in tones if x != None]
        return tones, has_holes

    def _get_tones(self):
        tones, _ = self._get_tones_and_packing_info()
        return tones

    def get_max_tone_count(self):
        return self._TONES_MAX

    def get_tone_count(self):
        return len(self._get_tones())

    def add_tone(self):
        tones, has_holes = self._get_tones_and_packing_info()
        if has_holes:
            tones.append([1, 0, 0])
            self._set_all_tones(tones)
        else:
            new_index = len(tones)
            self._set_tone_pitch(new_index, 1)
            self._set_tone_volume(new_index, 0)
            self._set_tone_panning(new_index, 0)

    def get_tone_pitch(self, index):
        return self._get_tones()[index][0]

    def set_tone_pitch(self, index, pitch):
        tones, has_holes = self._get_tones_and_packing_info()
        if has_holes:
            tones[index][0] = pitch
            self._set_all_tones(tones)
        else:
            self._set_tone_pitch(index, pitch)

    def get_tone_volume(self, index):
        return self._get_tones()[index][1]

    def set_tone_volume(self, index, volume):
        tones, has_holes = self._get_tones_and_packing_info()
        if has_holes:
            tones[index][1] = volume
            self._set_all_tones(tones)
        else:
            self._set_tone_volume(index, volume)

    def get_tone_panning(self, index):
        return self._get_tones()[index][2]

    def set_tone_panning(self, index, panning):
        tones, has_holes = self._get_tones_and_packing_info()
        if has_holes:
            tones[index][2] = panning
            self._set_all_tones(tones)
        else:
            self._set_tone_panning(index, panning)

    def remove_tone(self, index):
        tones, has_holes = self._get_tones_and_packing_info()
        if has_holes:
            del tones[index]
            self._set_all_tones(tones)
        else:
            self._remove_tone(index)


