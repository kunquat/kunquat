# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from kunquat.kunquat.kunquat import get_default_value


class Generator():

    def __init__(self, ins_id, gen_id):
        assert ins_id
        assert gen_id
        self._ins_id = ins_id
        self._gen_id = gen_id
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def _get_key(self, subkey):
        return '{}/{}/{}'.format(self._ins_id, self._gen_id, subkey)

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)

    def get_out_ports(self):
        out_ports = []
        for i in xrange(0x100):
            port_id = 'out_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                out_ports.append(port_id)

        return out_ports

    def get_name(self):
        key = self._get_key('m_name.json')
        return self._store.get(key)

    def set_name(self, name):
        key = self._get_key('m_name.json')
        self._store[key] = name

    def get_type(self):
        key = self._get_key('p_gen_type.json')
        return self._store.get(key)

    def get_type_params(self):
        types = { 'add': GeneratorParamsAdd }
        cons = types[self.get_type()]
        return cons(self._ins_id, self._gen_id, self._controller)


class GeneratorParams():

    def __init__(self, ins_id, gen_id, controller):
        self._key_prefix = '{}/{}/'.format(ins_id, gen_id)
        self._controller = controller
        self._store = controller.get_store()

    def _get_key(self, impl_or_conf, subkey):
        assert impl_or_conf in ('i/', 'c/')
        return ''.join((self._key_prefix, impl_or_conf, subkey))

    # Protected interface

    def _get_value(self, subkey, default_value):
        conf_key = self._get_key('c/', subkey)
        if conf_key in self._store:
            return self._store[conf_key]
        impl_key = self._get_key('i/', subkey)
        return self._store.get(impl_key, default_value)

    def _set_value(self, subkey, value):
        key = self._get_key('c/', subkey)
        self._store[key] = value


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
    amount *= 2
    if x < 0:
        return -((-x)**(4**amount))
    return x**(4**amount)

def pre_stretch_asym(x, amount):
    amount *= 2
    x = (x + 1) / 2
    x = x**(4**amount)
    return x * 2 - 1

def pre_scale(x, amount):
    return x * 8**amount

def pre_sine_shift(x, amount):
    return math.sin(x * 6**(amount + 0.5))

def mod_y(y):
    y += 1
    if not 0 <= y <= 2:
        y -= 2 * math.floor(y / 2)
    return y - 1

def post_clip(y, amount):
    y *= 8**amount
    return mod_y(y)

def post_shift(y, amount):
    return mod_y(y + amount)

def post_stretch(y, amount):
    return pre_stretch(y, amount * 2)

def post_stretch_asym(y, amount):
    return pre_stretch_asym(y, amount * 2)

def post_quantise(y, amount):
    amount = 2**(-(amount - 1.3) * 4)
    y *= amount - 1
    y = math.floor(y) + 0.5
    return y / amount


class GeneratorParamsAdd(GeneratorParams):

    _PREWARP_FUNCS = [
            ('None', None),
            ('Shift', pre_shift),
            ('Stretch', pre_stretch),
            ('Stretch asym', pre_stretch_asym),
            ('Scale', pre_scale),
            ('Sine shift', pre_sine_shift),
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
            ('None', None),
            ('Clip', post_clip),
            ('Shift', post_shift),
            ('Stretch', post_stretch),
            ('Stretch asym', post_stretch_asym),
            ('Quantise', post_quantise),
        ]
    _POSTWARP_FUNCS_DICT = dict(_POSTWARP_FUNCS)

    def __init__(self, ins_id, gen_id, controller):
        GeneratorParams.__init__(self, ins_id, gen_id, controller)

    def get_base_waveform(self):
        base = self._get_value('p_ln_base.json', None)
        if not base:
            base = [-math.sin(phase * math.pi * 2 / self._WAVEFORM_SAMPLE_COUNT)
                    for phase in xrange(self._WAVEFORM_SAMPLE_COUNT)]
        if len(base) != self._WAVEFORM_SAMPLE_COUNT:
            base = base[:self._WAVEFORM_SAMPLE_COUNT]
            base.extend([0] * (self._WAVEFORM_SAMPLE_COUNT - len(base)))
        return base

    def get_base_waveform_func_names(self):
        return [name for (name, _) in self._WAVEFORM_FUNCS]

    def _get_clean_warp_funcs(self, warps, funcs_dict):
        if type(warps) != list:
            warps = []
        warp_count = len(warps)
        for i in xrange(warp_count):
            warp = warps[i]
            if ((type(warp) != list) or
                    (len(warp) != 2) or
                    (warp[0] not in funcs_dict) or
                    (type(warp[1]) not in (int, float)) or
                    (not -1 <= warp[1] <= 1)):
                warps[i] = None
        warps = filter(lambda w: w != None, warps)
        return warps

    def _get_base_waveform_def(self):
        base_def = self._get_value('i_base.json', None)
        if type(base_def) != dict:
            base_def = {}

        # Replace invalid entries with defaults
        prewarps = base_def.get('prewarps')
        base_def['prewarps'] = self._get_clean_warp_funcs(
                prewarps, self._PREWARP_FUNCS_DICT)

        if base_def.get('base_func', None) not in self._WAVEFORM_FUNCS_DICT:
            if not self._get_value('p_ln_base.json', None):
                base_def['base_func'] = 'Sine'
            else:
                base_def['base_func'] = None

        postwarps = base_def.get('postwarps')
        base_def['postwarps'] = self._get_clean_warp_funcs(
                postwarps, self._POSTWARP_FUNCS_DICT)

        return base_def

    def _update_base_waveform(self, base_def):
        base = [0] * self._WAVEFORM_SAMPLE_COUNT
        base_func = self._WAVEFORM_FUNCS_DICT[base_def['base_func']]

        prewarp_chain = [(self._PREWARP_FUNCS_DICT[name], value)
                for (name, value) in base_def['prewarps'] if name != 'None']

        postwarp_chain = [(self._POSTWARP_FUNCS_DICT[name], value)
                for (name, value) in base_def['postwarps'] if name != 'None']

        for i in xrange(self._WAVEFORM_SAMPLE_COUNT):
            x = i * 2 / float(self._WAVEFORM_SAMPLE_COUNT) - 1
            for (f, a) in prewarp_chain:
                x = normalise(f(x, a))
            y = base_func(x)
            for (f, a) in postwarp_chain:
                y = f(y, a)
            base[i] = y

        self._set_value('p_ln_base.json', base)
        self._set_value('i_base.json', base_def)

    def get_base_waveform_func(self):
        base_def = self._get_base_waveform_def()
        return base_def['base_func']

    def set_base_waveform_func(self, name):
        assert name in self._WAVEFORM_FUNCS_DICT
        base_def = self._get_base_waveform_def()
        base_def['base_func'] = name
        self._update_base_waveform(base_def)

    def get_prewarp_func_names(self):
        return [name for (name, _) in self._PREWARP_FUNCS]

    def get_prewarp_func_count(self):
        base_def = self._get_base_waveform_def()
        return len(base_def['prewarps'])

    def get_prewarp_func(self, index):
        base_def = self._get_base_waveform_def()
        return tuple(base_def['prewarps'][index])

    def add_prewarp_func(self):
        base_def = self._get_base_waveform_def()
        base_def['prewarps'].append(['Shift', 0])
        self._update_base_waveform(base_def)

    def set_prewarp_func(self, index, name, arg):
        base_def = self._get_base_waveform_def()
        base_def['prewarps'][index] = [name, arg]
        self._update_base_waveform(base_def)

    def move_prewarp_func(self, from_index, to_index):
        base_def = self._get_base_waveform_def()
        entry = base_def['prewarps'].pop(from_index)
        base_def['prewarps'].insert(to_index, entry)
        self._update_base_waveform(base_def)

    def remove_prewarp_func(self, index):
        base_def = self._get_base_waveform_def()
        del base_def['prewarps'][index]
        self._update_base_waveform(base_def)

    def get_postwarp_func_names(self):
        return [name for (name, _) in self._POSTWARP_FUNCS]

    def get_postwarp_func_count(self):
        base_def = self._get_base_waveform_def()
        return len(base_def['postwarps'])

    def get_postwarp_func(self, index):
        base_def = self._get_base_waveform_def()
        return tuple(base_def['postwarps'][index])

    def add_postwarp_func(self):
        base_def = self._get_base_waveform_def()
        base_def['postwarps'].append(['Shift', 0])
        self._update_base_waveform(base_def)

    def set_postwarp_func(self, index, name, arg):
        base_def = self._get_base_waveform_def()
        base_def['postwarps'][index] = [name, arg]
        self._update_base_waveform(base_def)

    def move_postwarp_func(self, from_index, to_index):
        base_def = self._get_base_waveform_def()
        entry = base_def['postwarps'].pop(from_index)
        base_def['postwarps'].insert(to_index, entry)
        self._update_base_waveform(base_def)

    def remove_postwarp_func(self, index):
        base_def = self._get_base_waveform_def()
        del base_def['postwarps'][index]
        self._update_base_waveform(base_def)

    def get_phase_mod_enabled(self):
        return (self._get_value('p_i_mod.json', 0) == 1)

    def set_phase_mod_enabled(self, enabled):
        self._set_value('p_i_mod.json', 1 if enabled else 0)

    def get_mod_volume(self):
        return self._get_value('p_f_mod_volume.json', 0.0)

    def set_mod_volume(self, volume):
        self._set_value('p_f_mod_volume.json', volume)

    def get_mod_envelope_enabled(self):
        return self._get_value('p_b_mod_env_enabled.json', False)

    def set_mod_envelope_enabled(self, enabled):
        self._set_value('p_b_mod_env_enabled.json', enabled)

    def get_mod_envelope(self):
        ret_env = { 'nodes': [ [0, 1], [1, 1] ], 'smooth': False }
        stored_env = self._get_value('p_e_mod_env.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_mod_envelope(self, envelope):
        self._set_value('p_e_mod_env.json', envelope)

    def get_mod_envelope_scale_amount(self):
        return self._get_value('p_f_mod_env_scale_amount.json', 0)

    def set_mod_envelope_scale_amount(self, value):
        self._set_value('p_f_mod_env_scale_amount.json', value)

    def get_mod_envelope_scale_center(self):
        return self._get_value('p_f_mod_env_scale_center.json', 0)

    def set_mod_envelope_scale_center(self, value):
        self._set_value('p_f_mod_env_scale_center.json', value)


