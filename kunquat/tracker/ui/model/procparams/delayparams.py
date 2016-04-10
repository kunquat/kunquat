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

import math

from .procparams import ProcParams


class DelayParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio L',
            'in_01':  'audio R',
            'in_02':  'delay',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_max_delay(self):
        return self._get_value('p_f_max_delay.json', 2.0)

    def set_max_delay(self, value):
        self._set_value('p_f_max_delay.json', value)

    def get_init_delay(self):
        return self._get_value('p_f_init_delay.json', 0.0)

    def set_init_delay(self, value):
        self._set_value('p_f_init_delay.json', value)


