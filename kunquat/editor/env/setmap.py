# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011-2012
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

from boolrange import BoolRange
from chselect import ChSelect
from floatrange import FloatRange
from intrange import IntRange
import kunquat.editor.kqt_limits as lim
import kunquat.editor.timestamp as ts
import kunquat.editor.trigtypes as ttypes
from tsrange import TsRange
from typeselect import TypeSelect


class SetMap(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._source = SetSource()
        QtCore.QObject.connect(self._source,
                               QtCore.SIGNAL('sourceChanged(int)'),
                               self._source_changed)
        QtCore.QObject.connect(self._source,
                               QtCore.SIGNAL('nameChanged(int, QString*)'),
                               self._source_name_changed)
        QtCore.QObject.connect(self._source,
                               QtCore.SIGNAL('typeChanged(int, QString*)'),
                               self._source_type_changed)
        layout.addWidget(self._source, 0)
        self._targets = Targets()
        QtCore.QObject.connect(self._targets,
                               QtCore.SIGNAL('targetChanged(bool)'),
                               self._target_changed)
        layout.addWidget(self._targets, 1)
        self.set_key('p_set_map.json')

    def set_key(self, key):
        self.blockSignals(True)
        self._key = key
        try:
            self._data = self._project[key]
            if self._data:
                sources = [m[:2] for m in self._data]
            else:
                self._data = []
                sources = []
            self._source.set_sources(sources)
            self._source_changed(max(0, min(len(sources),
                                            self._source.currentRow())))
        finally:
            self.blockSignals(False)

    def sync(self):
        self.set_key(self._key)

    def _flatten(self, immediate=True):
        m = []
        for i, binding in enumerate(self._data):
            if binding[0] and binding[1]:
                if i == self._targets.index:
                    b = [binding[0], binding[1], []]
                    for row in self._targets.data:
                        if row[2]:
                            b[2].extend([row])
                    m.extend([b])
                else:
                    m.extend([binding])
        print(immediate, m)
        self._project.set(self._key, m, immediate)

    def _source_changed(self, num):
        targets = []
        source_type = None
        if num >= 0:
            try:
                targets = self._data[num][2]
                source_type = self._data[num][0]
            except (IndexError, TypeError):
                targets = []
        self._targets.set_targets(num, source_type, targets)

    def _source_name_changed(self, index, name):
        assert index >= 0
        name = str(name)
        if index < len(self._data):
            self._data[index][1] = name
        else:
            self._data.extend([['', name, []]])
        self._flatten()

    def _source_type_changed(self, index, var_type):
        if var_type:
            var_type = str(var_type)
        if index < len(self._data):
            self._data[index][0] = var_type
            cons = { 'bool': bool, 'int': int, 'float': float }[var_type]
            for mapping in self._data[index][2]:
                mapping[0] = [cons(val) for val in mapping[0]]
        else:
            self._data.extend([[var_type, '', []]])
        self._flatten()
        if self._source.currentRow() == index:
            self._targets.set_targets(index, var_type, self._data[index][2])

    def _target_changed(self, immediate):
        self._flatten(immediate)


class SetSource(QtGui.QTableWidget):

    sourceChanged = QtCore.pyqtSignal(int, name='sourceChanged')
    nameChanged = QtCore.pyqtSignal(int, str, name='nameChanged')
    typeChanged = QtCore.pyqtSignal(int, str, name='typeChanged')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setHorizontalHeaderLabels(['Type', 'Name'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentCellChanged('
                                             'int, int, int, int)'),
                               self._cell_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._changed)

    def set_sources(self, sources):
        self.blockSignals(True)
        try:
            index = 0
            for var_type, var_name in sources:
                if index >= self.rowCount():
                    self._append_row()
                self._set_row(index, var_type, var_name)
                index += 1
            if index >= self.rowCount():
                self._append_row()
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)

    def _append_row(self):
        pos = self.rowCount()
        self.insertRow(pos)
        self.setCellWidget(pos, 0, TypeSelect(pos))
        self._set_row(pos, None, '')

    def _cell_changed(self, row, col, prow, pcol):
        if row != prow:
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('sourceChanged(int)'),
                                row)

    def _set_row(self, index, var_type, var_name):
        select = self.cellWidget(index, 0)
        QtCore.QObject.disconnect(select,
                                  QtCore.SIGNAL('typeChanged(int, QString*)'),
                                  self._type_changed)
        select.index = index
        select.type_format = var_type
        self.setItem(index, 1, QtGui.QTableWidgetItem(var_name))
        QtCore.QObject.connect(select,
                               QtCore.SIGNAL('typeChanged(int, QString*)'),
                               self._type_changed)

    def _changed(self, row, col):
        if col == 1:
            text = self.item(row, col).text()
            self._name_changed(row, text)
            if row == self.rowCount() - 1 and text and \
                    self.cellWidget(row, 0).type_format:
                self._append_row()

    def _name_changed(self, index, name):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('nameChanged(int, QString*)'),
                            index, name)

    def _type_changed(self, index, var_type):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('typeChanged(int, QString*)'),
                            index, var_type)
        if index == self.rowCount() - 1 and self.item(index, 1).text():
            self._append_row()


class Targets(QtGui.QTableWidget):

    changed = QtCore.pyqtSignal(bool, name='targetChanged')

    ranges = { None: None,
               'bool': BoolRange,
               bool: BoolRange,
               'float': FloatRange,
               float: FloatRange,
               'int': IntRange,
               int: IntRange,
               'timestamp': TsRange,
               ts.Timestamp: TsRange,
             }

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 4, parent)
        self.setHorizontalHeaderLabels(['Source range',
                                        'Channel',
                                        'Event',
                                        'Target range'])
        for i in (1, 2, 3):
            self.horizontalHeader().setResizeMode(i, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        self._data = []
        self._index = 0

    @property
    def data(self):
        return self._data

    @property
    def index(self):
        return self._index

    def set_targets(self, index, source_type, targets):
        self._index = index
        self._data = targets
        self.blockSignals(True)
        try:
            index = 0
            for sr, ch, event, tr in targets:
                if index >= self.rowCount():
                    self.insertRow(index)
                    self.setCellWidget(index, 1, ChSelect(index))
                self._set_row(index, source_type, sr, ch, event, tr)
                index += 1
            if index >= self.rowCount():
                self.insertRow(index)
                self.setCellWidget(index, 1, ChSelect(index))
            self._set_row(index, source_type, [], -1, '', '')
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)

    def _set_row(self, index,
                 source_type, source_range,
                 ch, event, target_range):
        cons = Targets.ranges[source_type]
        if cons:
            self.takeItem(index, 0)
            if not isinstance(self.cellWidget(index, 0), cons):
                sr = cons(index, False)
                QtCore.QObject.connect(sr,
                                       QtCore.SIGNAL('rangeChanged(int)'),
                                       self._source_range_changed)
                self.setCellWidget(index, 0, sr)
            self.cellWidget(index, 0).range = source_range
        else:
            self.removeCellWidget(index, 0)
            self.setItem(index, 0, QtGui.QTableWidgetItem(str(source_range)))
        self.cellWidget(index, 1).ch = ch
        self.setItem(index, 2, QtGui.QTableWidgetItem(event))
        target_type = None
        try:
            if event in ttypes.global_triggers and event != 'wj':
                target_type = ttypes.global_triggers[event][0][0]
            elif event in ttypes.channel_triggers:
                target_type = ttypes.channel_triggers[event][0][0]
            elif event in ttypes.general_triggers:
                target_type = ttypes.general_triggers[event][0][0]
        except IndexError:
            pass
        if Targets.ranges[target_type]:
            self.takeItem(index, 3)
            r = Targets.ranges[target_type](index)
            r.range = target_range
            QtCore.QObject.connect(r,
                                   QtCore.SIGNAL('rangeChanged(int)'),
                                   self._target_range_changed)
            self.setCellWidget(index, 3, r)
        else:
            self.removeCellWidget(index, 3)
            self.setItem(index, 3, QtGui.QTableWidgetItem(str(target_range)))

    def _source_range_changed(self, index):
        assert index >= 0
        immediate = isinstance(self.cellWidget(index, 0), BoolRange)
        if index < len(self._data):
            self._data[index][0] = self.cellWidget(index, 0).range
        else:
            tr_widget = self.cellWidget(index, 3)
            tr = tr_widget.range if tr_widget else []
            etext = str(self.item(index, 2).text())
            self._data.extend([[self.cellWidget(index, 0).range,
                                self.cellWidget(index, 1).ch,
                                etext,
                                tr]])
            if not etext or not tr_widget:
                immediate = False
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('targetChanged(bool)'),
                            immediate)

    def _target_range_changed(self, index):
        print('target', index, self.cellWidget(index, 3).range)


