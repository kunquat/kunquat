# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
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
from procparamschorus import ProcParamsChorus
from procparamsdelay import ProcParamsDelay
from procparamsenvgen import ProcParamsEnvgen
from procparamsfilter import ProcParamsFilter
from procparamsfreeverb import ProcParamsFreeverb
from procparamsgaincomp import ProcParamsGainComp
from procparamsvolume import ProcParamsVolume


class Processor():

    def __init__(self, au_id, proc_id):
        assert au_id
        assert proc_id
        assert proc_id.startswith(au_id)
        assert len(proc_id.split('/')) == len(au_id.split('/')) + 1
        self._au_id = au_id
        self._proc_id = proc_id
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def _get_key(self, subkey):
        return '{}/{}'.format(self._proc_id, subkey)

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
            'chorus':   ProcParamsChorus,
            'delay':    ProcParamsDelay,
            'envgen':   ProcParamsEnvgen,
            'filter':   ProcParamsFilter,
            'freeverb': ProcParamsFreeverb,
            'gaincomp': ProcParamsGainComp,
            'volume':   ProcParamsVolume,
        }
        cons = types[self.get_type()]
        return cons(self._proc_id, self._controller)

    def get_signal_type(self):
        key = self._get_key('p_signal_type.json')
        return self._store.get(key, get_default_value(key))

    def set_signal_type(self, signal_type):
        key = self._get_key('p_signal_type.json')
        self._store[key] = signal_type

    def _get_vf_key(self, port, vf):
        return 'out_{:02x}/p_vf_{}.json'.format(port, vf)

    def get_vf_pitch(self, port):
        key = self._get_key(self._get_vf_key(port, 'pitch'))
        return self._store.get(key, get_default_value(key))

    def set_vf_pitch(self, port, enabled):
        key = self._get_key(self._get_vf_key(port, 'pitch'))
        self._store[key] = enabled

    def get_vf_force(self, port):
        key = self._get_key(self._get_vf_key(port, 'force'))
        return self._store.get(key, get_default_value(key))

    def set_vf_force(self, port, enabled):
        key = self._get_key(self._get_vf_key(port, 'force'))
        self._store[key] = enabled

    def get_vf_filter(self, port):
        key = self._get_key(self._get_vf_key(port, 'filter'))
        return self._store.get(key, get_default_value(key))

    def set_vf_filter(self, port, enabled):
        key = self._get_key(self._get_vf_key(port, 'filter'))
        self._store[key] = enabled

    def get_vf_panning(self, port):
        key = self._get_key(self._get_vf_key(port, 'panning'))
        return self._store.get(key, get_default_value(key))

    def set_vf_panning(self, port, enabled):
        key = self._get_key(self._get_vf_key(port, 'panning'))
        self._store[key] = enabled


