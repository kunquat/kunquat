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

from procparams import ProcParams


class ProcParamsEnvgen(ProcParams):

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_port_names(self):
        return { 'in_00': u'freq', 'in_01': u'scale', 'out_00': u'env' }

    def get_scale(self):
        return self._get_value('p_f_scale.json', 0.0)

    def set_scale(self, value):
        self._set_value('p_f_scale.json', value)

    def get_y_range(self):
        return self._get_value('p_ln_y_range.json', [0, 1])

    def set_y_range(self, y_range):
        assert len(y_range) == 2
        assert y_range[0] <= y_range[1]
        self._set_value('p_ln_y_range.json', y_range)

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

    def get_time_env_scale_amount(self):
        return self._get_value('p_f_env_scale_amount.json', 0)

    def set_time_env_scale_amount(self, value):
        self._set_value('p_f_env_scale_amount.json', value)

    def get_time_env_scale_center(self):
        return self._get_value('p_f_env_scale_center.json', 0)

    def set_time_env_scale_center(self, value):
        self._set_value('p_f_env_scale_center.json', value)

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


