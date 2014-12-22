# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value

from song import Song


class Album():

    def __init__(self):
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_existence(self):
        key = 'album/p_manifest.json'
        try:
            manifest = self._store[key]
        except KeyError:
            return False
        return (type(manifest) == type({}))

    def get_track_count(self):
        assert self.get_existence()
        return len(self._get_track_list())

    def get_song_by_track(self, track_num):
        tracks = self._get_track_list()
        song_num = tracks[track_num]
        song_id = 'song_{:02x}'.format(song_num)
        song = Song(song_id, track_num)
        song.set_controller(self._controller)
        return song

    def _get_track_list(self):
        key = 'album/p_tracks.json'
        try:
            track_list = self._store[key]
        except KeyError:
            track_list = get_default_value(key)
        return track_list


