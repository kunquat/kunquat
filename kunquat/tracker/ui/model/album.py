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

class Album():

    def __init__(self):
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_existence(self):
        key = 'album/p_manifest.json'
        manifest = self._store[key]
        return (type(manifest) == type({}))

    def get_track_list(self):
        key = 'album/p_tracks.json'
        track_list = self._store[key]
        return track_list


