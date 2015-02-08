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

from generatorparamsadd import GeneratorParamsAdd
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


