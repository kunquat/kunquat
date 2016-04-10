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


class PitchParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return u'voice'

    @staticmethod
    def get_port_info():
        return { 'out_00': u'pitch' }

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)


