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

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_connections(self):
        key = 'p_connections.json'
        return self._store.get(key, get_default_value(key))

    def set_connections(self, conns):
        key = 'p_connections.json'
        self._store[key] = conns

    def get_layout(self):
        key = 'i_connections_layout.json'
        return self._store.get(key, {})

    def set_layout(self, layout):
        key = 'i_connections_layout.json'
        self._store[key] = layout


