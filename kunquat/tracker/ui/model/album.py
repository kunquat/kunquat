# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2015
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

from patterninstance import PatternInstance
from song import Song


PATTERNS_MAX = 1024 # TODO: define in libkunquat interface


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

    def get_pattern_instance_location(self, pattern_instance):
        for track_num in xrange(self.get_track_count()):
            song = self.get_song_by_track(track_num)
            for system_num in xrange(song.get_system_count()):
                cur_pinst = song.get_pattern_instance(system_num)
                if cur_pinst == pattern_instance:
                    return (track_num, system_num)
        return None

    def get_new_pattern_num(self):
        free_nums = set(xrange(PATTERNS_MAX))

        used_pinsts = self._get_used_pattern_instances()
        free_nums -= set(p.get_pattern_num() for p in used_pinsts)

        if not free_nums:
            return None

        # Return the ID with smallest pattern number
        free_list = sorted(list(free_nums))
        free_num = free_list[0]
        return free_num

    def insert_pattern_instance(self, track_num, system_num, pattern_num, instance_num):
        song = self.get_song_by_track(track_num)

        pattern_instance = PatternInstance(pattern_num, instance_num)
        pattern_instance.set_controller(self._controller)
        create_pinst = pattern_instance.get_edit_create_pattern_instance()
        create_pat = pattern_instance.get_pattern().get_edit_create_pattern()
        insert_pinst = song.get_edit_insert_pattern_instance(
                system_num, pattern_instance)

        transaction = {}
        transaction.update(create_pinst)
        transaction.update(create_pat)
        transaction.update(insert_pinst)
        self._store.put(transaction)

    def remove_pattern_instance(self, track_num, system_num):
        song = self.get_song_by_track(track_num)
        assert song.get_system_count() > 1

        pattern_instance = song.get_pattern_instance(system_num)
        pattern = pattern_instance.get_pattern()
        instance_ids = pattern.get_instance_ids()
        is_last_instance = (len(instance_ids - set([pattern_instance.get_id()])) == 0)

        transaction = {}
        transaction.update(song.get_edit_remove_pattern_instance(system_num))
        transaction.update(pattern_instance.get_edit_remove_pattern_instance())
        if is_last_instance:
            transaction.update(pattern.get_edit_remove_pattern())
        print(transaction)
        self._store.put(transaction)

    def _get_used_pattern_instances(self):
        pattern_instances = []
        for track_num in xrange(self.get_track_count()):
            song = self.get_song_by_track(track_num)
            for system_num in xrange(song.get_system_count()):
                pinst = song.get_pattern_instance(system_num)
                pattern_instances.append(pinst)
        return pattern_instances

    def _get_track_list(self):
        key = 'album/p_tracks.json'
        try:
            track_list = self._store[key]
        except KeyError:
            track_list = get_default_value(key)
        return track_list


