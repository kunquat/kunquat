# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .procparams import ProcParams


class EnvgenParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return { 'in_00': 'stretch', 'in_01': 'force', 'out_00': 'env' }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_time_env_enabled(self):
        return self._get_value('p_b_env_enabled.json', False)

    def set_time_env_enabled(self, enabled):
        self._set_value('p_b_env_enabled.json', enabled)

    def get_time_env(self):
        ret_env = { 'nodes': [ [0, 1], [1, 1] ], 'marks': [0, 1], 'smooth': False }
        stored_env = self._get_value('p_e_env.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_time_env(self, envelope):
        self._set_value('p_e_env.json', envelope)

    def get_time_env_loop_enabled(self):
        return self._get_value('p_b_env_loop_enabled.json', False)

    def set_time_env_loop_enabled(self, enabled):
        self._set_value('p_b_env_loop_enabled.json', enabled)

    def get_time_env_is_release(self):
        return self._get_value('p_b_env_is_release.json', False)

    def set_time_env_is_release(self, value):
        self._set_value('p_b_env_is_release.json', value)

    def get_linear_force_enabled(self):
        return self._get_value('p_b_linear_force.json', False)

    def set_linear_force_enabled(self, enabled):
        self._set_value('p_b_linear_force.json', enabled)

    def get_global_adjust(self):
        return self._get_value('p_f_global_adjust.json', 0)

    def set_global_adjust(self, value):
        self._set_value('p_f_global_adjust.json', value)

    def get_force_env_enabled(self):
        return self._get_value('p_b_force_env_enabled.json', False)

    def set_force_env_enabled(self, enabled):
        self._set_value('p_b_force_env_enabled.json', enabled)

    def get_force_env(self):
        ret_env = { 'nodes': [ [0, 0], [1, 1] ], 'smooth': False }
        stored_env = self._get_value('p_e_force_env.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_force_env(self, envelope):
        self._set_value('p_e_force_env.json', envelope)

    def get_y_range(self):
        return self._get_value('p_ln_y_range.json', [0, 1])

    def set_y_range(self, y_range):
        assert len(y_range) == 2
        assert y_range[0] <= y_range[1]
        self._set_value('p_ln_y_range.json', y_range)


