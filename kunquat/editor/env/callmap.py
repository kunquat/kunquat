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
        self.set_key('p_call_map.json')

    def set_key(self, key):
        self._key = key
        self.blockSignals(True)
        try:
            self._data = self._project[key] or []
            self._fe.set_events(b[0] for b in self._data)
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


class FiringEvents(QtGui.QTableWidget):

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 1, parent)
        self.setHorizontalHeaderLabels(['Event'])
        self.horizontalHeader().setResizeMode(0, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()

    def set_events(self, events):
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


class BindSpec(QtGui.QWidget):

    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(5)
        self._conds = BindConditions()
        self._actions = BindActions()
        layout.addWidget(self._conds)
        layout.addWidget(self._actions)

    def set_spec(self, conditions, actions):
        self.blockSignals(True)
        try:
            self._conds.set_conditions(conditions)
            self._actions.set_actions(actions)
        finally:
            self.blockSignals(False)


class BindConditions(QtGui.QTableWidget):

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setHorizontalHeaderLabels(['Event', 'Condition'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()

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

    def _append_row(self):
        self.insertRow(self.rowCount())


class BindActions(QtGui.QTableWidget):

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setHorizontalHeaderLabels(['Event', 'Argument'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()

    def set_actions(self, actions):
        self.blockSignals(True)
        try:
            index = 0
            for action in actions:
                if index >= self.rowCount():
                    self._append_row()
                self.setItem(index, 0, QtGui.QTableWidgetItem(action[0]))
                self.setItem(index, 1, QtGui.QTableWidgetItem(str(action[1][0])))
                index += 1
            if index >= self.rowCount():
                self._append_row()
            self.setItem(index, 0, QtGui.QTableWidgetItem())
            self.setItem(index, 1, QtGui.QTableWidgetItem())
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)

    def _append_row(self):
        self.insertRow(self.rowCount())


