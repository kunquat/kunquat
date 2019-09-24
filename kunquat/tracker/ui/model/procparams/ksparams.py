# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2019
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

    _DEFAULT_AUDIO_RATE_RANGE_MIN = 48000
    _DEFAULT_AUDIO_RATE_RANGE_MAX = 48000

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
        return self._get_value('p_f_damp.json', 100.0)

    def set_damp(self, value):
        self._set_value('p_f_damp.json', value)

    def get_audio_rate_range_enabled(self):
        return self._get_value('p_b_audio_rate_range_enabled.json', False)

    def set_audio_rate_range_enabled(self, enabled):
        self._set_value('p_b_audio_rate_range_enabled.json', enabled)

    def get_audio_rate_range_min(self):
        return self._get_value(
                'p_i_audio_rate_range_min.json', self._DEFAULT_AUDIO_RATE_RANGE_MIN)

    def set_audio_rate_range_min(self, rate):
        irate = int(rate)

        cur_max_rate = self.get_audio_rate_range_max()
        if cur_max_rate < irate:
            self._set_value('p_i_audio_rate_range_max.json', irate)

        self._set_value('p_i_audio_rate_range_min.json', irate)

    def get_audio_rate_range_max(self):
        return self._get_value(
                'p_i_audio_rate_range_max.json', self._DEFAULT_AUDIO_RATE_RANGE_MAX)

    def set_audio_rate_range_max(self, rate):
        irate = int(rate)

        cur_min_rate = self.get_audio_rate_range_min()
        if cur_min_rate > irate:
            self._set_value('p_i_audio_rate_range_min.json', irate)

        self._set_value('p_i_audio_rate_range_max.json', irate)


