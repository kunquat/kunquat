# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .procparams import ProcParams


class MultParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'sig1 L',
            'in_01':  'sig1 R',
            'in_02':  'sig2 L',
            'in_03':  'sig2 R',
            'out_00': 'sig L',
            'out_01': 'sig R',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)


