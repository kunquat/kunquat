# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2020
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .procparams import ProcParams


class RangeMapParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'in 1',
            'in_01':  'in 2',
            'out_00': 'out 1',
            'out_01': 'out 2',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_from_min(self):
        return self._get_value('p_f_from_min.json', 0.0)

    def set_from_min(self, value):
        self._set_value('p_f_from_min.json', value)

    def get_from_max(self):
        return self._get_value('p_f_from_max.json', 1.0)

    def set_from_max(self, value):
        self._set_value('p_f_from_max.json', value)

    def get_min_to(self):
        return self._get_value('p_f_min_to.json', 0.0)

    def set_min_to(self, value):
        self._set_value('p_f_min_to.json', value)

    def get_max_to(self):
        return self._get_value('p_f_max_to.json', 1.0)

    def set_max_to(self, value):
        self._set_value('p_f_max_to.json', value)

    def get_clamp_dest_min(self):
        return self._get_value('p_b_clamp_dest_min.json', True)

    def set_clamp_dest_min(self, enabled):
        self._set_value('p_b_clamp_dest_min.json', enabled)

    def get_clamp_dest_max(self):
        return self._get_value('p_b_clamp_dest_max.json', True)

    def set_clamp_dest_max(self, enabled):
        self._set_value('p_b_clamp_dest_max.json', enabled)

    def get_envelope_enabled(self):
        return self._get_value('p_b_env_enabled.json', False)

    def set_envelope_enabled(self, enabled):
        self._set_value('p_b_env_enabled.json', enabled)

    def get_envelope(self):
        ret_env = { 'nodes': [ [0, 0], [1, 1] ], 'smooth': False }
        stored_env = self._get_value('p_e_envelope.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_envelope(self, envelope):
        self._set_value('p_e_envelope.json', envelope)


