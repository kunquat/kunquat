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

class Song():

    def __init__(self, song_id):
        assert song_id
        self._song_id = song_id
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_existence(self):
        key = '{}/p_manifest.json'.format(self._song_id)
        manifest = self._store[key]
        return (type(manifest) == type({}))

    def get_order_list(self):
        key = '{}/p_order_list.json'.format(self._song_id)
        order_list = self._store[key]
        return order_list


