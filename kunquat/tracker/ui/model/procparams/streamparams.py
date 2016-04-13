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

from .procparams import ProcParams


class StreamParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return { 'out_00': 'stream' }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_init_value(self):
        return self._get_value('p_f_init_value.json', 0.0)

    def set_init_value(self, value):
        self._set_value('p_f_init_value.json', value)

    def get_init_osc_speed(self):
        return self._get_value('p_f_init_osc_speed.json', 0.0)

    def set_init_osc_speed(self, value):
        self._set_value('p_f_init_osc_speed.json', value)

    def get_init_osc_depth(self):
        return self._get_value('p_f_init_osc_depth.json', 0.0)

    def set_init_osc_depth(self, value):
        self._set_value('p_f_init_osc_depth.json', value)


