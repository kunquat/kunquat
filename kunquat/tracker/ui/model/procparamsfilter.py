# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from procparams import ProcParams


class ProcParamsFilter(ProcParams):

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_cutoff(self):
        return self._get_value('p_f_cutoff.json', 100.0)

    def set_cutoff(self, value):
        self._set_value('p_f_cutoff.json', value)

    def get_resonance(self):
        return self._get_value('p_f_resonance.json', 0.0)

    def set_resonance(self, value):
        self._set_value('p_f_resonance.json', value)


