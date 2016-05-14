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


class BaseWave():

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

    def __init__(self, get_waveform_def_data, get_waveform_data, set_data):
        self._get_waveform_def_data = get_waveform_def_data
        self._get_waveform_data = get_waveform_data
        self._set_data = set_data

    def get_waveform(self):
        waveform = self._get_waveform_data()
        if not waveform:
            waveform = [-math.sin(phase * math.pi * 2 / self._WAVEFORM_SAMPLE_COUNT)
                    for phase in range(self._WAVEFORM_SAMPLE_COUNT)]

        if len(waveform) != self._WAVEFORM_SAMPLE_COUNT:
            waveform = waveform[:self._WAVEFORM_SAMPLE_COUNT]
            waveform.extend([0] * (self._WAVEFORM_SAMPLE_COUNT - len(waveform)))
        return waveform

    def get_waveform_func_names(self):
        return [name for (name, _) in self._WAVEFORM_FUNCS]

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

    def _get_waveform_def(self):
        base_def = self._get_waveform_def_data()
        if type(base_def) != dict:
            base_def = {}

        # Remove invalid entries
        pre_info = self._WARPS['pre']
        prewarps = base_def.get(pre_info['def_key'])
        base_def[pre_info['def_key']] = self._get_clean_warp_funcs(
                prewarps, pre_info['funcs_dict'])

        waveform_funcs_dict = self._WAVEFORM_FUNCS_DICT
        if base_def.get('base_func', None) not in waveform_funcs_dict:
            if not self._get_waveform_data():
                base_def['base_func'] = 'Sine'
            else:
                base_def['base_func'] = None

        post_info = self._WARPS['post']
        postwarps = base_def.get(post_info['def_key'])
        base_def[post_info['def_key']] = self._get_clean_warp_funcs(
                postwarps, post_info['funcs_dict'])

        return base_def

    def _update_waveform(self, base_def):
        base = [0] * self._WAVEFORM_SAMPLE_COUNT
        waveform_funcs_dict = self._WAVEFORM_FUNCS_DICT
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

        self._set_data(base_def, base)

    def get_waveform_func(self):
        base_def = self._get_waveform_def()
        return base_def['base_func']

    def set_waveform_func(self, name):
        assert name in self._WAVEFORM_FUNCS_DICT
        base_def = self._get_waveform_def()
        base_def['base_func'] = name
        self._update_waveform(base_def)

    def get_warp_func_names(self, warp_type):
        return [name for (name, _) in self._WARPS[warp_type]['funcs']]

    def get_max_warp_func_count(self):
        return self._WARPS_MAX

    def get_warp_func_count(self, warp_type):
        base_def = self._get_waveform_def()
        return len(base_def[self._WARPS[warp_type]['def_key']])

    def get_warp_func(self, warp_type, index):
        base_def = self._get_waveform_def()
        return tuple(base_def[self._WARPS[warp_type]['def_key']][index])

    def add_warp_func(self, warp_type):
        base_def = self._get_waveform_def()
        base_def[self._WARPS[warp_type]['def_key']].append(['Shift', 0])
        self._update_waveform(base_def)

    def set_warp_func(self, warp_type, index, name, arg):
        base_def = self._get_waveform_def()
        base_def[self._WARPS[warp_type]['def_key']][index] = [name, arg]
        self._update_waveform(base_def)

    def move_warp_func(self, warp_type, from_index, to_index):
        base_def = self._get_waveform_def()
        entry = base_def[self._WARPS[warp_type]['def_key']].pop(from_index)
        base_def[self._WARPS[warp_type]['def_key']].insert(to_index, entry)
        self._update_waveform(base_def)

    def remove_warp_func(self, warp_type, index):
        base_def = self._get_waveform_def()
        del base_def[self._WARPS[warp_type]['def_key']][index]
        self._update_waveform(base_def)


