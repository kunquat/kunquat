# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4 import QtCore, QtGui

import kunquat.editor.trigtypes as ttypes


class CallMap(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._fe = FiringEvents()
        layout.addWidget(self._fe, 0)
        self._bindspec = BindSpec()
        layout.addWidget(self._bindspec, 1)
        QtCore.QObject.connect(self._fe,
                               QtCore.SIGNAL('eventChanged(int)'),
                               self._fe_changed)
        QtCore.QObject.connect(self._fe,
                               QtCore.SIGNAL('nameChanged(int, QString*)'),
                               self._fe_name_changed)
        QtCore.QObject.connect(self._bindspec,
                               QtCore.SIGNAL('bindChanged()'),
                               self._bind_changed)
        self.set_key('p_call_map.json')

    def set_key(self, key):
        self._key = key
        self.blockSignals(True)
        try:
            self._data = self._project[key] or []
            self._fe.events = (b[0] for b in self._data)
            self._fe_changed(max(0, min(len(self._data),
                                        self._fe.currentRow())))
        finally:
            self.blockSignals(False)

    def sync(self):
        self.set_key(self._key)

    def _fe_changed(self, index):
        try:
            conditions = self._data[index][1]
            actions = self._data[index][2]
        except IndexError:
            conditions = []
            actions = []
        self._bindspec.set_spec(conditions, actions)

    def _fe_name_changed(self, index, name):
        name = str(name)
        if index >= len(self._data):
            assert index == len(self._data)
            self._data.extend([[name, [], []]])
        else:
            self._data[index][0] = name
        self._flatten()

    def _bind_changed(self):
        self._flatten()

    def _flatten(self):
        m = []
        for binding in self._data:
            if binding[0] not in ttypes.triggers or binding[0] == 'wj':
                continue
            c = []
            for constraint in binding[1]:
                if constraint[0] not in ttypes.triggers or \
                        constraint[0] == 'wj' or not constraint[1]:
                    continue
                c.extend([[constraint[0], constraint[1]]])
            a = []
            for action in binding[2]:
                if action[0] not in ttypes.triggers or action[0] == 'wj' or \
                        not action[1]:
                    continue
                a.extend([[action[0], action[1]]])
            m.extend([[binding[0], c, a]])
        self._project[self._key] = m


class FiringEvents(QtGui.QTableWidget):

    eventChanged = QtCore.pyqtSignal(int, name='eventChanged')
    nameChanged = QtCore.pyqtSignal(int, str, name='nameChanged')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 1, parent)
        self.setHorizontalHeaderLabels(['Event'])
        self.horizontalHeader().setResizeMode(0, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentCellChanged('
                                             'int, int, int, int)'),
                               self._cell_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._changed)

    @property
    def events(self):
        events = []
        for i in xrange(self.rowCount()):
            events.extend([str(self.item(i, 0))])
        return events

    @events.setter
    def events(self, events):
        self.blockSignals(True)
        try:
            index = 0
            for event in events:
                if index >= self.rowCount():
                    self._append_row()
                self.setItem(index, 0, QtGui.QTableWidgetItem(event))
                index += 1
            if index >= self.rowCount():
                self._append_row()
            self.setItem(index, 0, QtGui.QTableWidgetItem())
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)

    def _append_row(self):
        self.insertRow(self.rowCount())

    def _cell_changed(self, row, col, prow, pcol):
        if row != prow:
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('eventChanged(int)'),
                                row)

    def _changed(self, row, col):
        text = self.item(row, col).text()
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('nameChanged(int, QString*)'),
                            row, text)
        if row == self.rowCount() - 1 and text:
            self._append_row()


class BindSpec(QtGui.QWidget):

    bindChanged = QtCore.pyqtSignal(name='bindChanged')

    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(5)
        self._conds = BindConditions()
        self._actions = BindActions()
        layout.addWidget(self._conds)
        layout.addWidget(self._actions)
        QtCore.QObject.connect(self._conds,
                               QtCore.SIGNAL('changed()'),
                               self._modified)
        QtCore.QObject.connect(self._actions,
                               QtCore.SIGNAL('changed()'),
                               self._modified)

    def set_spec(self, conditions, actions):
        self.blockSignals(True)
        try:
            self._conds.set_conditions(conditions)
            self._actions.set_actions(actions)
        finally:
            self.blockSignals(False)

    def _modified(self):
        QtCore.QObject.emit(self, QtCore.SIGNAL('bindChanged()'))


class BindConditions(QtGui.QTableWidget):

    changed = QtCore.pyqtSignal(name='changed')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setHorizontalHeaderLabels(['Event', 'Condition'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._changed)

    def set_conditions(self, conditions):
        self.blockSignals(True)
        try:
            index = 0
            for cond in conditions:
                if index >= self.rowCount():
                    self._append_row()
                self.setItem(index, 0, QtGui.QTableWidgetItem(cond[0]))
                self.setItem(index, 1, QtGui.QTableWidgetItem(cond[1]))
                index += 1
            if index >= self.rowCount():
                self._append_row()
            self.setItem(index, 0, QtGui.QTableWidgetItem())
            self.setItem(index, 1, QtGui.QTableWidgetItem())
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)
        self._conditions = conditions

    def _append_row(self):
        self.insertRow(self.rowCount())

    def _changed(self, row, col):
        text = str(self.item(row, col).text())
        if row >= len(self._conditions):
            assert row == len(self._conditions)
            cond = ['', '']
            cond[col] = text
            self._conditions.extend([cond])
        else:
            self._conditions[row][col] = text
        QtCore.QObject.emit(self, QtCore.SIGNAL('changed()'))
        if row == self.rowCount() - 1 and text:
            self._append_row()


class BindActions(QtGui.QTableWidget):

    changed = QtCore.pyqtSignal(name='changed')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setHorizontalHeaderLabels(['Event', 'Argument'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._changed)

    def set_actions(self, actions):
        self.blockSignals(True)
        try:
            index = 0
            for action in actions:
                if index >= self.rowCount():
                    self._append_row()
                self.setItem(index, 0, QtGui.QTableWidgetItem(action[0]))
                self.setItem(index, 1, QtGui.QTableWidgetItem(str(action[1])))
                index += 1
            if index >= self.rowCount():
                self._append_row()
            self.setItem(index, 0, QtGui.QTableWidgetItem())
            self.setItem(index, 1, QtGui.QTableWidgetItem())
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)
        self._actions = actions

    def _append_row(self):
        self.insertRow(self.rowCount())

    def _changed(self, row, col):
        text = str(self.item(row, col).text())
        if row >= len(self._actions):
            assert row == len(self._actions)
            action = ['', '']
            action[col] = text
            self._actions.extend([action])
        else:
            self._actions[row][col] = text
        QtCore.QObject.emit(self, QtCore.SIGNAL('changed()'))
        if row == self.rowCount() - 1 and text:
            self._append_row()


