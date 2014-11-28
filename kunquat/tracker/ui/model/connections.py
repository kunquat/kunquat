# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value


class Connections():

    def __init__(self):
        self._controller = None
        self._store = None
        self._ins_id = None
        self._eff_id = None

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_eff_id(self, eff_id):
        self._eff_id = eff_id

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def _get_complete_key(self, subkey):
        parts = []
        if self._ins_id:
            parts.append(self._ins_id)
        if self._eff_id:
            parts.append(self._eff_id)
        parts.append(subkey)
        return '/'.join(parts)

    def _get_graph_key(self):
        return self._get_complete_key('p_connections.json')

    def _get_layout_key(self):
        return self._get_complete_key('i_connections_layout.json')

    def get_connections(self):
        key = self._get_graph_key()
        return self._store.get(key, get_default_value(key))

    def set_connections(self, conns):
        key = self._get_graph_key()
        self._store[key] = conns

    def get_layout(self):
        key = self._get_layout_key()
        return self._store.get(key, {})

    def set_layout(self, layout):
        key = self._get_layout_key()
        self._store[key] = layout


