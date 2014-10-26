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

    def get_global_force(self):
        key = self._get_key('p_instrument.json')
        try:
            global_force = self._store[key]['global_force']
        except KeyError:
            global_force = get_default_value(key)['global_force']
        return global_force

    def set_global_force(self, global_force):
        key = self._get_key('p_instrument.json')
        d = self._store.get(key, get_default_value(key))
        d['global_force'] = global_force
        self._store[key] = d

    def get_force_envelope(self):
        key = self._get_key('p_envelope_force.json')
        try:
            envelope = get_default_value(key)
            envelope.update(self._store[key])
        except KeyError:
            pass
        return envelope

    def set_force_envelope(self, envelope):
        key = self._get_key('p_envelope_force.json')
        self._store[key] = envelope


