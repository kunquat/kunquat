# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value
from connections import Connections
from generator import Generator
from effect import Effect


class Instrument():

    def __init__(self, instrument_id):
        assert(instrument_id)
        self._instrument_id = instrument_id
        self._store = None
        self._controller = None
        self._instrument_number = None
        self._existence = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_id(self):
        return self._instrument_id

    def _get_key(self, subkey):
        return '{}/{}'.format(self._instrument_id, subkey)

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store[key]
        if type(manifest) == type({}):
            return True
        return False

    def get_out_ports(self):
        out_ports = []
        for i in xrange(0x100):
            port_id = 'out_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                out_ports.append(port_id)

        return out_ports

    def get_connections(self):
        connections = Connections()
        connections.set_ins_id(self._instrument_id)
        connections.set_controller(self._controller)
        return connections

    def get_generator(self, gen_id):
        generator = Generator(self._instrument_id, gen_id)
        generator.set_controller(self._controller)
        return generator

    def get_generator_ids(self):
        gen_ids = set()
        for key in self._store.keys():
            start = '{}/gen_'.format(self._instrument_id)
            if key.startswith(start):
                gen_id = key.split('/')[1]
                gen_ids.add(gen_id)
        return gen_ids

    def get_effect(self, eff_id):
        effect = Effect(eff_id)
        effect.set_ins_id(self._instrument_id)
        effect.set_controller(self._controller)
        return effect

    def get_effect_ids(self):
        effect_ids = set()
        for key in self._store.keys():
            start = '{}/eff_'.format(self._instrument_id)
            if key.startswith(start):
                effect_id = key.split('/')[1]
                effect_ids.add(effect_id)
        return effect_ids

    def get_name(self):
        key = self._get_key('m_name.json')
        try:
            name = self._store[key]
        except KeyError:
            return None
        return name

    def set_name(self, name):
        key = self._get_key('m_name.json')
        self._store[key] = name

    def _get_value_from_ins_dict(self, entry_key):
        key = self._get_key('p_instrument.json')
        try:
            value = self._store[key][entry_key]
        except KeyError:
            value = get_default_value(key)[entry_key]
        return value

    def _set_value_of_ins_dict(self, entry_key, value):
        key = self._get_key('p_instrument.json')
        d = self._store.get(key, get_default_value(key))
        d[entry_key] = value
        self._store[key] = d

    def get_global_force(self):
        return self._get_value_from_ins_dict('global_force')

    def set_global_force(self, global_force):
        self._set_value_of_ins_dict('global_force', global_force)

    def get_force_variation(self):
        return self._get_value_from_ins_dict('force_variation')

    def set_force_variation(self, force_var):
        self._set_value_of_ins_dict('force_variation', force_var)

    def get_default_force(self):
        return self._get_value_from_ins_dict('force')

    def set_default_force(self, default_force):
        self._set_value_of_ins_dict('force', default_force)

    def get_force_envelope(self):
        key = self._get_key('p_envelope_force.json')
        try:
            envelope = get_default_value(key)
            default_markers = envelope['envelope']['marks']
            envelope.update(self._store[key])
            if 'marks' not in envelope['envelope']:
                envelope['envelope']['marks'] = default_markers
        except KeyError:
            pass
        return envelope

    def set_force_envelope(self, envelope):
        key = self._get_key('p_envelope_force.json')
        self._store[key] = envelope

    def get_force_release_envelope(self):
        key = self._get_key('p_envelope_force_release.json')
        try:
            envelope = get_default_value(key)
            envelope.update(self._store[key])
        except KeyError:
            pass
        return envelope

    def set_force_release_envelope(self, envelope):
        key = self._get_key('p_envelope_force_release.json')
        self._store[key] = envelope


