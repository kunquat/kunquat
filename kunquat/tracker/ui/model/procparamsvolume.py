# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from procparams import ProcParams


class ProcParamsVolume(ProcParams):

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_volume(self):
        return self._get_value('p_f_volume.json', 0.0)

    def set_volume(self, value):
        self._set_value('p_f_volume.json', value)


