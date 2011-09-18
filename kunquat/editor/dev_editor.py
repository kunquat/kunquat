# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function
from itertools import takewhile
import re

from PyQt4 import QtCore, QtGui

import kqt_limits as lim
from envelope import Envelope
from param_sample_header import ParamSampleHeader
from param_sample_file import ParamSampleFile
from param_slider import ParamSlider
from param_wave import ParamWave
from sample_map import SampleMap


class DevEditor(QtGui.QSplitter):

    def __init__(self, project, constraints, key_base, parent=None):
        QtGui.QSplitter.__init__(self, parent)
        self._project = project
        self._key_base = key_base
        self._key_list = KeyList(project,
                                 self._key_base + 'c/')
        self._key_editor = KeyEditor(project, constraints)
        QtCore.QObject.connect(self._key_list,
                               QtCore.SIGNAL('keyChanged(QString)'),
                               self._key_editor.set_key)
        self.addWidget(self._key_list)
        self.addWidget(self._key_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([280, 1])

    def set_key_base(self, key_base):
        self._key_base = key_base
        self._key_list.set_key(self._key_base + 'c/')

    def sync(self):
        self._key_list.sync()


class KeyList(QtGui.QTableWidget):

    keyChanged = QtCore.pyqtSignal(QtCore.QString, name='keyChanged')

    def __init__(self, project, key, parent=None):
        QtGui.QTableWidget.__init__(self, 1, 1, parent)
        self._project = project
        self._current = ''
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()
        self.verticalHeader().hide()
        self.setTabKeyNavigation(False)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('itemChanged(QTableWidgetItem*)'),
                               self._item_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentItemChanged('
                                             'QTableWidgetItem*,'
                                             'QTableWidgetItem*)'),
                               self._set_current)
        self._lock_update = False
        self._key = ''
        self.set_key(key)

    def _set_current(self, item, old):
        prev_current = self._current
        if item:
            self._current = str(item.text())
        else:
            self._current = ''
        if self._current != prev_current:
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('keyChanged(QString)'),
                                self._key + self._current)

    def _item_changed(self, item):
        if self._lock_update:
            return
        self._lock_update = True
        if item:
            new_text = str(item.text())
            if new_text:
                if not lim.valid_key(new_text):
                    item.setText(self._current)
                    self._lock_update = False
                    return
                for row in xrange(self.rowCount()):
                    if row == self.currentRow():
                        if self._current == new_text:
                            self._lock_update = False
                            return
                        continue
                    i = self.item(row, 0)
                    if i and i.text() == new_text:
                        item.setText(self._current)
                        self._lock_update = False
                        return
        prev_current = self._current

        if self._current:
            key = self._key + self._current
            self._project[key] = None
        else:
            assert self.currentRow() == self.rowCount() - 1
            self.insertRow(self.rowCount())

        new_text = str(item.text()) if item else ''
        if new_text:
            self._current = new_text
        else:
            self.removeRow(self.currentRow())
            item = self.currentItem()
            self._current = str(item.text()) if item else ''
        self._lock_update = False

        if self._current != prev_current:
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('keyChanged(QString)'),
                                self._key + self._current)

    def set_key(self, key, force=False):
        prev_key = self._key
        prev_current = self._current
        self._key = key
        self._lock_update = True
        subtree = filter(lambda k: k[len(key):].split('/')[-1].startswith('p_'),
                         self._project.subtree(self._key))
        if not subtree:
            while self.rowCount() > 1:
                self.removeRow(0)
            if self.rowCount() == 0:
                self.insertRow(0)
            self.setItem(0, 0, QtGui.QTableWidgetItem())
            self._lock_update = False
            return
        new_rows = len(subtree) - self.rowCount()
        for _ in xrange(new_rows):
            self.insertRow(self.rowCount())
        for _ in xrange(-new_rows):
            self.removeRow(self.rowCount() - 1)
        for i, k in enumerate(subtree):
            key_trunc = k[len(self._key):]
            item = QtGui.QTableWidgetItem(key_trunc)
            self.setItem(i, 0, item)
        self.sortItems(0)
        self.insertRow(self.rowCount())
        self.setItem(self.rowCount() - 1, 0, QtGui.QTableWidgetItem())
        self._lock_update = False
        self._set_current(self.currentItem(), None)
        if prev_key != self._key or prev_current != self._current or force:
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('keyChanged(QString)'),
                                self._key + self._current)

    def sync(self):
        self.set_key(self._key, True)


class KeyEditor(QtGui.QStackedWidget):

    def __init__(self, project, constraints, parent=None):
        QtGui.QStackedWidget.__init__(self)
        self._project = project
        self._constraints = constraints
        self._key = ''
        self._slider = ParamSlider(project,
                                   '',
                                   (-100, 100),
                                   0,
                                   '',
                                   decimals=2)
        self._env = Envelope(project,
                             (0, 1),
                             (0, 1),
                             (False, False),
                             (False, False),
                             [[0, 1], [1, 0]],
                             16,
                             '')
        self._wave = ParamWave(project,
                               (-1, 1),
                               128,
                               '')
        self._sample_header = ParamSampleHeader(project, '')
        self._sample_file = ParamSampleFile(project, '')
        self._sample_map = SampleMap(project, '')
        self._default = QtGui.QWidget()
        self.addWidget(self._default)
        self.addWidget(self._env)
        self.addWidget(self._slider)
        self.addWidget(self._wave)
        self.addWidget(self._sample_header)
        self.addWidget(self._sample_file)
        self.addWidget(self._sample_map)
        self._map = {
                        'jsone': self._env,
                        'jsonf': self._slider,
                        'jsoni': self._slider,
                        'jsonln': self._wave,
                        'jsonsh': self._sample_header,
                        'jsonsm': self._sample_map,
                        'wv': self._sample_file,
                    }

    def set_key(self, key):
        key = str(key)
        self._key = key
        if not lim.valid_key(key):
            self.setCurrentWidget(self._default)
            return
        suffix = self._key.split('.')[-1]
        if suffix in self._map:
            current = self._map[suffix]
            constr = self.get_constraints(key)
            if constr:
                if suffix == 'jsoni' and 'decimals' in constr:
                    constr['decimals'] = 0
                current.set_constraints(constr)
            current.set_key(self._key)
            self.setCurrentWidget(current)
        else:
            self.setCurrentWidget(self._default)

    def sync(self):
        self.set_key(self._key)

    def get_constraints(self, key):
        if not lim.valid_key(key):
            return None
        components = key.split('/')
        dev_components = takewhile(lambda x: x != 'c', components)
        dev_path = '/'.join(dev_components)
        type_key = dev_path + '/p_{0}_type.json'.format('gen'
                if 'kqtg{0}'.format(lim.FORMAT_VERSION) in components
                else 'dsp')
        d = self._constraints[self._project[type_key]]
        if d == None:
            const_key = dev_path + '/i_constraints.json'
            d = self._project[const_key]
        if isinstance(d, dict):
            for k in d.iterkeys():
                if re.match(k, components[-1]):
                    return d[k]
        return None


