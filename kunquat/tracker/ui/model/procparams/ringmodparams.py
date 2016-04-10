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
        return u'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  u'audio1 L',
            'in_01':  u'audio1 R',
            'in_02':  u'audio2 L',
            'in_03':  u'audio2 R',
            'out_00': u'audio L',
            'out_01': u'audio R',
        }

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)


