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

from __future__ import print_function
import re

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim


class Env(QtGui.QTableWidget):

    tsre = re.compile('\(?([0-9]*) *, *([0-9]*)\)?$')

    def __init__(self,
                 project,
                 key,
                 parent=None):
        QtGui.QTableWidget.__init__(self, 0, 4, parent)
        self._project = project
        self.setHorizontalHeaderLabels(['Type',
                                        'Name',
                                        'Initial value',
                                        'Current value'])
        #self.horizontalHeader().setStretchLastSection(True)
        for i in (1, 2, 3):
            self.horizontalHeader().setResizeMode(i, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        self.set_key(key)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._data_changed)

    def set_key(self, key):
        self.blockSignals(True)
        self._key = key
        var_list = self._project[self._key] or []
        self._resize_table(len(var_list) + 1)
        for i, v in enumerate(var_list):
            var_type, var_name, var_init = v
            self._set_row(i, var_type, var_name, var_init)
        self._set_row(self.rowCount() - 1)
        for i in (1, 2, 3):
            self.setItem(self.rowCount() - 1, i, QtGui.QTableWidgetItem())
        self.blockSignals(False)

    def sync(self):
        self.set_key(self._key)

    def _resize_table(self, length):
        new_rows = length - self.rowCount()
        for _ in xrange(new_rows):
            self.insertRow(self.rowCount())
            self.setCellWidget(self.rowCount() - 1, 0,
                               TypeSelect(self.rowCount() - 1))
        for _ in xrange(-new_rows):
            self.removeRow(self.rowCount() - 1)

    def _set_row(self, index, var_type=None, var_name=None, var_init=None):
        select = self.cellWidget(index, 0)
        QtCore.QObject.disconnect(select,
                                  QtCore.SIGNAL('typeChanged(int, QString*)'),
                                  self._type_changed)
        select.index = index
        format_changed = select.type_format != var_type
        select.type_format = var_type
        if not var_name:
            if not var_type or not self.item(index, 1):
                var_name = ''
            else:
                var_name = self.item(index, 1).text()
        self.setItem(index, 1, QtGui.QTableWidgetItem(var_name))
        if var_type == 'bool':
            cb_init = QtGui.QCheckBox()
            cb_cur = QtGui.QCheckBox()
            QtCore.QObject.connect(cb_init,
                                   QtCore.SIGNAL('clicked()'),
                                   self._flatten)
            if var_init:
                cb_init.setCheckState(QtCore.Qt.Checked)
                cb_cur.setCheckState(QtCore.Qt.Checked)
            self.setCellWidget(index, 2, cb_init)
            self.setCellWidget(index, 3, cb_cur)
        else:
            var_init = str(var_init or '')
            if var_type in ('int', 'float'):
                if not var_init:
                    var_init = '0'
            elif var_type == 'timestamp':
                if not var_init:
                    var_init = '0, 0'
            for i in (2, 3):
                self.removeCellWidget(index, i)
                self.setItem(index, i, QtGui.QTableWidgetItem(var_init))
        QtCore.QObject.connect(select,
                               QtCore.SIGNAL('typeChanged(int, QString*)'),
                               self._type_changed)

    def _type_changed(self, index, value):
        self.blockSignals(True)
        #print('changed', index, value)
        if value:
            self._set_row(index, str(value))
            if index == self.rowCount() - 1:
                self._resize_table(self.rowCount() + 1)
                self._set_row(self.rowCount() - 1)
        else:
            self.removeRow(index)
            for i in xrange(index, self.rowCount()):
                self.cellWidget(i, 0).index = i
        self._flatten()
        self.blockSignals(False)

    def _data_changed(self, row, col):
        self._flatten()

    def _flatten(self):
        var_list = []
        for i in xrange(self.rowCount()):
            var_type = self.cellWidget(i, 0).type_format
            var_name = str(self.item(i, 1).text())
            if not var_type or not var_name:
                continue
            try:
                if var_type == 'bool':
                    var_init = (self.cellWidget(i, 2).checkState() ==
                                QtCore.Qt.Checked)
                elif var_type == 'int':
                    var_init = int(self.item(i, 2).text())
                elif var_type == 'float':
                    var_init = float(self.item(i, 2).text())
                elif var_type == 'timestamp':
                    mo = Env.tsre.match(self.item(i, 2).text())
                    if not mo:
                        continue
                    var_init = [int(mo.group(1)), int(mo.group(2))]
                    if not 0 <= var_init[1] < lim.TIMESTAMP_BEAT:
                        continue
                else:
                    continue
            except ValueError:
                continue
            var_list.extend([[var_type, var_name, var_init]])
        print(var_list)


class TypeSelect(QtGui.QComboBox):

    typeChanged = QtCore.pyqtSignal(int, str, name='typeChanged')

    type_to_index = {
                None: 0,
                'bool': 1,
                'int': 2,
                'float': 3,
                'timestamp': 4,
            }

    index_to_type = dict((y, x) for (x, y) in
                         type_to_index.iteritems())

    def __init__(self, index, parent=None):
        QtGui.QComboBox.__init__(self, parent)
        self._index = index
        for item in ('None', 'Boolean', 'Integer', 'Floating', 'Timestamp'):
            self.addItem(item)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentIndexChanged(int)'),
                               self._type_changed)

    @property
    def index(self):
        return self._index

    @index.setter
    def index(self, value):
        self._index = value

    @property
    def type_format(self):
        return TypeSelect.index_to_type[self.currentIndex()]

    @type_format.setter
    def type_format(self, type_name):
        self.setCurrentIndex(TypeSelect.type_to_index[type_name])

    def _type_changed(self, index):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('typeChanged(int, QString*)'),
                            self.index,
                            TypeSelect.index_to_type[index])


class EnvVar(QtCore.QObject):

    removed = QtCore.pyqtSignal(int, name='removed')
    modified = QtCore.pyqtSignal(int, name='modified')
    finished = QtCore.pyqtSignal(int, name='finished')

    def __init__(self, index, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.index = index
        #self._layout = QtGui.QHBoxLayout(self)
        self.typew = QtGui.QComboBox()
        for t in ('Boolean', 'Integer', 'Floating', 'Timestamp'):
            self.typew.addItem(t)
        #self._layout.addWidget(self._type)
        self.namew = QtGui.QLineEdit()
        #self._layout.addWidget(self._name)
        self.init_valuew = QtGui.QLineEdit()
        #self._layout.addWidget(self._init_value)
        self.cur_valuew = QtGui.QLineEdit()
        #self._layout.addWidget(self._cur_value)
        self.removew = QtGui.QPushButton('Remove')
        #self._layout.addWidget(self._remove)

    @property
    def index(self):
        return self._index

    @index.setter
    def index(self, idx):
        self._index = idx

    @property
    def type(self):
        return self.typew.itemText(self.var_type.currentIndex())

    @property
    def name(self):
        return self.namew.text()

    @property
    def init_value(self):
        return self.init_valuew.text()

    @property
    def cur_value(self):
        return self.cur_valuew.text()


