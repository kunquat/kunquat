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


class RingmodParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio1 L',
            'in_01':  'audio1 R',
            'in_02':  'audio2 L',
            'in_03':  'audio2 R',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)


