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

from chselect import ChSelect
import kunquat.tracker.trigtypes as ttypes
import kunquat.tracker.kqt_limits as lim


auto_events = {
        'Asubsong': [(int, lambda x: x >= -1 and x < lim.SUBSONGS_MAX, '0')],
        'Asection': [(int, lambda x: x >= -1 and x < lim.SECTIONS_MAX, '0')],
        'Apattern': [(int, lambda x: 0 <= x < lim.PATTERNS_MAX, '0')],
        'Arow': [ttypes.any_ts],
        'Avoices': [ttypes.any_int],
        'Af': [ttypes.any_float],
}


bind_events = dict(auto_events.items() + ttypes.triggers.items())
del bind_events['mj']


class Bind(QtGui.QWidget):

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
                               QtCore.SIGNAL('bindChanged(bool)'),
                               self._bind_changed)
        self.set_key('p_bind.json')

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
            assert index == len(self._data)
            self._data.extend([['', [], []]])
            conditions = self._data[index][1]
            actions = self._data[index][2]
        self._bindspec.set_spec(conditions, actions)

    def _fe_name_changed(self, index, name):
        name = str(name)
        if index >= len(self._data):
            assert index == len(self._data)
            self._data.extend([[name, [], []]])
        else:
            self._data[index][0] = name
        m = self._flatten() or None
        saved = self._project[self._key] or None
        if m != saved:
            self._project[self._key] = m

    def _bind_changed(self, immediate):
        m = self._flatten(immediate) or None
        saved = self._project[self._key] or None
        if m != saved:
            self._project.set(self._key, m, immediate)

    def _flatten(self, immediate=True):
        m = []
        for binding in self._data:
            if binding[0] not in bind_events:
                continue
            c = []
            for constraint in binding[1]:
                if constraint[0] not in bind_events or not constraint[1]:
                    continue
                c.extend([[constraint[0], constraint[1]]])
            a = []
            for action in binding[2]:
                if action[1][0] not in bind_events or not action[1][1]:
                    continue
                a.extend([[action[0], [action[1][0], action[1][1]]]])
            m.extend([[binding[0], c, a]])
        return m


class FiringEvents(QtGui.QTableWidget):

    eventChanged = QtCore.pyqtSignal(int, name='eventChanged')
    nameChanged = QtCore.pyqtSignal(int, str, name='nameChanged')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
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

    bindChanged = QtCore.pyqtSignal(bool, name='bindChanged')

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
                               QtCore.SIGNAL('changed(bool)'),
                               self._modified)
        QtCore.QObject.connect(self._actions,
                               QtCore.SIGNAL('changed(bool)'),
                               self._modified)

    def set_spec(self, conditions, actions):
        self.blockSignals(True)
        try:
            self._conds.conditions = conditions
            self._actions.actions = actions
        finally:
            self.blockSignals(False)

    @property
    def conditions(self):
        return self._conds.conditions

    @property
    def actions(self):
        return self._actions.actions

    def _modified(self, immediate):
        QtCore.QObject.emit(self, QtCore.SIGNAL('bindChanged(bool)'),
                            immediate)


class BindConditions(QtGui.QTableWidget):

    changed = QtCore.pyqtSignal(bool, name='changed')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.setHorizontalHeaderLabels(['Event', 'Condition'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._changed)

    @property
    def conditions(self):
        return self._conditions

    @conditions.setter
    def conditions(self, conditions):
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
        QtCore.QObject.emit(self, QtCore.SIGNAL('changed(bool)'), True)
        if row == self.rowCount() - 1 and text:
            self._append_row()


class BindActions(QtGui.QTableWidget):

    changed = QtCore.pyqtSignal(bool, name='changed')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 3, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.setHorizontalHeaderLabels(['Ch. offset', 'Event', 'Argument'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._event_changed)

    @property
    def actions(self):
        return self._actions

    @actions.setter
    def actions(self, actions):
        self.blockSignals(True)
        try:
            index = 0
            for action in actions:
                if index >= self.rowCount():
                    self._append_row()
                self.cellWidget(index, 0).ch = action[0]
                self.setItem(index, 1, QtGui.QTableWidgetItem(action[1][0]))
                self.setItem(index, 2, QtGui.QTableWidgetItem(str(action[1][1])))
                index += 1
            if index >= self.rowCount():
                self._append_row()
            self.cellWidget(index, 0).ch = 0
            self.setItem(index, 1, QtGui.QTableWidgetItem())
            self.setItem(index, 2, QtGui.QTableWidgetItem())
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)
        self._actions = actions

    def _append_row(self):
        self.insertRow(self.rowCount())
        index = self.rowCount() - 1
        self.setCellWidget(index, 0, ChSelect(index))
        QtCore.QObject.connect(self.cellWidget(index, 0),
                               QtCore.SIGNAL('chChanged(int)'),
                               self._ch_changed)

    def _ch_changed(self, index):
        if index >= len(self._actions):
            assert index == len(self._actions)
            action = [self.cellWidget(index, 0).ch, ['', '']]
            self._actions.extend([action])
        else:
            self._actions[index][0] = self.cellWidget(index, 0).ch
        QtCore.QObject.emit(self, QtCore.SIGNAL('changed(bool)'), False)

    def _event_changed(self, row, col):
        assert col > 0
        text = str(self.item(row, col).text())
        if row >= len(self._actions):
            assert row == len(self._actions)
            action = [0, ['', '']]
            action[1][col - 1] = text
            self._actions.extend([action])
        else:
            self._actions[row][1][col - 1] = text
        QtCore.QObject.emit(self, QtCore.SIGNAL('changed(bool)'), True)
        if row == self.rowCount() - 1 and text:
            self._append_row()


