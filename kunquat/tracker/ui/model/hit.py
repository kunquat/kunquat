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

from kunquat.kunquat.kunquat import get_default_value


class Hit():

    def __init__(self, au_id, index):
        self._au_id = au_id
        self._index = index
        self._controller = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()

    def _get_key(self, subkey):
        return '{}/hit_{:02x}/{}'.format(self._au_id, self._index, subkey)

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)

    def set_existence(self, existence):
        key = self._get_key('p_manifest.json')
        if existence:
            self._store[key] = {}
        else:
            del self._store[key]

    def get_excluded_processors(self):
        key = self._get_key('p_hit_proc_filter.json')
        return self._store.get(key, [])

    def set_excluded_processors(self, procs):
        key = self._get_key('p_hit_proc_filter.json')
        self._store[key] = procs

    def get_name(self):
        key = self._get_key('m_name.json')
        return self._store.get(key, None)

    def set_name(self, name):
        assert isinstance(name, str)
        key = self._get_key('m_name.json')
        self._store[key] = name


