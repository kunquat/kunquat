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

import kunquat.editor.kqt_limits as lim
from kunquat.editor.param_slider import ParamSlider


class GenGeneric(QtGui.QSplitter):

    def __init__(self, project, parent=None):
        QtGui.QSplitter.__init__(self, parent)
        self._project = project
        self._cur_inst = 0
        self._cur_gen = 0
        self._key_temp = ('ins_{{0:02x}}/kqti{0}/'
                          'gen_{{1:02x}}/kqtg{0}/'.format(lim.FORMAT_VERSION))
        self._key_base = self._key_temp.format(self._cur_inst, self._cur_gen)
        self._key_list = KeyList(project,
                                 self._key_base + 'c/')
        self._key_editor = KeyEditor(project)
        QtCore.QObject.connect(self._key_list,
                               QtCore.SIGNAL('keyChanged(QString)'),
                               self._key_editor.set_key)
        self.addWidget(self._key_list)
        self.addWidget(self._key_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([280, 1])

    def inst_changed(self, num):
        self._cur_inst = num
        self._update_keys()

    def gen_changed(self, num):
        self._cur_gen = num
        self._update_keys()

    def _update_keys(self):
        self._key_base = self._key_temp.format(self._cur_inst, self._cur_gen)
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
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('itemChanged(QTableWidgetItem*)'),
                               self._item_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentItemChanged('
                                             'QTableWidgetItem*,'
                                             'QTableWidgetItem*)'),
                               self._set_current)
        self._lock_update = False
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

    def set_key(self, key):
        self._key = key
        self._lock_update = True
        while self.rowCount() > 0:
            self.removeRow(0)
        self.setItem(0, 0, QtGui.QTableWidgetItem())
        for key in list(self._project.subtree(self._key)):
            key_trunc = key[len(self._key):]
            if not key_trunc.split('/')[-1].startswith('p_'):
                continue
            item = QtGui.QTableWidgetItem(key_trunc)
            self.insertRow(self.rowCount())
            self.setItem(self.rowCount() - 1, 0, item)
        self.sortItems(0)
        self.insertRow(self.rowCount())
        self._lock_update = False

    def sync(self):
        self.set_key(self._key)


class KeyEditor(QtGui.QStackedWidget):

    def __init__(self, project, parent=None):
        QtGui.QStackedWidget.__init__(self)
        self._project = project
        self._key = ''
        self._slider = ParamSlider(project,
                                   '',
                                   (-100, 100),
                                   0,
                                   '',
                                   decimals=2)
        self._default = QtGui.QWidget()
        self.addWidget(self._default)
        self.addWidget(self._slider)
        self._map = {
                        'jsonf': self._slider,
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
            constraints = self.get_constraints(key)
            if constraints:
                current.set_constraints(constraints)
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
        gen_components = takewhile(lambda x: x != 'c', components)
        const_key = '/'.join(gen_components) + '/i_constraints.json'
        d = self._project[const_key]
        if isinstance(d, dict):
            for k in d.iterkeys():
                if re.match(k, components[-1]):
                    return d[k]
        return None


