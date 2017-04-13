# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import takewhile

from .procparams import ProcParams


class GainCompParams(ProcParams):

    @staticmethod
    def get_default_signal_type():
        return 'mixed'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'audio L',
            'in_01':  'audio R',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    _NODES_MAX = 256

    @staticmethod
    def get_max_node_count():
        return GainCompParams._NODES_MAX

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

    def get_mapping_enabled(self):
        return self._get_value('p_b_map_enabled.json', False)

    def set_mapping_enabled(self, enabled):
        self._set_value('p_b_map_enabled.json', enabled)

    def is_mapping_symmetric(self):
        env = self.get_mapping()
        return (env['nodes'][0][0] == 0)

    def set_mapping_symmetric(self, make_symmetric):
        if self.is_mapping_symmetric() == make_symmetric:
            return

        orig_env = self.get_mapping()
        orig_nodes = orig_env['nodes']

        if make_symmetric:
            new_nodes = []

            first_nonneg_index = len(list(takewhile(lambda n: n[0] < 0, orig_nodes)))
            nonneg_nodes = orig_nodes[first_nonneg_index:]
            if nonneg_nodes[0][0] != 0:
                last_neg_x, last_neg_y = orig_nodes[first_nonneg_index - 1]
                first_pos_x, first_pos_y = nonneg_nodes[0]
                lerp_t = -last_neg_x / (-last_neg_x + first_pos_x)
                new_nodes = [
                        [0, max(0, last_neg_y + (first_pos_y - last_neg_y) * lerp_t)]]

            new_nodes += [[x, max(0, y)] for x, y in nonneg_nodes]

        else:
            if len(orig_nodes) * 2 <= self._NODES_MAX:
                centre_nodes = []
                if tuple(orig_nodes[0]) != (0, 0):
                    _, first_y = orig_nodes[0]
                    second_x, _ = orig_nodes[1]
                    new_first_x = min(0.0001, second_x * 0.5)
                    centre_nodes = [[-new_first_x, -first_y], [new_first_x, first_y]]
                else:
                    centre_nodes = [[0, 0]]

                pos_nodes = orig_nodes[1:]
                neg_nodes = [[-x, -y] for x, y in reversed(pos_nodes)]
                new_nodes = neg_nodes + centre_nodes + pos_nodes
            else:
                new_nodes = [[-1, -1]] + orig_nodes[1:]

        new_env = { 'nodes': new_nodes, 'smooth': orig_env['smooth'] }
        self.set_mapping(new_env)

    def get_mapping(self):
        ret_env = { 'nodes': [ [-1, -1], [1, 1] ], 'smooth': False }
        stored_env = self._get_value('p_e_map.json', None) or {}
        ret_env.update(stored_env)
        return ret_env

    def set_mapping(self, envelope):
        nodes = [(x, y) for x, y in envelope['nodes']]
        assert len(nodes) <= self._NODES_MAX
        assert nodes[-1][0] == 1
        assert all(x1 < x2 for x1, x2 in zip(nodes, nodes[1:]))
        if nodes[0][0] == -1:
            assert all(-1 <= y <= 1 for _, y in nodes)
        elif nodes[0][0] == 0:
            assert all(0 <= y <= 1 for _, y in nodes)
        else:
            assert False

        self._set_value('p_e_map.json', envelope)


