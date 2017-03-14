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


class FilterParams(ProcParams):

    _TYPE_NAME_MAP = { 0: 'lowpass', 1: 'highpass' }
    _NAME_TYPE_MAP = dict((v, k) for (k, v) in _TYPE_NAME_MAP.items())

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio L',
            'in_01':  'audio R',
            'in_02':  'cutoff',
            'in_03':  'reso',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_type(self):
        return self._TYPE_NAME_MAP.get(
                self._get_value('p_i_type.json', 0), 'unsupported')

    def set_type(self, new_type):
        self._set_value('p_i_type.json', self._NAME_TYPE_MAP[new_type])

    def get_cutoff(self):
        return self._get_value('p_f_cutoff.json', 100.0)

    def set_cutoff(self, value):
        self._set_value('p_f_cutoff.json', value)

    def get_resonance(self):
        return self._get_value('p_f_resonance.json', 0.0)

    def set_resonance(self, value):
        self._set_value('p_f_resonance.json', value)


