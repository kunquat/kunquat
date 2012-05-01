# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function

from collections import defaultdict
from copy import deepcopy
import errno
from itertools import izip, takewhile
import json
import os
import re
import tarfile
import time
import types

from PyQt4 import QtCore

import kqt_limits as lim
import kunquat


class Process(QtCore.QThread):

    def __init__(self, parent=None):
        QtCore.QThread.__init__(self, parent)
        self._reset()

    def process(self, func, *args):
        self._func = func
        self._args = args
        self.start()

    def _reset(self):
        self._func = None
        self._args = ()

    def run(self):
        self.setPriority(QtCore.QThread.LowPriority)
        if not self._func:
            self._reset()
            return
        func = self._func
        args = self._args
        self._reset()
        func(*args)


class Project(QtCore.QObject):

    """An abstraction for Kunquat Projects.

    A Project combines the functionalities of the Kunquat RWCHandle
    and History.

    """
    _start_task = QtCore.pyqtSignal(int, name='startTask')
    _step = QtCore.pyqtSignal(QtCore.QString, name='step')
    _end_task = QtCore.pyqtSignal(name='endTask')
    _sync = QtCore.pyqtSignal(name='sync')

    def __init__(self, proj_id, mixing_rate=48000, parent=None):
        """Create a new Project.

        Arguments:
        proj_id -- An ID number of the Project.  If the ID is already
                   in use, the Project will use the data associated
                   with that ID.  Otherwise an empty Project will be
                   created.

        Optional arguments:
        mixing_rate -- Mixing rate in frames per second.

        """
        QtCore.QObject.__init__(self, parent)
        self._process = Process()
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
        self.status_view = None
        self._callbacks = defaultdict(list)

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

    def subtree(self, prefix):
        """Return a sequence of all the keys inside a project subtree.

        Arguments:
        prefix -- The path of the subtree.

        Return value:
        The sequence of keys.

        """
        return (key for key in self._keys if key.startswith(prefix))

    def __getitem__(self, key):
        """Get data from the Kunquat Handle.

        Arguments:
        key -- The key of the data in the composition.

        Return value:
        The data associated with the key if found, otherwise None.

        """
        return self._handle[key]

    def __setitem__(self, key, value):
        """Set data in the Kunquat Handle and History.

        For JSON keys, this function converts the given Python object
        into a JSON string.

        Arguments:
        key   -- The key of the data in the composition.
        value -- The data to be set.

        """
        self.set(key, value)

    def __delitem__(self, key):
        """Remove data from the Kunquat Handle.

        Removing a key from a project is equivalent to setting the
        corresponding value to None.

        Arguments:
        key -- The key of the data in the composition.

        """
        self[key] = None

    def set(self, key, value, immediate=True, autoconnect=True):
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
        assert immediate in (True, False)
        if value == None:
            autoconnect = False
        if autoconnect:
            autoconnect = self._autoconnect(key, immediate)
        try:
            self._history.step(key, deepcopy(value), immediate=immediate)
            self.set_raw(key, value)
            """
            if value == None:
                self._history.step(key, '', immediate=immediate)
                self.set_raw(key, '')
            elif key[key.index('.'):].startswith('.json'):
                jvalue = json.dumps(value)
                self._history.step(key, jvalue, immediate=immediate)
                self.set_raw(key, jvalue)
            else:
                self._history.step(key, value, immediate=immediate)
                self.set_raw(key, value)
            """
        finally:
            if autoconnect:
                self._autoconnect_finish()
        self._changed = True
        #self._history.show_latest_branch()

    def update_random(self):
        """Update the automatic random seed."""
        custom = self['i_custom_random_seed.json']
        if custom:
            return
        value = self['p_random_seed.json']
        if not value:
            value = 0
        value += 1
        self.set_raw('p_random_seed.json', value)
        QtCore.QObject.emit(self, QtCore.SIGNAL('sync()'))

    def set_callback(self, event_name, func, *args):
        """Set a callback function for an event type."""
        self._callbacks[event_name].extend([(func, args)])

    def tfire(self, ch, event):
        """Mark an event fired."""
        if event[0].endswith('"'):
            event[0] = event[0][:-1]
        for func, args in self._callbacks[event[0]]:
            func(ch, event, *args)

    def _autoconnect(self, key, immediate):
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
            if not list(izip((1,), self.subtree(gen_prefix))):
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
        if not list(izip((1,), self.subtree(ins_prefix))):
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
        return True

    def _autoconnect_finish(self):
        self._history.end_group()
        QtCore.QObject.emit(self, QtCore.SIGNAL('sync()'))

    def set_raw(self, key, value):
        """Set raw data in the Project.

        This function does not update the History.

        """
        self._handle[key] = value
        if value:
            self._keys.add(key)
        else:
            self._keys.discard(key)

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
        self.set_raw(key, old_data)

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
        """Removes all composition data (but not the history).

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        """
        self._process.process(self._clear)

    def _clear(self):
        self._history.start_group('Clear all')
        try:
            self._remove_dir('')
        finally:
            self._history.end_group()

    def remove_dir(self, directory):
        """Removes a directory inside a composition.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        """
        self._process.process(self._remove_dir, directory)

    def _remove_dir(self, directory):
        self._history.start_group('Remove ' + directory)
        target_keys = [k for k in self._keys if k.startswith(directory)]
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'),
                            len(target_keys))
        try:
            for key in target_keys:
                QtCore.QObject.emit(self, QtCore.SIGNAL('step(QString)'),
                                    'Removing {0} ...'.format(key))
                self.set(key, None, autoconnect=False)
        finally:
            self._history.end_group()
            QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def export_kqt(self, dest):
        """Exports the composition in the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        Arguments:
        dest -- The destination file name.  If the name contains '.gz'
                or '.bz2' as a suffix, the file will be compressed
                using, respectively, gzip or bzip2.

        """
        self._process.process(self._export_kqt, dest)

    def _export_kqt(self, dest):
        root = 'kqtc' + lim.FORMAT_VERSION + '/'
        compression = ''
        if dest.endswith('.gz'):
            compression = 'gz'
        elif dest.endswith('.bz2'):
            compression = 'bz2'
        tfile = None
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'),
                            len(self._keys))
        try:
            tfile = tarfile.open(dest, 'w:' + compression,
                                 format=tarfile.USTAR_FORMAT)
            for key in self._keys:
                QtCore.QObject.emit(self, QtCore.SIGNAL('step(QString)'),
                        'Exporting {0}:{1} ...'.format(dest, key))
                kfile = KeyFile(root + key, self._handle[key])
                info = tarfile.TarInfo()
                info.name = root + key
                info.size = kfile.size
                info.mtime = int(time.mktime(time.localtime(time.time())))
                tfile.addfile(info, fileobj=kfile)
        finally:
            if tfile:
                tfile.close()
            QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def export_kqti(self, index, dest):
        """Exports an instrument in the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        Arguments:
        index -- The instrument number -- must be >= 0 and
                 < lim.INSTRUMENTS_MAX.
        dest  -- The destination file name.  If the name contains '.gz'
                 or '.bz2' as a suffix, the file will be compressed
                 using, respectively, gzip or bzip2.

        """
        self._process.process(self._export_kqti, index, dest)

    def _export_kqti(self, index, dest):
        root = 'kqtc{0}/'.format(lim.FORMAT_VERSION)
        ins_root = 'ins_{0:02x}/'.format(index)
        self._export_subtree(ins_root, dest,
                             'Exporting instrument {0}'.format(index), 'i')

    def export_kqte(self, base, index, dest):
        """Exports an effect in the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        Arguments:
        index -- The effect number -- must be >= 0 and
                 < lim.EFFECTS_MAX (global effect) or
                 < lim.INST_EFFECTS_MAX (instrument effect).
        dest  -- The destination file name.  If the name contains '.gz'
                 or '.bz2' as a suffix, the file will be compressed
                 using, respectively, gzip or bzip2.

        """
        self._process.process(self._export_kqte, base, index, dest)

    def _export_kqte(self, base, index, dest):
        eff_root = 'eff_{0:02x}/'.format(index)
        self._export_subtree(base + eff_root, dest,
                             'Exporting effect {0}'.format(index), 'e')

    def _export_subtree(self, prefix, dest, msg, ftype):
        assert not prefix or prefix.endswith('/')
        assert len(ftype) == 1
        assert ftype in 'ei'
        compression = ''
        if dest.endswith('.gz'):
            compression = 'gz'
        elif dest.endswith('.bz2'):
            compression = 'bz2'
        tfile = None
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'),
                            len(self._keys))
        try:
            tfile = tarfile.open(dest, 'w:' + compression,
                                 format=tarfile.USTAR_FORMAT)
            keys = [k for k in self._keys if k.startswith(prefix)]
            for key in keys:
                QtCore.QObject.emit(self, QtCore.SIGNAL('step(QString)'),
                        '{0} ({1}) ...'.format(msg, key))
                name = key[len(prefix):]
                kfile = KeyFile(name, self._handle[key])
                info = tarfile.TarInfo()
                info.name = name
                info.size = kfile.size
                info.mtime = int(time.mktime(time.localtime(time.time())))
                tfile.addfile(info, fileobj=kfile)
            if not keys:
                info = tarfile.TarInfo('kqt{0}{1}'.format(ftype,
                                                          lim.FORMAT_VERSION))
                info.type = tarfile.DIRTYPE
                info.mtime = int(time.mktime(time.localtime(time.time())))
                tfile.addfile(info)
        finally:
            if tfile:
                tfile.close()
            QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def import_kqt(self, src):
        """Imports a composition into the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        This function will replace any composition data the Project
        may contain before invocation.

        Arguments:
        src -- The source file name.

        """
        self._process.process(self._import_kqt, src)

    def _import_kqt(self, src):
        self._history.start_group('Import composition {0}'.format(src))
        tfile = None
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), 0)
        try:
            self._clear()
            if os.path.isdir(src):
                if not src or src[-1] != '/':
                    src = src + '/'
                for dir_spec in os.walk(src):
                    for fname in dir_spec[2]:
                        full_path = os.path.join(dir_spec[0], fname)
                        key = full_path[len(src):]
                        with open(full_path, 'rb') as f:
                            QtCore.QObject.emit(self,
                                    QtCore.SIGNAL('step(QString)'),
                                    'Importing {0} ...'.format(full_path))
                            if key[key.index('.'):].startswith('.json'):
                                self.set(key, json.loads(f.read()),
                                         autoconnect=False)
                            else:
                                self.set(key, f.read(), autoconnect=False)
            else:
                tfile = tarfile.open(src, format=tarfile.USTAR_FORMAT)
                entry = tfile.next()
                while entry:
                    if not re.match('kqtc[0-9a-f][0-9a-f]($|/)', entry.name):
                        raise kunquat.KunquatFormatError(
                                'The file is not a Kunquat composition')
                    if entry.name[4:6] != lim.FORMAT_VERSION:
                        raise kunquat.KunquatFormatError(
                                'Unsupported format version: {0}'.format(
                                                            entry.name[4:6]))
                    if entry.name.find('/') < 0:
                        entry = tfile.next()
                        continue
                    key = entry.name[entry.name.index('/') + 1:]
                    if entry.isfile():
                        QtCore.QObject.emit(self,
                                QtCore.SIGNAL('step(QString)'),
                                'Importing {0}:{1} ...'.format(src, key))
                        data = tfile.extractfile(entry).read()
                        if key[key.index('.'):].startswith('.json'):
                            self.set(key, json.loads(data), autoconnect=False)
                        else:
                            self.set(key, data, autoconnect=False)
                    entry = tfile.next()
        finally:
            if tfile:
                tfile.close()
            self._history.end_group()
            QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def import_kqti(self, index, src):
        """Imports a Kunquat instrument into the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        Arguments:
        index -- The index of the new instrument.  Any existing
                 instrument data will be removed before loading.
        src   -- The source file name.

        """
        self._process.process(self._import_kqti, index, src)

    def _import_kqti(self, index, src):
        assert index >= 0
        assert index < lim.INSTRUMENTS_MAX
        ins_path = 'ins_{0:02x}'.format(index)
        tfile = None
        self._history.start_group(
                'Load {0} into instrument {1:d}'.format(src, index))
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), 0)
        try:
            self._remove_dir(ins_path)
            if os.path.isdir(src):
                if not src or src[-1] != '/':
                    src = src + '/'
                key_base = '/'.join((ins_path, 'kqti' + lim.FORMAT_VERSION))
                for dir_spec in os.walk(src):
                    for fname in dir_spec[2]:
                        full_path = os.path.join(dir_spec[0], fname)
                        ins_key = '/'.join((key_base, full_path[len(src):]))
                        with open(full_path) as f:
                            QtCore.QObject.emit(self,
                                    QtCore.SIGNAL('step(QString)'),
                                    'Importing instrument {0} ...'.format(
                                        full_path))
                            if ins_key[ins_key.index('.'):].startswith(
                                                                '.json'):
                                self.set(ins_key, json.loads(f.read()),
                                         autoconnect=False)
                            else:
                                self.set(ins_key, f.read(), autoconnect=False)
            else:
                tfile = tarfile.open(src, format=tarfile.USTAR_FORMAT)
                entry = tfile.next()
                while entry:
                    #print(entry.name)
                    if not entry.name.startswith('kqti'):
                        raise kunquat.KunquatFormatError(
                                'The file is not a Kunquat instrument')
                    if entry.isfile():
                        key_path = '/'.join((ins_path, entry.name))
                        QtCore.QObject.emit(self,
                                QtCore.SIGNAL('step(QString)'),
                                'Importing instrument {0}:{1} ...'.format(src,
                                    key_path))
                        data = tfile.extractfile(entry).read()
                        if key_path[key_path.index('.'):].startswith('.json'):
                            self.set(key_path, json.loads(data),
                                     autoconnect=False)
                        else:
                            self.set(key_path, data, autoconnect=False)
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
            self.set('p_connections.json', connections, autoconnect=False)
        finally:
            if tfile:
                tfile.close()
            self._history.end_group()
            QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def import_kqte(self, base, index, src):
        """Imports a Kunquat effect into the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        Arguments:
        base  -- The base key path of the effect.
        index -- The index of the new effect.  Any existing effect data
                 will be removed before loading.
        src   -- The source file name.

        """
        self._process.process(self._import_kqte, base, index, src)

    def _import_kqte(self, base, index, src):
        max_index = lim.INST_EFFECTS_MAX if base.startswith('ins') \
                                         else lim.EFFECTS_MAX
        assert index >= 0
        assert index < max_index
        eff_path = '{0}eff_{1:02x}'.format(base, index)
        tfile = None
        if base:
            ins_num = int(base[4:6], 16)
            self._history.start_group(
                    'Load {0} into effect {1:d} of instrument {2:d}'.format(
                            src, index, ins_num))
        else:
            self._history.start_group(
                    'Load {0} into effect {1:d}'.format(src, index))
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), 0)
        try:
            self._remove_dir(eff_path)
            if os.path.isdir(src):
                if not src or src[-1] != '/':
                    src = src + '/'
                key_base = '/'.join((eff_path, 'kqte' + lim.FORMAT_VERSION))
                for dir_spec in os.walk(src):
                    for fname in dir_spec[2]:
                        full_path = os.path.join(dir_spec[0], fname)
                        eff_key = '/'.join((key_base, full_path[len(src):]))
                        with open(full_path) as f:
                            QtCore.QObject.emit(self,
                                    QtCore.SIGNAL('step(QString)'),
                                    'Importing effect {0} ...'.format(
                                        full_path))
                            if eff_key[eff_key.index('.'):].startswith(
                                                                '.json'):
                                self.set(eff_key, json.loads(f.read()),
                                         autoconnect=False)
                            else:
                                self.set(eff_key, f.read(), autoconnect=False)
            else:
                tfile = tarfile.open(src, format=tarfile.USTAR_FORMAT)
                entry = tfile.next()
                while entry:
                    #print(entry.name)
                    if not entry.name.startswith('kqte'):
                        raise kunquat.KunquatFormatError(
                                'The file is not a Kunquat effect')
                    if entry.isfile():
                        key_path = '/'.join((eff_path, entry.name))
                        QtCore.QObject.emit(self,
                                QtCore.SIGNAL('step(QString)'),
                                'Importing effect {0}:{1} ...'.format(src,
                                    key_path))
                        data = tfile.extractfile(entry).read()
                        if key_path[key_path.index('.'):].startswith('.json'):
                            self.set(key_path, json.loads(data),
                                     autoconnect=False)
                        else:
                            self.set(key_path, data, autoconnect=False)
                    entry = tfile.next()
        finally:
            if tfile:
                tfile.close()
            self._history.end_group()
            QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def save(self):
        """Saves the Project data.

        """
        self._handle.commit()
        self._history.set_commit()
        self._changed = False

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
        """Undoes a change made in the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        """
        self._process.process(self._undo)

    def _undo(self):
        self._history.undo(self)
        #self._history.show_latest_branch()

    def redo(self, branch=None):
        """Redoes a change made in the Project.

        NOTE: this function returns immediately.  Do not access the
              Project again until it emits the endTask() signal.

        Optional arguments:
        branch -- The branch of changes to follow.  The default is
                  None, in which case the last change used will be
                  selected.

        """
        self._process.process(self._redo, branch)

    def _redo(self, branch=None):
        self._history.redo(branch, self)
        #self._history.show_latest_branch()

    def __del__(self):
        self._handle = None


class KeyFile(object):

    def __init__(self, name, data):
        self.name = name
        if name[name.index('.'):].startswith('.json'):
            self._data = json.dumps(data)
        else:
            self._data = data
        self._cur = 0

    def read(self, size=-1):
        if size < 0:
            if not self._cur:
                self._cur = len(self._data)
                return self._data
            cur = self._cur
            self._cur = len(self._data)
            return self._data[cur:]
        cur = self._cur
        self._cur = min(self._cur + size, len(self._data))
        return self._data[cur:self._cur]

    @property
    def size(self):
        return len(self._data)

    def close(self):
        self._cur = len(self._data)


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
        self._pending = {}

    def at_commit(self):
        assert not self._group
        return self._commit == self._current

    def set_commit(self):
        assert not self._group
        self._commit = self._current

    def show_latest_branch(self):
        cur = self._root
        while cur:
            print(cur.name, '<-' if cur == self._current else '')
            cur = cur.child()

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
        #assert not self._group
        node = self._current.parent
        while node:
            yield node
            node = node.parent

    def step(self, key, new_data, name='', immediate=True):
        if not immediate:
            if key in self._pending:
                old_value = self._pending[key][1]
            else:
                old_value = self._project[key]
                if old_value == None:
                    old_value = ''
                elif key[key.index('.'):].startswith('.json'):
                    old_value = json.dumps(old_value)
            self._pending[key] = new_data, old_value, name
            #print('added pending change', key, '-',
            #      new_data if len(new_data) < 100 else new_data[:97] + '...')
            return

        old_value = self._project[key]
        if old_value == None:
            old_value = ''
        elif key[key.index('.'):].startswith('.json'):
            old_value = json.dumps(old_value)

        if key in self._pending:
            old_value = self._pending[key][1]
            del self._pending[key]
        """
        if self._pending:
            for k, value in self._pending.iteritems():
                if k == key:
                    continue
                pdata, pold, pname = value
                self._step(k, pdata, pold, pname)
                #print('changed pending', k, '-',
                #      pdata if len(pdata) < 100 else pdata[:97] + '...')
            if key in self._pending:
                old_value = self._pending[key][1]
            self._pending = {}
        """
        self._step(key, new_data, old_value, name)
        #print('added immediate change', key, '-',
        #      new_data if len(new_data) < 100 else new_data[:97] + '...')
        #for p in self.parents():
        #    print(p.name)

    def _step(self, key, new_data, old_data, name=''):
        if not self._group:
            self._current = Step(self._current, name)
        self._current.add_change(Change(key, old_data, new_data))

    def flush(self, key):
        if key not in self._pending:
            return
        new_data, old_data, name = self._pending.pop(key)
        self._step(key, new_data, old_data, name)
        for k in [c for c in self._pending.iterkeys()
                          if c.endswith('p_connections.json')]:
            new_data, old_data, name = self._pending.pop(k)
            self._step(k, new_data, old_data, name)

    def cancel(self, key):
        old_data = ''
        if key in self._pending:
            new_data, old_data, name = self._pending.pop(key)
        for k in [c for c in self._pending.iterkeys()
                          if c.endswith('p_connections.json')]:
            del self._pending[k]
        return old_data

    def undo(self, step_signaller=None):
        """Undoes a step."""
        assert not self._group
        if self._pending:
            if len(self._pending) > 1:
                self.start_group()
            while self._pending:
                k, value = self._pending.popitem()
                pdata, pold, pname = value
                self._step(k, pdata, pold, pname)
            if self._group:
                self.end_group()
            self.undo(step_signaller)
            return
        if not self._current.parent:
            return
            #raise RuntimeError('Nothing to undo')
        #print('undoing', self._current.name)
        if step_signaller:
            QtCore.QObject.emit(step_signaller,
                                QtCore.SIGNAL('startTask(int)'),
                                len(self._current.changes))
        try:
            for change in self._current.changes:
                #print('undoing', change.key)
                if step_signaller:
                    QtCore.QObject.emit(step_signaller,
                                        QtCore.SIGNAL('step(QString)'),
                                        'Undoing {0} ({1}) ...'.format(
                                            self._current.name, change.key))
                old_data = None
                if change.old_data:
                    if change.key[change.key.index('.'):].startswith('.json'):
                        old_data = json.loads(change.old_data)
                    else:
                        old_data = change.old_data
                self._project.set_raw(change.key, old_data)
                #self._project.handle[change.key] = change.old_data
        finally:
            if step_signaller:
                QtCore.QObject.emit(step_signaller,
                                    QtCore.SIGNAL('endTask()'))
        self._current = self._current.parent

    def redo(self, branch=None, step_signaller=None):
        """Redoes a step.

        Arguments:
        branch -- The index of the branch to be used, or None for
                  the last used branch in the current node.

        """
        assert not self._group
        if self._pending:
            # XXX: should we step() here?
            return
        child = self._current.child(branch)
        if not child:
            return
            #raise RuntimeError('Nothing to redo')
        #print('redoing', child.name)
        if step_signaller:
            QtCore.QObject.emit(step_signaller,
                                QtCore.SIGNAL('startTask(int)'),
                                len(child.changes))
        try:
            for change in child.changes:
                if step_signaller:
                    QtCore.QObject.emit(step_signaller,
                                        QtCore.SIGNAL('step(QString)'),
                                        'Redoing {0} ({1}) ...'.format(
                                            child.name, change.key))
                self._project.set_raw(change.key, change.new_data)
                #self._project.handle[change.key] = change.new_data
        finally:
            if step_signaller:
                QtCore.QObject.emit(step_signaller,
                                    QtCore.SIGNAL('endTask()'))
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
        for i, ch in enumerate(self._changes):
            if ch.key == change.key:
                self._changes[i] = Change(ch.key, ch.old_data,
                                          change.new_data)
                break
        else:
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
        The name of the step.  The default name is generated based on
        the modified keys.

        """
        if self._name:
            return self._name
        if not self._changes:
            return 'No change'
        prefix = self._changes[0].key
        for change in self._changes[1:]:
            common_len = sum(1 for _ in takewhile(lambda x: x[0] == x[1],
                                                  izip(prefix, change.key)))
            prefix = prefix[:common_len]
            """
            for i, ch in enumerate(prefix):
                if ch != key[i]:
                    prefix = prefix[:i]
                    break
            """
        if len(prefix) < len(self._changes[0].key):
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
        self._old_data = old_data if old_data != None else ''
        self._new_data = new_data if new_data != None else ''

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


