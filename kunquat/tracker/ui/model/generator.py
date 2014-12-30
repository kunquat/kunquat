# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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


def wave_sine(x):
    return math.sin(x * math.pi)

def wave_triangle(x):
    if x < -0.5:
        return -x * 2 - 2
    elif x > 0.5:
        return -x * 2 + 2
    return x * 2

def wave_pulse(x):
    if x < 0:
        return -1
    return 1

def wave_saw(x):
    return x


class GeneratorParamsAdd(GeneratorParams):

    _WAVEFORM_SAMPLE_COUNT = 4096
    _WAVEFORM_FUNCS = [
            ('Sine', wave_sine),
            ('Triangle', wave_triangle),
            ('Pulse', wave_pulse),
            ('Sawtooth', wave_saw),
        ]
    _WAVEFORM_FUNCS_DICT = dict(_WAVEFORM_FUNCS)

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

    def _get_base_waveform_def(self):
        base_def = self._get_value('i_base.json', None)
        if type(base_def) != dict:
            base_def = {}

        # Replace invalid entries with defaults
        if base_def.get('base_func', None) not in self._WAVEFORM_FUNCS_DICT:
            if not self._get_value('p_ln_base.json', None):
                base_def['base_func'] = 'Sine'
            else:
                base_def['base_func'] = None

        return base_def

    def _update_base_waveform(self, base_def):
        base = [0] * self._WAVEFORM_SAMPLE_COUNT
        base_func = self._WAVEFORM_FUNCS_DICT[base_def['base_func']]

        for i in xrange(self._WAVEFORM_SAMPLE_COUNT):
            x = i * 2 / float(self._WAVEFORM_SAMPLE_COUNT) - 1
            base[i] = base_func(x)

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


