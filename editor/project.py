# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function

import errno
import json
import tarfile
import os

import kqt_limits as lim
import kunquat


class Project(object):

    """An abstraction for Kunquat Projects.

    A Project combines the functionalities of the Kunquat RWCHandle
    and History.

    """

    def __init__(self, proj_id, mixing_rate=48000):
        """Create a new Project.

        Arguments:
        proj_id -- An ID number of the Project.  If the ID is already
                   in use, the Project will use the data associated
                   with that ID.  Otherwise an empty Project will be
                   created.

        Optional arguments:
        mixing_rate -- Mixing rate in frames per second.

        """
        self._root = os.path.join(os.path.expanduser('~'),
                                  '.kunquat', 'projects',
                                  '{0:08x}'.format(proj_id))
        try:
            os.makedirs(self._root)
        except OSError as err:
            if err.errno != errno.EEXIST:
                raise
        self._mixing_rate = mixing_rate
        self._handle = kunquat.RWCHandle(self._root, mixing_rate)
        self._handle.buffer_size = 1024
        self._find_keys()
        self._changed = False

    def _find_keys(self):
        """Synchronises the internal set of used keys.

        Using this method should only be necessary inside the __init__
        method.

        """
        keys = set()
        comp_root = os.path.join(self._root, 'committed', 'kqtc00')
        for path in (os.path.join(d[0], f) for d in
                     os.walk(comp_root) for f in d[2]):
            components = []
            assert path.startswith(comp_root) and path != comp_root
            head, tail = os.path.split(path)
            while head.startswith(comp_root):
                components.append(tail)
                head, tail = os.path.split(head)
            components.reverse()
            keys.add('/'.join(components))
        self._keys = keys

    @property
    def changed(self):
        """Whether the Project has changed since the last commit."""
        return self._changed

    @property
    def handle(self):
        """The Kunquat Handle associated with the Project."""
        return self._handle

    def __getitem__(self, key):
        """Get data from the Kunquat Handle.

        This function returns JSON keys as Python objects.

        Arguments:
        key -- The key of the data in the composition.

        Return value:
        The data associated with the key if found, otherwise None.

        """
        if key[key.index('.'):].startswith('.json'):
            value = self._handle[key]
            return json.loads(value) if value else None
        else:
            pass

    def __setitem__(self, key, value):
        """Set data in the Kunquat Handle.

        For JSON keys, this function converts the given Python object
        into a JSON string.

        Arguments:
        key   -- The key of the data in the composition.
        value -- The data to be set.

        """
        # TODO: update history
        if value == None:
            self._handle[key] = ''
            self._keys.discard(key)
        elif key[key.index('.'):].startswith('.json'):
            js = json.dumps(value)
            self._handle[key] = js
            if js:
                self._keys.add(key)
            else:
                self._keys.discard(key)
        else:
            self._handle[key] = value # FIXME: conversion
            if value:
                self._keys.add(key)
            else:
                self._keys.discard(key)
        self._changed = True

    @property
    def mixing_rate(self):
        """Mixing rate in frames per second."""
        return self._mixing_rate

    @mixing_rate.setter
    def mixing_rate(self, value):
        """Set the mixing rate."""
        self._handle.mixing_rate = value
        self._mixing_rate = value

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

    def clear(self):
        """Removes all composition data (but not the history)."""
        self.remove_dir('')

    def remove_dir(self, directory):
        """Removes a directory inside a composition."""
        for key in [k for k in self._keys if k.startswith(directory)]:
            #print('removing', key)
            self[key] = None

    def export_kqt(self, dest):
        """Exports the composition in the Project.

        Arguments:
        dest -- The destination file name.  If the name contains '.gz'
                or '.bz2' as a suffix, the file will be compressed
                using, respectively, gzip or bzip2.

        """
        pass

    def import_kqt(self, src):
        """Imports a composition into the Project.

        This function will replace any composition data the Project
        may contain before invocation.

        Arguments:
        src -- The source file name.

        """
        pass

    def import_kqti(self, index, src):
        """Imports a Kunquat instrument into the Project.

        Arguments:
        index -- The index of the new instrument. Any existing
                 instrument data will be removed before loading.
        src   -- The source file name.

        """
        assert index >= 0
        assert index < lim.INSTRUMENTS_MAX
        ins_path = 'ins_{0:02x}'.format(index)
        self.remove_dir(ins_path)
        if os.path.isdir(src):
            if not src or src[-1] != '/':
                src = src + '/'
            key_base = '/'.join((ins_path, 'kqti' + lim.FORMAT_VERSION))
            for dir_spec in os.walk(src):
                for fname in dir_spec[2]:
                    full_path = os.path.join(dir_spec[0], fname)
                    ins_key = '/'.join((key_base, full_path[len(src):]))
                    with open(full_path) as f:
                        if ins_key[ins_key.index('.'):].startswith('.json'):
                            self[ins_key] = json.loads(f.read())
                        else:
                            self[ins_key] = f.read()
        else:
            tfile = tarfile.open(src, format=tarfile.USTAR_FORMAT)
            entry = tfile.next()
            while entry:
                if not entry.name.startswith('kqti'):
                    raise kunquat.KunquatFormatError('Invalid directory'
                                                     ' inside the instrument')
                if entry.isfile():
                    key_path = '/'.join((ins_path, entry.name))
                    self[key_path] = tfile.extractfile(entry).read()
                tfile.next()
        connections = self['p_connections.json']
        if not connections:
            connections = []
        ins_out = ins_path + '/kqtiXX/out_00'
        for connection in connections:
            if ins_out in connection:
                break
        else:
            connections.append([ins_out, 'out_00'])
            self['p_connections.json'] = connections

    def save(self):
        """Saves the Project data."""
        self._handle.commit()
        self._changed = False

    def undo(self):
        """Undoes a change made in the Project."""
        pass

    def redo(self, branch=None):
        """Redoes a change made in the Project.

        Optional arguments:
        branch -- The branch of changes to follow. The default is
                  None, in which case the last change used will be
                  selected.

        """
        pass

    def __del__(self):
        self._handle = None


