# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .procparams import ProcParams


class KsParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'pitch',
            'in_01':  'force',
            'in_02':  'excit',
            'in_03':  'damp',
            'out_00': 'audio',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_damp(self):
        return self._get_value('p_f_damp.json', 0.0)

    def set_damp(self, value):
        self._set_value('p_f_damp.json', value)

    def get_init_env_loop_enabled(self):
        return self._get_value('p_b_init_env_loop_enabled.json', False)

    def set_init_env_loop_enabled(self, enabled):
        self._set_value('p_b_init_env_loop_enabled.json', enabled)

    def get_init_env_scale_amount(self):
        return self._get_value('p_f_init_env_scale_amount.json', 0.0)

    def set_init_env_scale_amount(self, value):
        self._set_value('p_f_init_env_scale_amount.json', value)

    def get_init_env_scale_centre(self):
        return self._get_value('p_f_init_env_scale_center.json', 0.0)

    def set_init_env_scale_centre(self, value):
        self._set_value('p_f_init_env_scale_center.json', value)

    def get_init_env(self):
        ret_env = { 'nodes': [ [0, 1], [0.01, 0] ], 'marks': [0, 1], 'smooth': False }
        stored_env = self._get_value('p_e_init_env.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_init_env(self, envelope):
        self._set_value('p_e_init_env.json', envelope)

    def get_shift_env_enabled(self):
        return self._get_value('p_b_shift_env_enabled.json', False)

    def set_shift_env_enabled(self, enabled):
        self._set_value('p_b_shift_env_enabled.json', enabled)

    def get_shift_env_scale_amount(self):
        return self._get_value('p_f_shift_env_scale_amount.json', 0.0)

    def set_shift_env_scale_amount(self, value):
        self._set_value('p_f_shift_env_scale_amount.json', value)

    def get_shift_env_scale_centre(self):
        return self._get_value('p_f_shift_env_scale_center.json', 0.0)

    def set_shift_env_scale_centre(self, value):
        self._set_value('p_f_shift_env_scale_center.json', value)

    def get_shift_env(self):
        ret_env = { 'nodes': [ [0, 1], [0.001, 0] ], 'smooth': False }
        stored_env = self._get_value('p_e_shift_env.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_shift_env(self, envelope):
        self._set_value('p_e_shift_env.json', envelope)

    def get_shift_env_trigger_threshold(self):
        return self._get_value('p_f_shift_env_trig_threshold.json', 40)

    def set_shift_env_trigger_threshold(self, value):
        self._set_value('p_f_shift_env_trig_threshold.json', value)

    def get_shift_env_strength_var(self):
        return self._get_value('p_f_shift_env_strength_var.json', 0)

    def set_shift_env_strength_var(self, value):
        self._set_value('p_f_shift_env_strength_var.json', value)

    def get_release_env_enabled(self):
        return self._get_value('p_b_rel_env_enabled.json', False)

    def set_release_env_enabled(self, enabled):
        self._set_value('p_b_rel_env_enabled.json', enabled)

    def get_release_env_scale_amount(self):
        return self._get_value('p_f_rel_env_scale_amount.json', 0.0)

    def set_release_env_scale_amount(self, value):
        self._set_value('p_f_rel_env_scale_amount.json', value)

    def get_release_env_scale_centre(self):
        return self._get_value('p_f_rel_env_scale_center.json', 0.0)

    def set_release_env_scale_centre(self, value):
        self._set_value('p_f_rel_env_scale_center.json', value)

    def get_release_env(self):
        ret_env = { 'nodes': [ [0, 1], [0.01, 0] ], 'smooth': False }
        stored_env = self._get_value('p_e_rel_env.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_release_env(self, envelope):
        self._set_value('p_e_rel_env.json', envelope)

    def get_release_env_strength_var(self):
        return self._get_value('p_f_rel_env_strength_var.json', 0)

    def set_release_env_strength_var(self, value):
        self._set_value('p_f_rel_env_strength_var.json', value)


