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

from .procparams import ProcParams


# Base wave functions

def wave_sine(x):
    return math.sin(x * math.pi)

def wave_triangle(x):
    if x < -0.5:
        return -x * 2 - 2
    elif x > 0.5:
        return -x * 2 + 2
    return x * 2

def wave_square(x):
    if x < 0:
        return -1
    return 1

def wave_saw(x):
    return x


# Warp functions

def normalise(x):
    return (x + 1) % 2 - 1

def pre_shift(x, amount):
    return x + amount

def pre_stretch(x, amount):
    amount *= 3
    if amount >= 0:
        if x >= 0:
            return x**(4**amount)
        else:
            return -((-x)**(4**amount))
    else:
        if x >= 0:
            return 1 - (1 - x)**(4**(-amount))
        else:
            return (1 + x)**(4**(-amount)) - 1

def pre_stretch_inverse(x, amount):
    amount *= 3
    if amount >= 0:
        if x >= 0:
            return 1 - (1 - x)**(4**(-amount))
        else:
            return (1 + x)**(4**(-amount)) - 1
    else:
        if x >= 0:
            return x**(4**amount)
        else:
            return -((-x)**(4**amount))

def pre_scale(x, amount):
    return x * 8**amount

def mod_y(y):
    y += 1
    if not 0 <= y <= 2:
        y -= 2 * math.floor(y / 2)
    return y - 1

def post_clip(y, amount):
    y *= 4**(amount + 1)
    return mod_y(y)

def post_mirror(y, amount):
    m = 4.0
    y = wave_triangle(normalise(y * m * (amount + 1 + 0.5 / m)))
    return mod_y(y)

def post_quantise(y, amount):
    amount = 2**(-(amount - 1.1) * 3)
    y *= amount - 1
    y = math.floor(y) + 0.5
    return y / amount

def post_shift(y, amount):
    return mod_y(y + amount)

def post_stretch(y, amount):
    return pre_stretch(y, amount * 2)

def post_stretch_asym(y, amount):
    amount *= 4
    if amount >= 0:
        y = (y + 1) / 2
        y = y**(4**amount)
        return y * 2 - 1
    else:
        y = (-y + 1) / 2
        y = y**(4**(-amount))
        return 1 - y * 2

def post_stretch_sine(y, amount):
    return math.sin(y * 2**((amount + 1) * 3))

def post_scaled_shift(y, amount):
    scale = 1 - abs(amount)
    offset = amount
    return (scale * y) + offset


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

    _PREWARP_FUNCS = [
            ('Scale', pre_scale),
            ('Shift', pre_shift),
            ('Stretch', pre_stretch),
            ('Stretch inv', pre_stretch_inverse),
        ]
    _PREWARP_FUNCS_DICT = dict(_PREWARP_FUNCS)

    _WAVEFORM_SAMPLE_COUNT = 4096
    _WAVEFORM_FUNCS = [
            ('Sine', wave_sine),
            ('Triangle', wave_triangle),
            ('Square', wave_square),
            ('Sawtooth', wave_saw),
        ]
    _WAVEFORM_FUNCS_DICT = dict(_WAVEFORM_FUNCS)

    _POSTWARP_FUNCS = [
            ('Clip', post_clip),
            ('Mirror', post_mirror),
            ('Quantise', post_quantise),
            ('Scaled shift', post_scaled_shift),
            ('Shift', post_shift),
            ('Stretch', post_stretch),
            ('Stretch asym', post_stretch_asym),
            ('Stretch sine', post_stretch_sine),
        ]
    _POSTWARP_FUNCS_DICT = dict(_POSTWARP_FUNCS)

    _WARPS_MAX = 8
    _TONES_MAX = 32

    _WAVES = {
            'base': {
                'waveform_key': 'p_base.wav',
                'waveform_funcs': _WAVEFORM_FUNCS,
                'waveform_funcs_dict': _WAVEFORM_FUNCS_DICT,
                'def_key': 'i_base.json',
                'tone_key_fmt': 'tone_{:02x}/{}',
            },
        }

    _WARPS = {
            'pre': {
                'funcs': _PREWARP_FUNCS,
                'funcs_dict': _PREWARP_FUNCS_DICT,
                'def_key': 'prewarps',
            },
            'post': {
                'funcs': _POSTWARP_FUNCS,
                'funcs_dict': _POSTWARP_FUNCS_DICT,
                'def_key': 'postwarps',
            },
        }

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_ramp_attack_enabled(self):
        return self._get_value('p_b_ramp_attack.json', True)

    def set_ramp_attack_enabled(self, enabled):
        self._set_value('p_b_ramp_attack.json', enabled)

    def get_waveform(self, wave_type):
        key = self._WAVES[wave_type]['waveform_key']
        wav_data = self._get_value(key, None)
        if wav_data:
            sf = SndFileRMem(wav_data)
            channels = sf.read()
            waveform = channels[0]
            sf.close()
        else:
            waveform = [-math.sin(phase * math.pi * 2 / self._WAVEFORM_SAMPLE_COUNT)
                    for phase in range(self._WAVEFORM_SAMPLE_COUNT)]

        if len(waveform) != self._WAVEFORM_SAMPLE_COUNT:
            waveform = waveform[:self._WAVEFORM_SAMPLE_COUNT]
            waveform.extend([0] * (self._WAVEFORM_SAMPLE_COUNT - len(waveform)))
        return waveform

    def get_waveform_func_names(self, wave_type):
        return [name for (name, _) in self._WAVES[wave_type]['waveform_funcs']]

    def _get_clean_warp_funcs(self, warps, funcs_dict):
        if type(warps) != list:
            warps = []
        warp_count = len(warps)
        for i in range(warp_count):
            warp = warps[i]
            if ((type(warp) != list) or
                    (len(warp) != 2) or
                    (warp[0] not in funcs_dict) or
                    (type(warp[1]) not in (int, float)) or
                    (not -1 <= warp[1] <= 1)):
                warps[i] = None
        warps = [w for w in warps if w != None]
        warps = warps[:self._WARPS_MAX]
        return warps

    def _get_waveform_def(self, wave_type):
        key = self._WAVES[wave_type]['def_key']
        base_def = self._get_value(key, None)
        if type(base_def) != dict:
            base_def = {}

        # Remove invalid entries
        pre_info = self._WARPS['pre']
        prewarps = base_def.get(pre_info['def_key'])
        base_def[pre_info['def_key']] = self._get_clean_warp_funcs(
                prewarps, pre_info['funcs_dict'])

        waveform_funcs_dict = self._WAVES[wave_type]['waveform_funcs_dict']
        if base_def.get('base_func', None) not in waveform_funcs_dict:
            waveform_key = self._WAVES[wave_type]['waveform_key']
            if not self._get_value(waveform_key, None):
                base_def['base_func'] = 'Sine'
            else:
                base_def['base_func'] = None

        post_info = self._WARPS['post']
        postwarps = base_def.get(post_info['def_key'])
        base_def[post_info['def_key']] = self._get_clean_warp_funcs(
                postwarps, post_info['funcs_dict'])

        return base_def

    def _update_waveform(self, wave_type, base_def):
        base = [0] * self._WAVEFORM_SAMPLE_COUNT
        waveform_funcs_dict = self._WAVES[wave_type]['waveform_funcs_dict']
        base_func = waveform_funcs_dict[base_def['base_func']]

        pre_info = self._WARPS['pre']
        prewarp_chain = [(pre_info['funcs_dict'][name], value)
                for (name, value) in base_def[pre_info['def_key']] if name != 'None']

        post_info = self._WARPS['post']
        postwarp_chain = [(post_info['funcs_dict'][name], value)
                for (name, value) in base_def[post_info['def_key']] if name != 'None']

        for i in range(self._WAVEFORM_SAMPLE_COUNT):
            x = i * 2 / float(self._WAVEFORM_SAMPLE_COUNT) - 1
            for (f, a) in prewarp_chain:
                x = normalise(f(x, a))
            y = base_func(x)
            for (f, a) in postwarp_chain:
                y = f(y, a)
            base[i] = y

        def_key = self._WAVES[wave_type]['def_key']
        self._set_value(def_key, base_def)

        waveform_key = self._WAVES[wave_type]['waveform_key']
        sf = SndFileWMem(channels=1, use_float=True, bits=32)
        sf.write(base)
        sf.close()
        self._set_value(waveform_key, bytes(sf.get_file_contents()))

    def get_waveform_func(self, wave_type):
        base_def = self._get_waveform_def(wave_type)
        return base_def['base_func']

    def set_waveform_func(self, wave_type, name):
        assert name in self._WAVES[wave_type]['waveform_funcs_dict']
        base_def = self._get_waveform_def(wave_type)
        base_def['base_func'] = name
        self._update_waveform(wave_type, base_def)

    def get_warp_func_names(self, warp_type):
        return [name for (name, _) in self._WARPS[warp_type]['funcs']]

    def get_max_warp_func_count(self):
        return self._WARPS_MAX

    def get_warp_func_count(self, wave_type, warp_type):
        base_def = self._get_waveform_def(wave_type)
        return len(base_def[self._WARPS[warp_type]['def_key']])

    def get_warp_func(self, wave_type, warp_type, index):
        base_def = self._get_waveform_def(wave_type)
        return tuple(base_def[self._WARPS[warp_type]['def_key']][index])

    def add_warp_func(self, wave_type, warp_type):
        base_def = self._get_waveform_def(wave_type)
        base_def[self._WARPS[warp_type]['def_key']].append(['Shift', 0])
        self._update_waveform(wave_type, base_def)

    def set_warp_func(self, wave_type, warp_type, index, name, arg):
        base_def = self._get_waveform_def(wave_type)
        base_def[self._WARPS[warp_type]['def_key']][index] = [name, arg]
        self._update_waveform(wave_type, base_def)

    def move_warp_func(self, wave_type, warp_type, from_index, to_index):
        base_def = self._get_waveform_def(wave_type)
        entry = base_def[self._WARPS[warp_type]['def_key']].pop(from_index)
        base_def[self._WARPS[warp_type]['def_key']].insert(to_index, entry)
        self._update_waveform(wave_type, base_def)

    def remove_warp_func(self, wave_type, warp_type, index):
        base_def = self._get_waveform_def(wave_type)
        del base_def[self._WARPS[warp_type]['def_key']][index]
        self._update_waveform(wave_type, base_def)

    def _get_tone_subkey(self, wave_type, index, subkey):
        return self._WAVES[wave_type]['tone_key_fmt'].format(index, subkey)

    def _get_tone_value(self, wave_type, index, subkey, default_value):
        tone_subkey = self._get_tone_subkey(wave_type, index, subkey)
        return self._get_value(tone_subkey, default_value)

    def _set_tone_value(self, wave_type, index, subkey, value):
        tone_subkey = self._get_tone_subkey(wave_type, index, subkey)
        self._set_value(tone_subkey, value)

    def _get_tone_pitch(self, wave_type, index):
        default_pitch = 1 if (index == 0) else 0
        return self._get_tone_value(wave_type, index, 'p_f_pitch.json', default_pitch)

    def _set_tone_pitch(self, wave_type, index, pitch):
        self._set_tone_value(wave_type, index, 'p_f_pitch.json', pitch)

    def _get_tone_volume(self, wave_type, index):
        default_volume = 0 if (index == 0) else float('-inf')
        return self._get_tone_value(wave_type, index, 'p_f_volume.json', default_volume)

    def _set_tone_volume(self, wave_type, index, volume):
        self._set_tone_value(wave_type, index, 'p_f_volume.json', volume)

    def _get_tone_panning(self, wave_type, index):
        assert wave_type == 'base'
        return self._get_tone_value(wave_type, index, 'p_f_pan.json', 0)

    def _set_tone_panning(self, wave_type, index, panning):
        assert wave_type == 'base'
        self._set_tone_value(wave_type, index, 'p_f_pan.json', panning)

    def _remove_tone(self, wave_type, index):
        if index == 0:
            self._set_tone_pitch(wave_type, index, 0)
        else:
            self._set_tone_pitch(wave_type, index, None)

        self._set_tone_volume(wave_type, index, None)
        if wave_type == 'base':
            self._set_tone_panning(wave_type, index, None)

    def _set_all_tones(self, wave_type, tones):
        # TODO: do this in one transaction
        for i, tone in enumerate(tones):
            pitch, volume, panning = tone
            self._set_tone_pitch(wave_type, i, pitch)
            self._set_tone_volume(wave_type, i, volume)
            if wave_type == 'base':
                self._set_tone_panning(wave_type, i, panning)
        for i in range(len(tones), self._TONES_MAX):
            self._remove_tone(wave_type, i)

    def _get_tone_existence(self, wave_type, index):
        has_pitch = (self._get_tone_pitch(wave_type, index) > 0)
        has_volume = not math.isinf(self._get_tone_volume(wave_type, index))
        return (has_pitch and has_volume)

    def _get_tones_raw(self, wave_type):
        tones_raw = []
        for i in range(self._TONES_MAX):
            if self._get_tone_existence(wave_type, i):
                pitch = self._get_tone_pitch(wave_type, i)
                volume = self._get_tone_volume(wave_type, i)
                if wave_type == 'base':
                    panning = self._get_tone_panning(wave_type, i)
                else:
                    panning = 0
                tones_raw.append([pitch, volume, panning])
            else:
                tones_raw.append(None)
        return tones_raw

    def _get_tones_and_packing_info(self, wave_type):
        tones = self._get_tones_raw(wave_type)
        has_holes = (None in tones) and (
                tones.index(None) < sum(1 for t in tones if t != None))
        tones = [x for x in tones if x != None]
        return tones, has_holes

    def _get_tones(self, wave_type):
        tones, _ = self._get_tones_and_packing_info(wave_type)
        return tones

    def get_max_tone_count(self):
        return self._TONES_MAX

    def get_tone_count(self, wave_type):
        return len(self._get_tones(wave_type))

    def add_tone(self, wave_type):
        tones, has_holes = self._get_tones_and_packing_info(wave_type)
        if has_holes:
            tones.append([1, 0, 0])
            self._set_all_tones(wave_type, tones)
        else:
            new_index = len(tones)
            self._set_tone_pitch(wave_type, new_index, 1)
            self._set_tone_volume(wave_type, new_index, 0)
            if wave_type == 'base':
                self._set_tone_panning(wave_type, new_index, 0)

    def get_tone_pitch(self, wave_type, index):
        return self._get_tones(wave_type)[index][0]

    def set_tone_pitch(self, wave_type, index, pitch):
        tones, has_holes = self._get_tones_and_packing_info(wave_type)
        if has_holes:
            tones[index][0] = pitch
            self._set_all_tones(wave_type, tones)
        else:
            self._set_tone_pitch(wave_type, index, pitch)

    def get_tone_volume(self, wave_type, index):
        return self._get_tones(wave_type)[index][1]

    def set_tone_volume(self, wave_type, index, volume):
        tones, has_holes = self._get_tones_and_packing_info(wave_type)
        if has_holes:
            tones[index][1] = volume
            self._set_all_tones(wave_type, tones)
        else:
            self._set_tone_volume(wave_type, index, volume)

    def get_tone_panning(self, wave_type, index):
        assert wave_type == 'base'
        return self._get_tones(wave_type)[index][2]

    def set_tone_panning(self, wave_type, index, panning):
        assert wave_type == 'base'
        tones, has_holes = self._get_tones_and_packing_info(wave_type)
        if has_holes:
            tones[index][2] = panning
            self._set_all_tones(wave_type, tones)
        else:
            self._set_tone_panning(wave_type, index, panning)

    def remove_tone(self, wave_type, index):
        tones, has_holes = self._get_tones_and_packing_info(wave_type)
        if has_holes:
            del tones[index]
            self._set_all_tones(wave_type, tones)
        else:
            self._remove_tone(wave_type, index)


