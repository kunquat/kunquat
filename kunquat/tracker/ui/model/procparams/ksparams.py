# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
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
        return self._get_value('p_f_damp.json', 0.0)

    def set_damp(self, value):
        self._set_value('p_f_damp.json', value)


