# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .procparams import ProcParams


class SlopeParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'input',
            'out_00': 'slope',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_absolute(self):
        return self._get_value('p_b_absolute.json', False)

    def set_absolute(self, enabled):
        self._set_value('p_b_absolute.json', enabled)

    def get_smoothing(self):
        return self._get_value('p_f_smoothing.json', 0.0)

    def set_smoothing(self, value):
        self._set_value('p_f_smoothing.json', value)


