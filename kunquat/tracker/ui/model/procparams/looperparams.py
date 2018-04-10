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


class LooperParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio L',
            'in_01':  'audio R',
            'in_02':  'speed',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_max_rec_time(self):
        return self._get_value('p_f_max_rec_time.json', 16.0)

    def set_max_rec_time(self, value):
        self._set_value('p_f_max_rec_time.json', value)

    def get_state_xfade_time(self):
        return self._get_value('p_f_state_xfade_time.json', 0.005)

    def set_state_xfade_time(self, value):
        self._set_value('p_f_state_xfade_time.json', value)

    def get_play_xfade_time(self):
        return self._get_value('p_f_play_xfade_time.json', 0.005)

    def set_play_xfade_time(self, value):
        self._set_value('p_f_play_xfade_time.json', value)


