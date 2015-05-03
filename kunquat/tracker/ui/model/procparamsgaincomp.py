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


class ProcParamsGainComp(ProcParams):

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_mapping_enabled(self):
        return self._get_value('p_b_map_enabled.json', False)

    def set_mapping_enabled(self, enabled):
        self._set_value('p_b_map_enabled.json', enabled)

    def get_mapping(self):
        ret_env = { 'nodes': [ [0, 0], [1, 1] ], 'smooth': False }
        stored_env = self._get_value('p_e_map.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_mapping(self, envelope):
        self._set_value('p_e_map.json', envelope)


