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


class CompressParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio L',
            'in_01':  'audio R',
            'out_00': 'audio L',
            'out_01': 'audio R',
            'out_02': 'gain',
        }

    _DEFAULT_ATTACK = 1.0
    _DEFAULT_RELEASE = 100.0

    _DEFAULT_UPWARD_THRESHOLD = -60.0
    _DEFAULT_DOWNWARD_THRESHOLD = 0.0
    _DEFAULT_RATIO = 6.0

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    @staticmethod
    def get_react_range():
        return (1.0, 2000.0)

    def get_attack(self):
        return self._get_value('p_f_attack.json', self._DEFAULT_ATTACK)

    def set_attack(self, value):
        self._set_value('p_f_attack.json', value)

    def get_release(self):
        return self._get_value('p_f_release.json', self._DEFAULT_RELEASE)

    def set_release(self, value):
        self._set_value('p_f_release.json', value)

    @staticmethod
    def get_threshold_range():
        return (CompressParams._DEFAULT_UPWARD_THRESHOLD,
                CompressParams._DEFAULT_DOWNWARD_THRESHOLD)

    @staticmethod
    def get_ratio_range():
        return (1.0, 60.0)

    def get_upward_enabled(self):
        return self._get_value('p_b_upward_enabled.json', False)

    def set_upward_enabled(self, enabled):
        self._set_value('p_b_upward_enabled.json', enabled)

    def get_upward_threshold(self):
        return self._get_value(
                'p_f_upward_threshold.json', self._DEFAULT_UPWARD_THRESHOLD)

    def set_upward_threshold(self, value):
        self._set_value('p_f_upward_threshold.json', value)

    def get_upward_ratio(self):
        return self._get_value('p_f_upward_ratio.json', self._DEFAULT_RATIO)

    def set_upward_ratio(self, value):
        self._set_value('p_f_upward_ratio.json', value)

    def get_downward_enabled(self):
        return self._get_value('p_b_downward_enabled.json', False)

    def set_downward_enabled(self, enabled):
        self._set_value('p_b_downward_enabled.json', enabled)

    def get_downward_threshold(self):
        return self._get_value(
                'p_f_downward_threshold.json', self._DEFAULT_DOWNWARD_THRESHOLD)

    def set_downward_threshold(self, value):
        self._set_value('p_f_downward_threshold.json', value)

    def get_downward_ratio(self):
        return self._get_value('p_f_downward_ratio.json', self._DEFAULT_RATIO)

    def set_downward_ratio(self, value):
        self._set_value('p_f_downward_ratio.json', value)


