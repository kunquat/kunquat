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

from PyQt4 import QtCore, QtGui


"""
class Env(QtGui.QScrollArea):

    def __init__(self,
                 project,
                 key,
                 parent=None):
        QtGui.QScrollArea.__init__(self, parent)
        el = EnvList(project, key)
        self.setWidget(el)

    def set_key(self, key):
        self.widget().set_key(key)

    def sync(self):
        self.widget().sync()
"""


class Env(QtGui.QTableWidget):

    type_to_index = {
                'bool': 1,
                'int': 2,
                'float': 3,
                'timestamp': 4,
            }

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

    def set_key(self, key):
        self.blockSignals(True)
        self._key = key
        var_list = self._project[self._key] or []
        self._resize_table(len(var_list) + 1)
        for i, v in enumerate(var_list):
            var_type, var_name, var_initial = v
            select = self.cellWidget(i, 0)
            select.index = i
            select.setCurrentIndex(Env.type_to_index[var_type])
            self.setItem(i, 1, QtGui.QTableWidgetItem(var_name))
            self.setItem(i, 2, QtGui.QTableWidgetItem(var_initial))
            self.setItem(i, 3, QtGui.QTableWidgetItem(var_initial))
        select = self.cellWidget(self.rowCount() - 1, 0)
        select.index = self.rowCount() - 1
        select.setCurrentIndex(0)
        for i in (1, 2, 3):
            self.setItem(self.rowCount() - 1, i, QtGui.QTableWidgetItem())
        self.blockSignals(False)

    def add_var(self, type_index):
        pass

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


class TypeSelect(QtGui.QComboBox):

    def __init__(self, index, parent=None):
        QtGui.QComboBox.__init__(self, parent)
        self._index = index
        for item in ('None', 'Boolean', 'Integer', 'Floating', 'Timestamp'):
            self.addItem(item)

    @property
    def index(self):
        return self._index

    @index.setter
    def index(self, value):
        self._index = value


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


