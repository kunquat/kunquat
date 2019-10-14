# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .procparams import ProcParams


class PhaserParams(ProcParams):

    _STAGES_MIN = 1
    _STAGES_MAX = 32
    _STAGES_DEFAULT = 2

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio L',
            'in_01':  'audio R',
            'in_02':  'cutoff',
            'in_03':  'notch sep',
            'in_04':  'dry/wet',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    @staticmethod
    def get_min_stage_count():
        return PhaserParams._STAGES_MIN

    @staticmethod
    def get_max_stage_count():
        return PhaserParams._STAGES_MAX

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_stage_count(self):
        return self._get_value('p_i_stages.json', self._STAGES_DEFAULT)

    def set_stage_count(self, count):
        self._set_value('p_i_stages.json', count)

    def get_cutoff(self):
        return self._get_value('p_f_cutoff.json', 100.0)

    def set_cutoff(self, value):
        self._set_value('p_f_cutoff.json', value)

    def get_notch_separation(self):
        return self._get_value('p_f_notch_separation.json', 2)

    def set_notch_separation(self, value):
        self._set_value('p_f_notch_separation.json', value)

    def get_dry_wet_ratio(self):
        return self._get_value('p_f_dry_wet_ratio.json', 0.5)

    def set_dry_wet_ratio(self, ratio):
        self._set_value('p_f_dry_wet_ratio.json', ratio)


