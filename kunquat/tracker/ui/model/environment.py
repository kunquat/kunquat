# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value
from . import tstamp


class Environment():

    def __init__(self):
        self._controller = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()

    def get_var_names(self):
        var_names = [entry[1] for entry in self._get_env_list()]
        return var_names

    def get_var_types(self):
        return [bool, int, float, tstamp.Tstamp]

    def get_var_type(self, var_name):
        env_dict = self._get_env_dict()
        var_entry = env_dict[var_name]
        return self._get_object_type(var_entry[0])

    def get_var_init_value(self, var_name):
        env_dict = self._get_env_dict()
        var_entry = env_dict[var_name]
        var_type = self._get_object_type(var_entry[0])
        return var_type(var_entry[1])

    def add_var(self, var_name, var_type, init_value):
        assert var_name not in self._get_env_dict()
        type_name = self._get_format_type(var_type)
        env_list = self._get_env_list()
        env_list.append([type_name, var_name, init_value])

        self._store[self._get_key()] = env_list

    def remove_var(self, var_name):
        env_list = self._get_env_list()
        index = self._get_entry_index(env_list, var_name)
        del env_list[index]

        self._store[self._get_key()] = env_list

    def change_var_name(self, var_name, new_name):
        env_list = self._get_env_list()
        index = self._get_entry_index(env_list, var_name)
        env_list[index][1] = new_name

        self._store[self._get_key()] = env_list

    def change_var_type(self, var_name, new_type):
        new_type_name = self._get_format_type(new_type)

        env_list = self._get_env_list()
        index = self._get_entry_index(env_list, var_name)
        env_list[index][0] = new_type_name
        env_list[index][2] = new_type(0)

        self._store[self._get_key()] = env_list

    def change_var_init_value(self, var_name, new_init_value):
        if type(new_init_value) == tstamp.Tstamp:
            new_init_value = list(new_init_value)

        env_list = self._get_env_list()
        index = self._get_entry_index(env_list, var_name)
        env_list[index][2] = new_init_value

        self._store[self._get_key()] = env_list

    def _get_key(self):
        return 'p_environment.json'

    def _get_env_list(self):
        key = self._get_key()
        return self._store.get(key, get_default_value(key))

    def _get_entry_index(self, env_list, var_name):
        for i, entry in enumerate(env_list):
            _, entry_name, _ = entry
            if entry_name == var_name:
                return i
        raise ValueError('Variable name {} not in list'.format(var_name))

    def _get_env_dict(self):
        env_list = self._get_env_list()
        env_dict = {}
        for entry in env_list:
            type_name, var_name, init_value = entry
            env_dict[var_name] = (type_name, init_value)
        return env_dict

    def _get_object_type(self, type_name):
        type_map = {
                'bool': bool,
                'int': int,
                'float': float,
                'timestamp': tstamp.Tstamp,
            }
        return type_map[type_name]

    def _get_format_type(self, obj_type):
        type_map = {
                bool: 'bool',
                int: 'int',
                float: 'float',
                tstamp.Tstamp: 'timestamp',
            }
        return type_map[obj_type]


