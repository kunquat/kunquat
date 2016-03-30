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


class TuningTable():

    def __init__(self, table_id):
        self._controller = None
        self._store = None

        self._table_id = table_id

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()

    def _get_key(self, subkey):
        return '{}/{}'.format(self._table_id, subkey)

    def get_name(self):
        key = self._get_key('m_name.json')
        return self._store.get(key, None)

    def set_name(self, name):
        key = self._get_key('m_name.json')
        self._store[key] = name

    def remove(self):
        key_prefix = '{}/'.format(self._table_id)
        transaction = {}
        for key in self._store:
            if key.startswith(key_prefix):
                transaction[key] = None
        self._store.put(transaction)


