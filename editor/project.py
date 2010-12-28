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
        self._history = History(self)

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
        return self._history.at_commit()

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
            # TODO
            pass

    def __setitem__(self, key, value):
        """Set data in the Kunquat Handle.

        For JSON keys, this function converts the given Python object
        into a JSON string.

        Arguments:
        key   -- The key of the data in the composition.
        value -- The data to be set.

        """
        old_value = self._handle[key]
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
        self._history.step(key, old_value, value)
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
        self._history.start_group('Remove ' + directory)
        for key in [k for k in self._keys if k.startswith(directory)]:
            #print('removing', key)
            self[key] = None
        self._history.end_group()

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
        self._history.start_group(
                'Load {0} into instrument {1:d}'.format(src, index))
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
                #print(entry.name)
                if not entry.name.startswith('kqti'):
                    raise kunquat.KunquatFormatError('Invalid directory'
                                                     ' inside the instrument')
                if entry.isfile():
                    key_path = '/'.join((ins_path, entry.name))
                    data = tfile.extractfile(entry).read()
                    if key_path[key_path.index('.'):].startswith('.json'):
                        self[key_path] = json.loads(data)
                    else:
                        self[key_path] = data
                entry = tfile.next()
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
        self._history.end_group()

    def save(self):
        """Saves the Project data."""
        self._handle.commit()
        self._history.set_commit()
        self._changed = False

    def undo(self):
        """Undoes a change made in the Project."""
        self._history.undo()

    def redo(self, branch=None):
        """Redoes a change made in the Project.

        Optional arguments:
        branch -- The branch of changes to follow. The default is
                  None, in which case the last change used will be
                  selected.

        """
        self._history.redo(branch)

    def __del__(self):
        self._handle = None


class History(object):

    def __init__(self, project):
        """Create a new History.

        Arguments:
        project -- The Kunquat Project associated with this History.

        """
        self._project = project
        self._root = Step()
        self._current = self._root
        self._commit = self._root
        self._group = 0

    def at_commit(self):
        assert not self._group
        return self._commit == self._current

    def set_commit(self):
        assert not self._group
        self._commit = self._current

    def start_group(self, name=''):
        """Start a group of changes.

        Only the methods step() and end_group() may be called when a
        group of changes is being made.

        """
        self._group += 1
        if self._group > 1:
            return
        self._current = Step(self._current, name)

    def end_group(self):
        """End a group of changes."""
        assert self._group > 0
        self._group -= 1

    def rollback(self):
        assert self._group
        # TODO: rollback
        self._group = 0

    def parents(self):
        """Generate a sequence containing all ancestors of
        the current state.

        """
        assert not self._group
        node = self._current.parent
        while node:
            yield node
            node = node.parent

    def step(self, key, old_data, new_data, name=''):
        if not self._group:
            self._current = Step(self._current, name)
        self._current.add_change(Change(key, old_data, new_data))

    def undo(self):
        assert not self._group
        if not self._current.parent:
            raise RuntimeError('Nothing to undo')
        for change in self._current.changes:
            self._project[change.key] = change.old_data
        self._current = self._current.parent

    def redo(self, branch=None):
        assert not self._group
        child = self._current.child(branch)
        if not child:
            raise RuntimeError('Nothing to redo')
        for change in child.changes:
            self._project[change.key] = change.new_data
        self._current = child


class Step(object):

    """An editing step in a Kunquat Project."""

    def __init__(self, parent=None, name=''):
        """Create a new step.

        Optional arguments:
        parent  -- The step that precedes this step.
        name    -- The name of this step.

        """
        self._changes = []
        self._children = []
        self._last_child = None
        self._parent = parent
        if parent:
            parent._add_step(self)
        self._name = name

    def add_change(self, change):
        """Adds a change into the step.

        Arguments:
        change -- The change.

        """
        self._changes.append(change)

    @property
    def changes(self):
        return self._changes

    def child(self, index=None):
        if index == None:
            return self._last_child
        return self._children[index]

    @property
    def name(self):
        """Return the name of the step.

        Return value:
        The name of the step. The default name is generated based on
        the modified keys.

        """
        if self._name:
            return self._name
        if not self._children:
            return 'No change'
        prefix = self._children[0].key
        for key in self._children[1:]:
            for i, ch in enumerate(prefix):
                if ch != key[i]:
                    prefix = prefix[:i]
                    break
        if len(prefix) < len(self._children[0].key):
            prefix = prefix + '*'
        return prefix

    @name.setter
    def name(self, value):
        """Set a name for the step."""
        self._name = value

    @property
    def parent(self):
        return self._parent

    def _add_step(self, step):
        self._children.append(step)
        self._last_child = step


class Change(object):

    """A value change of a key inside a Kunquat Project."""

    def __init__(self, key, old_data, new_data):
        """Create a new Change.

        Arguments:
        key      -- The key modified.
        old_data -- The value of the key before modification.
        new_data -- The value of the key after modification.

        """
        self._key = key
        self._old_data = old_data
        self._new_data = new_data

    @property
    def key(self):
        """Return the key of the change."""
        return self._key

    @property
    def old_data(self):
        """Return the value of the key before this change."""
        return self._old_data

    @property
    def new_data(self):
        """Return the value of the key after this change."""
        return self._new_data


