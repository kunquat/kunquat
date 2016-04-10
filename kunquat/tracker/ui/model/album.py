# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2016
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
from kunquat.kunquat.limits import *

from .patterninstance import PatternInstance
from .song import Song


class Album():

    def __init__(self):
        self._store = None
        self._controller = None
        self._session = None
        self._ui_model = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller
        self._session = controller.get_session()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_existence(self):
        key = 'album/p_manifest.json'
        try:
            manifest = self._store[key]
        except KeyError:
            return False
        return (type(manifest) == type({}))

    def get_track_count(self):
        if not self.get_existence():
            return 0
        return len(self._get_track_list())

    def get_song_by_track(self, track_num):
        tracks = self._get_track_list()
        song_num = tracks[track_num]
        song_id = 'song_{:02x}'.format(song_num)
        song = Song(song_id, track_num)
        song.set_controller(self._controller)
        song.set_ui_model(self._ui_model)
        return song

    def set_selected_track_num(self, track_num):
        self._session.set_selected_track_num(track_num)

    def get_selected_track_num(self):
        return min(self._session.get_selected_track_num(), self.get_track_count() - 1)

    def get_pattern_instance_location(self, pattern_instance):
        for track_num in range(self.get_track_count()):
            song = self.get_song_by_track(track_num)
            for system_num in range(song.get_system_count()):
                cur_pinst = song.get_pattern_instance(system_num)
                if cur_pinst == pattern_instance:
                    return (track_num, system_num)
        return None

    def get_pattern_instance_location_by_nums(self, pat_num, inst_num):
        # TODO: This function exists to resolve cyclic dependency;
        #       revisit interfaces
        pinst = PatternInstance(pat_num, inst_num)
        return self.get_pattern_instance_location(pinst)

    def get_new_song_num(self):
        free_nums = set(range(SONGS_MAX))

        used_songs = self._get_used_songs()
        free_nums -= set(s.get_number() for s in used_songs)

        if not free_nums:
            return None

        # Return the song ID with smallest number
        free_list = sorted(list(free_nums))
        free_num = free_list[0]
        return free_num

    def get_new_pattern_num(self):
        free_nums = set(range(PATTERNS_MAX))

        used_pinsts = self._get_used_pattern_instances()
        free_nums -= set(p.get_pattern_num() for p in used_pinsts)

        if not free_nums:
            return None

        # Return the ID with smallest pattern number
        free_list = sorted(list(free_nums))
        free_num = free_list[0]
        return free_num

    def get_new_pattern_instance_num(self, pattern_num):
        free_nums = set(range(PAT_INSTANCES_MAX))

        used_pinsts = self._get_used_pattern_instances()
        free_nums -= set(p.get_instance_num()
                for p in used_pinsts if (p.get_pattern_num() == pattern_num))

        if not free_nums:
            return None

        # Return the ID with smallest instance number
        free_list = sorted(list(free_nums))
        free_num = free_list[0]
        return free_num

    def insert_pattern_instance(self, track_num, system_num, pattern_num, instance_num):
        song = self.get_song_by_track(track_num)

        pattern_instance = PatternInstance(pattern_num, instance_num)
        pattern_instance.set_controller(self._controller)
        pattern_instance.set_ui_model(self._ui_model)
        create_pinst = pattern_instance.get_edit_create_pattern_instance()
        create_pat = pattern_instance.get_pattern().get_edit_create_pattern()
        insert_pinst = song.get_edit_insert_pattern_instance(
                system_num, pattern_instance)

        transaction = {}
        transaction.update(create_pinst)
        transaction.update(create_pat)
        transaction.update(insert_pinst)
        self._store.put(transaction)

    def insert_song(self, track_num):
        song_num = self.get_new_song_num()
        pattern_num = self.get_new_pattern_num()
        if (song_num == None) or (pattern_num == None):
            return

        song_id = 'song_{:02x}'.format(song_num)
        song = Song(song_id, track_num)
        song.set_controller(self._controller)
        song.set_ui_model(self._ui_model)
        pattern_instance = PatternInstance(pattern_num, 0)
        pattern_instance.set_controller(self._controller)
        pattern_instance.set_ui_model(self._ui_model)

        track_list = self._get_track_list()
        track_list.insert(track_num, song_num)

        transaction = { 'album/p_tracks.json': track_list }
        if not self.get_existence():
            transaction.update({ 'album/p_manifest.json': {} })
        transaction.update(pattern_instance.get_edit_create_pattern_instance())
        transaction.update(pattern_instance.get_pattern().get_edit_create_pattern())
        transaction.update(song.get_edit_create_song())
        transaction.update(song.get_edit_insert_pattern_instance(0, pattern_instance))
        self._store.put(transaction)

    def remove_pattern_instance(self, track_num, system_num):
        is_last_track = len(self._get_track_list()) == 1

        song = self.get_song_by_track(track_num)
        is_last_system = (song.get_system_count() == 1)

        pattern_instance = song.get_pattern_instance(system_num)
        pattern = pattern_instance.get_pattern()
        instance_ids = pattern.get_instance_ids()
        is_last_instance = (len(instance_ids - set([pattern_instance.get_id()])) == 0)

        transaction = {}
        transaction.update(song.get_edit_remove_pattern_instance(system_num))
        transaction.update(pattern_instance.get_edit_remove_pattern_instance())
        if is_last_instance:
            transaction.update(pattern.get_edit_remove_pattern())
        if is_last_system:
            transaction.update(song.get_edit_remove_song())
            track_list = self._get_track_list()
            del track_list[track_num]
            transaction.update({ 'album/p_tracks.json': track_list })

            if is_last_track:
                transaction.update({ 'album/p_manifest.json': None })

        if is_last_instance:
            # Remove sheet history associated with the removed pattern
            sheet_history = self._ui_model.get_sheet_history()
            sheet_history.remove_pattern_changes(pattern)

        self._store.put(transaction)

    def remove_song(self, track_num):
        song = self.get_song_by_track(track_num)
        while song.get_existence():
            self.remove_pattern_instance(track_num, 0)

    def move_pattern_instance(
            self, from_track_num, from_system_num, to_track_num, to_system_num):
        from_song = self.get_song_by_track(from_track_num)
        to_song = self.get_song_by_track(to_track_num)

        remove_source_song = ((from_song.get_system_count() == 1) and
                (from_track_num != to_track_num))

        transaction = {}
        if from_track_num == to_track_num:
            transaction.update(from_song.get_edit_move_pattern_instance(
                from_system_num, to_system_num))
        else:
            pattern_instance = from_song.get_pattern_instance(from_system_num)
            transaction.update(from_song.get_edit_remove_pattern_instance(
                from_system_num))
            transaction.update(to_song.get_edit_insert_pattern_instance(
                to_system_num, pattern_instance))
            if remove_source_song:
                transaction.update(from_song.get_edit_remove_song())
                track_list = self._get_track_list()
                del track_list[from_track_num]
                transaction.update({ 'album/p_tracks.json': track_list })
        self._store.put(transaction)

    def move_song(self, from_track_num, to_track_num):
        track_list = self._get_track_list()
        song_num = track_list[from_track_num]
        del track_list[from_track_num]
        track_list.insert(to_track_num, song_num)
        self._store['album/p_tracks.json'] = track_list

    def _get_used_songs(self):
        songs = []
        for track_num in range(self.get_track_count()):
            song = self.get_song_by_track(track_num)
            songs.append(song)
        return songs

    def _get_used_pattern_instances(self):
        pattern_instances = []
        for track_num in range(self.get_track_count()):
            song = self.get_song_by_track(track_num)
            for system_num in range(song.get_system_count()):
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


