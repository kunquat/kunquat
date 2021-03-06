# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .procparams import ProcParams


class BitcrusherParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio L',
            'in_01':  'audio R',
            'in_02':  'cutoff',
            'in_03':  'resol',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_cutoff(self):
        return self._get_value('p_f_cutoff.json', 100.0)

    def set_cutoff(self, value):
        self._set_value('p_f_cutoff.json', value)

    def get_resolution(self):
        return self._get_value('p_f_resolution.json', 16.0)

    def set_resolution(self, value):
        self._set_value('p_f_resolution.json', value)

    def get_res_ignore_min(self):
        return self._get_value('p_f_res_ignore_min.json', 16.0)

    def set_res_ignore_min(self, value):
        self._set_value('p_f_res_ignore_min.json', value)


