# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class ProcParams():

    @staticmethod
    def get_default_signal_type():
        raise NotImplementedError

    @staticmethod
    def get_port_info():
        return {}

    @staticmethod
    def register_conversion_infos(data_converters):
        pass

    def __init__(self, proc_id, controller):
        self._key_prefix = '{}/'.format(proc_id)
        self._controller = controller
        self._store = controller.get_store()
        self._session = controller.get_session()
        self._updater = controller.get_updater()
        self._proc_id = proc_id

    def _get_key(self, impl_or_conf, subkey):
        assert impl_or_conf in ('i/', 'c/')
        return ''.join((self._key_prefix, impl_or_conf, subkey))

    # Protected interface

    def _get_conf_key(self, subkey):
        return self._get_key('c/', subkey)

    def _get_value(self, subkey, default_value):
        conf_key = self._get_key('c/', subkey)
        if conf_key in self._store:
            value = self._store[conf_key]
            if value != None:
                return value
        impl_key = self._get_key('i/', subkey)
        value = self._store.get(impl_key, default_value)
        if value != None:
            return value
        return default_value

    def _get_edit_set_value(self, subkey, value):
        key = self._get_key('c/', subkey)
        transaction = { key: value }
        return transaction

    def _set_value(self, subkey, value):
        transaction = self._get_edit_set_value(subkey, value)
        self._store.put(transaction)


