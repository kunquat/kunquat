# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2012
#         Toni Ruottu,       Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os
import re
from collections import defaultdict
from copy import deepcopy
from itertools import izip, takewhile

from PyQt4 import QtCore

import kunquat
import kqt_limits as lim
from kunquat.storage import storage, store
from history import History

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

    A Project combines the functionalities of libkunquat and storage.

    """
    _start_task = QtCore.pyqtSignal(int, name='startTask')
    _step = QtCore.pyqtSignal(QtCore.QString, name='step')
    _end_task = QtCore.pyqtSignal(name='endTask')
    _sync = QtCore.pyqtSignal(name='sync')

    def __init__(self, file_path=None, mixing_rate=48000, parent=None):
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
        self._mixing_rate = mixing_rate

        root_path = os.path.join(
                os.path.expanduser('~'),
                '.kunquat', 'projects')
        self._handle = kunquat.MHandle(mixing_rate)
        self._handle.buffer_size = 1024
        projects = storage.Storage(root_path, create=True)
        store_callbacks = [self.from_store]
        projects.get_store(file_path, callbacks=store_callbacks)

        self._changed = False
        self._history = History(self)
        self.status_view = None
        self._callbacks = defaultdict(list)

    # STORE EVENT INTERFACE

    def _store_init(self, store, **_):
        self._store = store
        root = '/kqtc{0}'.format(lim.FORMAT_VERSION)
        self._composition = self._store.get_view(root)

    def _store_value_update(self, key, **_):
        parts = key.split('/')
        root = parts.pop(0)
        kqtcxx = parts.pop(0)
        assert root == ''
        assert kqtcxx.startswith('kqtc')
        path = '/'.join(parts)
        value = self.get(path)
        self._handle.set_data(path, value)

    def _store_import_start(self, path, key_names, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), len(key_names))
        #self._history.start_group('Import composition {0}'.format(src))
        #self._history.start_group('Load {0} into instrument {1:d}'.format(src, index))
        #self._history.start_group('Load {0} into effect {1:d} of instrument {2:d}'.format(src, index, ins_num))
        #self._history.start_group('Load {0} into effect {1:d}'.format(src, index))


    def _store_import_status(self, dest, key, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('step(QString)'), 'Importing {0}:{1} ...'.format(dest, key))

    def _store_import_end(self, **_):
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
        #self._history.end_group()
        QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def _store_export_start(self, keycount, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), keycount)

    def _store_export_status(self, dest, key, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('step(QString)'), 'Exporting {0}:{1} ...'.format(dest, key))

    def _store_export_end(self, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def from_store(self, event):
        etype = event.__class__.__name__.lower()
        handler_name = '_store_%s' % etype
        if handler_name in dir(self):
            handler = getattr(self, handler_name)
            handler(**event)

    # STORE EVENT INTERFACE ENDS

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
        view = self._store.get_view(prefix)
        return view.keys()

    def __getitem__(self, key):
        """Get data from the Kunquat Handle.

        Arguments:
        key -- The key of the data in the composition.

        Return value:
        The data associated with the key if found, otherwise None.

        """
        return self.get(key)

    def get(self, key):
        suffix = key.split('.').pop()
        is_json = suffix.startswith('json')
        if is_json:
            value = self._composition.get_json(key)
        else:
            value = self._composition.get(key)
        return value if value else None

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
        self._composition[key] = value

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
        self._store.to_tar(dest)

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
        root = 'kqtc{0}'.format(lim.FORMAT_VERSION)
        ins = 'ins_{0:02x}'.format(index)
        prefix = '%s/%s/' % (root,ins)
        self._store.to_tar(dest, key_prefix=prefix)

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
        eff = 'eff_{0:02x}'.format(index)
        root = 'kqtc{0}'.format(lim.FORMAT_VERSION)
        prefix = '%s/%s%s/' % (root,base,eff)
        self._store.to_tar(dest, key_prefix=prefix)

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
        self._store.from_path(src)

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
        root = 'kqtc{0}'.format(lim.FORMAT_VERSION)
        ins = 'ins_{0:02x}'.format(index)
        prefix = '/%s/%s' % (root,ins)
        self._store.from_path(src, key_prefix=prefix)

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
        eff = 'eff_{0:02x}'.format(index)
        root = 'kqtc{0}'.format(lim.FORMAT_VERSION)
        prefix = '/%s/%s%s' % (root,base,eff)
        self._store.from_path(src, key_prefix=prefix)

    def save(self):
        """Saves the Project data.

        """
        self._store.commit()
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


