# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2010-2012
#          Toni Ruottu,       Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import izip, takewhile
import kqt_limits as lim
import re

class Composition():

    def __init__(self, store):
        root = '/kqtc{0}'.format(lim.FORMAT_VERSION)
        self._store = store
        self._view = store.get_view(root)

    def get(self, key):
        suffix = key.split('.').pop()
        is_json = suffix.startswith('json')
        if is_json:
            value = self._view.get_json(key)
        else:
            value = self._view.get(key)
        return value if value else None

    def __getitem__(self, key):
        return self.get(key)

    def put(self, key, value):
        return self._view.put(key, value)

    def __setitem__(self, key, value):
        return self.put(key,value)

    def get_pattern(self, subsong, section):
        """Get a pattern number based on subsong and section number."""
        if subsong < 0 or subsong >= lim.SUBSONGS_MAX:
            raise IndexError, 'Invalid subsong number'
        if section < 0 or section >= lim.SECTIONS_MAX:
            raise IndexError, 'Invalid section number'
        ss = self['subs_{0:02x}/p_subsong.json'.format(subsong)]
        if not ss or 'patterns' not in ss:
            return None
        patterns = ss['patterns']
        if len(patterns) <= section:
            return None
        return patterns[section]

    def get_effect(self, base_path, index):
        name = 'eff_{0:02x}'.format(index)
        base = self._view.get_view(base_path)
        effect = base.get_view(name)
        return effect

    def get_instrument(self, index):
        name = 'ins_{0:02x}'.format(index)
        instrument = self._view.get_view(name)
        return instrument

    def autoconnect(self, key, immediate):
        new_ins = -1
        new_gen = -1
        ins_conn_base = 'ins_{0:02x}/kqtiXX/'
        gen_conn_base = 'gen_{0:02x}/kqtgXX/C/'
        ins_prefix_base = 'ins_{{0:02x}}/kqti{0}/'.format(lim.FORMAT_VERSION)
        gen_prefix_base = '{0}gen_{{1:02x}}/kqtg{1}/'.format(ins_prefix_base,
                                                        lim.FORMAT_VERSION)
        ins_pattern = 'ins_([0-9a-f]{{2}})/kqti{0}/'.format(lim.FORMAT_VERSION)
        gen_pattern = '{0}gen_([0-9a-f]{{2}})/kqtg{1}/'.format(ins_pattern,
                                                        lim.FORMAT_VERSION)
        ins_mo = re.match(ins_pattern, key)
        if not ins_mo:
            return False
        new_ins = int(ins_mo.group(1), 16)
        ins_prefix = ins_prefix_base.format(new_ins)
        gen_mo = re.match(gen_pattern, key)
        if gen_mo:
            new_gen = int(gen_mo.group(2), 16)
            gen_prefix = gen_prefix_base.format(new_ins, new_gen)
            subtree = self._store.get_view(gen_prefix).keys()
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
        subtree = self._store.get_view(ins_prefix).keys()
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
            self.set('p_connections.json', connections, immediate=immediate,
                     autoconnect=False)
        if new_gen >= 0:
            self.set(ins_prefix + 'p_connections.json', ins_connections,
                     immediate=immediate, autoconnect=False)
        self._history.end_group()
        return True

    def fix_connections(self, prefix):
        '''
        Fixes connections after a successfull import at a certain location
        '''
        pass
        '''
            connections = self['p_connections.json']
            if not connections:
                connections = []
            ins_out = ins_path + '/kqtiXX/out_00'
            for connection in connections:
                if ins_out in connection:
                    break
            else:
                connections.append([ins_out, 'out_00'])
            self.set('p_connections.json', connections, autoconnect=False)
        '''

    def to_tar(self, path):
        self._store.to_tar(path)
        

