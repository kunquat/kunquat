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
        return { 'in_00': 'stretch', 'in_01': 'trigger', 'out_00': 'env' }

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

    def get_trig_immediate(self):
        return self._get_value('p_b_trig_immediate.json', True)

    def set_trig_immediate(self, enabled):
        self._set_value('p_b_trig_immediate.json', enabled)

    def get_trig_release(self):
        return self._get_value('p_b_trig_release.json', False)

    def set_trig_release(self, enabled):
        self._set_value('p_b_trig_release.json', enabled)

    def get_trig_impulse_floor(self):
        return self._get_value('p_b_trig_impulse_floor.json', False)

    def set_trig_impulse_floor(self, enabled):
        self._set_value('p_b_trig_impulse_floor.json', enabled)

    def get_trig_impulse_ceil(self):
        return self._get_value('p_b_trig_impulse_ceil.json', False)

    def set_trig_impulse_ceil(self, enabled):
        self._set_value('p_b_trig_impulse_ceil.json', enabled)

    def get_trig_impulse_floor_bounds(self):
        return self._get_value('p_ln_trig_impulse_floor_bounds.json', [-1.0, -0.5])

    def set_trig_impulse_floor_bounds(self, bounds):
        assert len(bounds) == 2
        assert bounds[0] <= bounds[1]
        self._set_value('p_ln_trig_impulse_floor_bounds.json', bounds)

    def get_trig_impulse_ceil_bounds(self):
        return self._get_value('p_ln_trig_impulse_ceil_bounds.json', [1.0, 0.5])

    def set_trig_impulse_ceil_bounds(self, bounds):
        assert len(bounds) == 2
        assert bounds[0] >= bounds[1]
        self._set_value('p_ln_trig_impulse_ceil_bounds.json', bounds)

    def get_linear_force_enabled(self):
        return self._get_value('p_b_linear_force.json', False)

    def set_linear_force_enabled(self, enabled):
        self._set_value('p_b_linear_force.json', enabled)

    def get_global_adjust(self):
        return self._get_value('p_f_global_adjust.json', 0)

    def set_global_adjust(self, value):
        self._set_value('p_f_global_adjust.json', value)

    def get_y_range(self):
        return self._get_value('p_ln_y_range.json', [0, 1])

    def set_y_range(self, y_range):
        assert len(y_range) == 2
        assert y_range[0] <= y_range[1]
        self._set_value('p_ln_y_range.json', y_range)

    def get_y_range_min_var(self):
        return self._get_value('p_f_y_range_min_var.json', 0.0)

    def set_y_range_min_var(self, value):
        assert value >= 0
        self._set_value('p_f_y_range_min_var.json', value)

    def get_y_range_max_var(self):
        return self._get_value('p_f_y_range_max_var.json', 0.0)

    def set_y_range_max_var(self, value):
        assert value >= 0
        self._set_value('p_f_y_range_max_var.json', value)


