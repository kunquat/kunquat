# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2010-2013
#          Toni Ruottu,       Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from instrument import Instrument
from pattern_instance import Pattern_instance
from song import Song
from copy import deepcopy
from itertools import izip, takewhile
from history import History
import kqt_limits as lim
import re
import tools

class Composition():

    def __init__(self, store, p):
        root = ''
        self._history = History(self)
        self._store = store
        self._view = store
        self.p = p
        self._tracks = []

    def get(self, key):
        suffix = key.split('.').pop()
        is_json = suffix.startswith('json')
        if is_json:
            value = self._view.get_json(key)
        else:
            value = self._view.get(key)
            value = value if value else None
        return value

    def __getitem__(self, key):
        return self.get(key)

    def get_view(self, path):
        return self._view.get_view(path)

    def put(self, key, value, immediate=True, autoconnect=True):
        """Set data in the Kunquat Handle.

        Arguments:
        key   -- The key of the data in the composition.
        value -- The data to be set.

        Optional arguments:
        immediate   -- If True, the data is immediately stored in the
                       project history. Otherwise, the data is delayed
                       until another call of set() or flush().
        autoconnect -- If True, create a simple connection path to
                       master output if the key implies creation of a
                       new instrument or generator.
        """
        assert '.json' not in key or not isinstance(value, str)
        assert immediate in (True, False)
        if value == None:
            autoconnect = False
        if autoconnect:
            autoconnect = self.autoconnect(key, immediate)

        self._history.step(key, deepcopy(value), immediate=immediate)
        self._view.put(key, value)
        #self.set_raw(key, value)

        self._changed = True
        #self._history.show_latest_branch()

    def __setitem__(self, key, value):
        return self.put(key,value)

    def delete(self, key):
        self._view.delete(key)

    def __delitem__(self, key):
        self.delete(key)

    def get_pattern(self, track, system):
        """Get a pattern number based on track and system number."""
        if track < 0 or track >= lim.SONGS_MAX:
            raise IndexError, 'Invalid track number'
        if system < 0 or system >= lim.SECTIONS_MAX:
            raise IndexError, 'Invalid system number'
        song = self._tracks[track]
        orderlist = self['song_{0:02x}/p_order_list.json'.format(song)]
        if not orderlist:
            return None
        if len(orderlist) <= system:
            return None
        return orderlist[system]

    def get_effect(self, base_path, index):
        name = 'eff_{0:02x}'.format(index)
        base = self._view.get_view(base_path)
        effect = base.get_view(name)
        return effect

    def get_pattern_instance(self, pattern_instance_ref):
        return Pattern_instance(self, pattern_instance_ref)

    def get_instrument(self, slot):
        return Instrument(self, slot)

    def subtree(self, prefix):
        """Return a sequence of all the keys inside a project subtree.

        Arguments:
        prefix -- The path of the subtree.

        Return value:
        The sequence of keys.

        """
        view = self._view.get_view(prefix)
        return view.keys()

    def autoconnect(self, key, immediate):
        new_ins = -1
        new_gen = -1
        ins_conn_base = 'ins_{0:02x}/'
        gen_conn_base = 'gen_{0:02x}/C/'
        ins_prefix_base = 'ins_{0:02x}/'
        gen_prefix_base = '{0}gen_{{1:02x}}/'.format(ins_prefix_base)
        ins_pattern = 'ins_([0-9a-f]{2})/'
        gen_pattern = '{0}gen_([0-9a-f]{{2}})/'.format(ins_pattern)
        ins_mo = re.match(ins_pattern, key)
        if not ins_mo:
            return False
        new_ins = int(ins_mo.group(1), 16)
        ins_prefix = ins_prefix_base.format(new_ins)
        gen_mo = re.match(gen_pattern, key)
        if gen_mo:
            new_gen = int(gen_mo.group(2), 16)
            gen_prefix = gen_prefix_base.format(new_ins, new_gen)
            subtree = self.subtree(gen_prefix) #self._store.get_view(gen_prefix).keys()
            if not list(izip((1,), subtree)):
                ins_connections = self[ins_prefix + 'p_connections.json']
                if not ins_connections:
                    ins_connections = []
                gen_conn_prefix = gen_conn_base.format(new_gen)
                for conn in ins_connections:
                    if conn[0].startswith(gen_conn_prefix) or \
                            conn[1].startswith(gen_conn_prefix):
                        new_gen = -1
                        break
                else:
                    ins_connections.extend([[gen_conn_prefix + 'out_00',
                                             'out_00']])
            else:
                new_gen = -1
        connections = self['p_connections.json']
        if not connections:
            connections = []
        ins_conn_prefix = ins_conn_base.format(new_ins)
        subtree = self.subtree(ins_prefix) #self._store.get_view(ins_prefix).keys()
        if not list(izip((1,), subtree)):
            for conn in connections:
                if conn[0].startswith(ins_conn_prefix) or \
                        conn[1].startswith(ins_conn_prefix):
                    new_ins = -1
                    break
            else:
                connections.extend([[ins_conn_prefix + 'out_00', 'out_00']])
        else:
            new_ins = -1
        if new_ins < 0 and new_gen < 0:
            return False
        self._history.start_group('{0} + autoconnect'.format(key))
        if new_ins >= 0:
            self.put('p_connections.json', connections, immediate=immediate,
                     autoconnect=False)
        if new_gen >= 0:
            self.put(ins_prefix + 'p_connections.json', ins_connections,
                     immediate=immediate, autoconnect=False)
        self._history.end_group()
        return True

    def fix_connections(self, instrument):
        if not instrument.startswith('ins_'):
            return
        connections = self['p_connections.json']
        if not connections:
            connections = []
        ins_out = instrument + '/out_00'
        for connection in connections:
            if ins_out in connection:
                break
        else:
            connections.append([ins_out, 'out_00'])
            self._view.put('p_connections.json', connections)

    def to_tar(self, path):
        prefix = 'kqtc{0}'.format(lim.FORMAT_VERSION)
        self._store.to_tar(path, prefix=prefix)

    def changed(self):
        """Whether the composition has changed since the last commit."""
        return self._history.at_commit()

    def flush(self, key):
        """Flush a previous store of a data value in the history.

        Arguments:
        key -- The key of the value.

        """
        self._history.flush(key)

    def cancel(self, key):
        """Cancel the storage of a pending data value in the history.

        Arguments:
        key -- The key of the value.

        """
        old_data = self._history.cancel(key)
        self._view.put(key, value)
        #self.set_raw(key, old_data)

    def save(self):
        self._history.set_commit()

    def start_group(self, name=''):
        """Marks the start of a group of modifications.

        Every call of start_group must always have a corresponding
        call of end_group, even in exceptional circumstances.

        Optional arguments:
        name -- The name of the change.

        """
        self._history.start_group(name)

    def end_group(self):
        """Marks the end of a group of modifications."""
        self._history.end_group()

    def undo(self):
        self._history.undo(self)
        #self._history.show_latest_branch()

    def redo(self, branch=None):
        self._history.redo(branch, self)
        #self._history.show_latest_branch()

    def pattern_ids(self):
        folders = [f.split('/')[0] for f in self._store.keys() if len(f.split('/')) > 1 and f.split('/')[1] == 'p_manifest.json']
        foo =  set([f for f in folders if f.startswith('pat')])
        return foo

    def song_ids(self):
        folders = [f.split('/')[0] for f in self._store.keys()]
        foo =  set([f for f in folders if f.startswith('subs')])
        return foo

    def instrument_ids(self):
        folders = [f.split('/')[0] for f in self._store.keys()]
        foo =  set([f for f in folders if f.startswith('ins')])
        return foo

    def song_ids(self):
        folders = [f.split('/')[0] for f in self._store.keys()]
        foo =  set([f for f in folders if f.startswith('song')])
        return foo

    def get_song(self, song_id):
        return Song(self, self.p, song_id)

    def left_over_patterns(self):
        system_ids = self.pattern_ids()
        systems = [int(i.split('_')[1]) for i in system_ids]
        all_systems = set(systems)

        in_use = set()
        for song_id in self.song_ids():
            song = self.get_song(song_id)
            order_list = song.get_order_list()
            in_use.update(order_list)

        left_over = all_systems - in_use
        left_over_ids = ['pat_{0:02x}'.format(i) for i in left_over]
        return left_over_ids

    def song_count(self):
        return len(self._tracks)

    def get_tracks(self):
        return self._tracks

    def get_pattern_instances(self):
        pattern_instances = []
        for track in self.get_tracks():
            song = self.get_song_by_track(track)
            order_list = song.get_order_list()
            pattern_instances += order_list
        return pattern_instances

    def set_tracks(self, tracks):
        self._view.put('album/p_tracks.json', tracks)

    def update_tracks(self, tracks_json):
        import json
        self._tracks = json.loads(tracks_json)
        try:
            songlist = self.p._sheet._subsongs
        except:
            return
        songlist.update_model()

    def delete_track(self, track):
        song = self.get_song_by_track(track)
        song.delete()
        tracks = self.get_tracks()
        del tracks[track]
        self.set_tracks(tracks)

    def get_track_by_song(self, song):
        song_ref = song.get_ref()
        track = self._tracks.index(song_ref)
        return track

    def get_song_by_track(self, track):
        song_number = self._tracks[track]
        song = self.get_song('song_%02d' % song_number)
        return song

    def move_track(self, track_number, target):
        print('move track %s to %s' % (track_number, target))
        tracks = tools.list_move(self._tracks, track_number, target)
        self.set_tracks(tracks)

    def delete_from_system(self, global_system):
        (track, system) = global_system
        song = self.get_song_by_track(track)
        song.delete_system(system)

    def _create_manifest(self, path):
        manifest_name = 'p_manifest.json'
        manifest_path = '%s/%s' % (path, manifest_name)
        manifest = {}
        self._view.put(manifest_path, manifest)

    def _init_pattern(self, pattern_id):
        instance = 0
        instance_folder = 'instance_%03d' % instance
        instance_path = '%s/%s' % (pattern_id, instance_folder)
        self._create_manifest(pattern_id)
        self._create_manifest(instance_path)
        pattern = int(pattern_id.split('_')[1])
        pattern_instance = (pattern, instance)
        return pattern_instance

    def new_pattern(self):
        existing = self.pattern_ids()
        pattern_ids = ('pat_%03d' % i for i in xrange(999))
        for pattern_id in pattern_ids:
            if pattern_id not in existing:
                pattern_instance = self._init_pattern(pattern_id)
                return pattern_instance
        raise Exception

    def move_system(self, global_system, global_target):
        print('move system %s to %s' % (global_system, global_target))
        (source_track, source_row) = global_system
        (target_track, target_place) = global_target
        if source_track == target_track:
            print('internal')
            song = self.get_song_by_track(source_track)
            song.move_system(source_row, target_place)
        else:
            print('from song %s to song %s' % (source_track, target_track))
            source_song = self.get_song_by_track(source_track)
            target_song = self.get_song_by_track(target_track)
            source_systems = source_song.get_order_list()
            target_systems = target_song.get_order_list()
            new_source_systems = list(source_systems)
            new_target_systems = list(target_systems)
            item = new_source_systems.pop(source_row)
            if item in target_systems:
                self.p.error('ERROR: Only one copy of pattern instance per song')
                return
            new_target_systems.insert(target_place, item)
            source_song.set_order_list(new_source_systems)
            target_song.set_order_list(new_target_systems)

