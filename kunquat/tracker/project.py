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

import os
import re
from collections import defaultdict

from PyQt4 import QtCore

import kunquat
from kunquat.storage import storage, store
from composition import Composition

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

    def __init__(self, p, parent=None):
        QtCore.QObject.__init__(self, parent)
        self.p = p

    def init(self, file_path=None, mixing_rate=48000):
        """Create a new Project.

        Optional arguments:
        mixing_rate -- Mixing rate in frames per second.

        """
        self._process = Process()
        self._mixing_rate = mixing_rate

        root_path = os.path.join(
                os.path.expanduser('~'),
                '.kunquat', 'projects')
        self._handle = kunquat.Handle(mixing_rate)
        self._handle.buffer_size = 1024

        self._changed = False
        self.status_view = None
        self._callbacks = defaultdict(list)

        projects = storage.Storage(root_path, prefix='kqtc00', create=True)
        store_callbacks = [self.from_store]
        projects.get_store(file_path, callbacks=store_callbacks)

    @property
    def changed(self):
        return self._composition.changed()

    @property
    def handle(self):
        """The Kunquat Handle associated with the Project."""
        return self._handle

    def __getitem__(self, key):
        return self._composition.get(key)

    def __setitem__(self, key, value):
        if value == None:
            self._composition.delete(key)
        else:
            self.set(key, value)

    def __delitem__(self, key):
        self._composition.delete(key)

    def set(self, key, value, immediate=True, autoconnect=True):
        self._composition.put(key, value, immediate, autoconnect)

    def subtree(self, prefix):
        return self._composition.subtree(prefix)

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

    def set_raw(self, key, value):
        """Set raw data in the Project.

        This function does not update the History.

        """
        self._composition[key] = value

    def flush(self, key):
        self._composition.flush(key)

    def cancel(self, key):
        self._composition.cancel()

    @property
    def mixing_rate(self):
        """Mixing rate in frames per second."""
        return self._mixing_rate

    @mixing_rate.setter
    def mixing_rate(self, value):
        """Set the mixing rate."""
        self._handle.mixing_rate = value
        self._mixing_rate = value

    def save(self):
        """Saves the Project data.

        """
        self._store.commit()
        self._composition.save()
        self._changed = False

    def start_group(self, name=''):
        self._composition.start_group(name)

    def end_group(self):
        self._composition.end_group()

    def _undo(self):
        self._composition.undo()

    def _redo(self, branch=None):
        self._composition.redo(branch)

    def __del__(self):
        self._handle = None

    def _update_player(self, key):
        value = self._composition.get(key)
        self._handle.set_data(key, value)

    # STORE EVENT INTERFACE

    def _store_init(self, store, **_):
        self._store = store
        self._composition = Composition(store)

    def _store_value_update(self, key, **_):
        self._update_player(key)
        self.p._toolbar.update_songs()
        self.p._toolbar.update_instruments()
        self.p._toolbar.update_scales()
        QtCore.QObject.emit(self, QtCore.SIGNAL('sync()'))

    def _store_import_start(self, prefix, path, key_names, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), len(key_names))
        self._composition.start_group('import:%s' % prefix)

    def _store_import_status(self, dest, key, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('step(QString)'), 'Importing {0}:{1} ...'.format(dest, key))

    def _store_import_end(self, prefix, **_):
        self._composition.fix_connections(prefix)
        self._composition.end_group()
        QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))

    def _store_export_start(self, key_names, **_):
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), len(key_names))

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

    # EXPORT/IMPORT INTERFACE

    def _export_kqt(self, dest):
        self._composition.to_tar(dest)

    def _export_kqti(self, index, dest):
        instrument = self._composition.get_instrument(index)
        instrument.to_tar(dest, prefix='kqti00')

    def _export_kqte(self, base, index, dest):
        effect = self._composition.get_effect(base, index)
        effect.to_tar(dest, prefix='kqte00')

    def _import_kqti(self, index, src):
        instrument = self._composition.get_instrument(index)
        instrument.from_path(src, prefix='kqti00')

    def _import_kqte(self, base, index, src):
        effect = self._composition.get_effect(base, index)
        effect.from_path(src, prefix='kqte00')

    # PROCESS WRAPPER INTERFACE
    #
    #  NOTE: The following wrappers return immediately. Do not access the
    #        Project again until it emits the endTask() signal.

    def export_kqt(self, dest):
        self._process.process(self._export_kqt, dest)

    def export_kqti(self, index, dest):
        self._process.process(self._export_kqti, index, dest)

    def export_kqte(self, base, index, dest):
        self._process.process(self._export_kqte, base, index, dest)

    def import_kqti(self, index, src):
        self._process.process(self._import_kqti, index, src)

    def import_kqte(self, base, index, src):
        self._process.process(self._import_kqte, base, index, src)

    def undo(self):
        self._process.process(self._undo)

    def redo(self, branch=None):
        self._process.process(self._redo, branch)
