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

from kunquat.kunquat.kunquat import get_default_value
from procparamsadd import ProcParamsAdd
from procparamsenvgen import ProcParamsEnvgen


class Processor():

    def __init__(self, au_id, proc_id):
        assert au_id
        assert proc_id
        self._au_id = au_id
        self._proc_id = proc_id
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def _get_key(self, subkey):
        return '{}/{}/{}'.format(self._au_id, self._proc_id, subkey)

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)

    def _get_ports(self, port_format):
        ports = []
        for i in xrange(0x100):
            port_id = port_format.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                ports.append(port_id)

        return ports

    def get_in_ports(self):
        return self._get_ports('in_{:02x}')

    def get_out_ports(self):
        return self._get_ports('out_{:02x}')

    def get_name(self):
        key = self._get_key('m_name.json')
        return self._store.get(key)

    def set_name(self, name):
        key = self._get_key('m_name.json')
        self._store[key] = name

    def get_type(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key)
        if manifest:
            return manifest['type']
        return None

    def get_type_params(self):
        types = {
            'add':      ProcParamsAdd,
            'envgen':   ProcParamsEnvgen,
        }
        cons = types[self.get_type()]
        return cons(self._au_id, self._proc_id, self._controller)

    def get_voice_signals_enabled(self):
        key = self._get_key('p_voice_support.json')
        return self._store.get(key)

    def set_voice_signals_enabled(self, enabled):
        key = self._get_key('p_voice_support.json')
        self._store[key] = enabled

    def get_mixed_signals_enabled(self):
        key = self._get_key('p_signal_support.json')
        return self._store.get(key)

    def set_mixed_signals_enabled(self, enabled):
        key = self._get_key('p_signal_support.json')
        self._store[key] = enabled


