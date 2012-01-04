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
import kunquat.editor.trigtypes as ttypes
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
        layout.addWidget(self._source, 0)
        self._targets = Targets()
        layout.addWidget(self._targets, 1)
        self.set_key('p_set_map.json')

    def set_key(self, key):
        self.blockSignals(True)
        self._key = key
        try:
            data = self._project[key]
            sources = (m[:2] for m in data)
            self._source.set_sources(sources)
        finally:
            self.blockSignals(False)

    def sync(self):
        self.set_key(self._key)

    def _source_changed(self, num):
        targets = []
        source_type = None
        if num >= 0:
            try:
                data = self._project[self._key]
                targets = data[num][2]
                source_type = data[num][0]
            except IndexError:
                targets = []
        self._targets.set_targets(source_type, targets)


class SetSource(QtGui.QTableWidget):

    sourceChanged = QtCore.pyqtSignal(int, name='sourceChanged')

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setHorizontalHeaderLabels(['Type', 'Name'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentCellChanged('
                                             'int, int, int, int)'),
                               self._cell_changed)

    def set_sources(self, sources):
        self.blockSignals(True)
        try:
            index = 0
            for var_type, var_name in sources:
                if index >= self.rowCount():
                    self.insertRow(index)
                    self.setCellWidget(index, 0, TypeSelect(index))
                self._set_row(index, var_type, var_name)
                index += 1
            if index >= self.rowCount():
                self.insertRow(index)
                self.setCellWidget(index, 0, TypeSelect(index))
            self._set_row(index, None, '')
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)

    def _cell_changed(self, row, col, prow, pcol):
        if row != prow:
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('sourceChanged(int)'),
                                row)

    def _set_row(self, index, var_type, var_name):
        select = self.cellWidget(index, 0)
        #QtCore.QObject.disconnect(
        select.index = index
        select.type_format = var_type
        self.setItem(index, 1, QtGui.QTableWidgetItem(var_name))
        #QtCore.QObject.connect(


class Targets(QtGui.QTableWidget):

    ranges = { None: None,
               'bool': BoolRange,
               bool: BoolRange,
               'float': FloatRange,
               float: FloatRange,
               'int': IntRange,
               int: IntRange,
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

    def set_targets(self, source_type, targets):
        self.blockSignals(True)
        try:
            index = 0
            for sr, ch, event, tr in targets:
                if index >= self.rowCount():
                    self.insertRow(index)
                    if Targets.ranges[source_type]:
                        self.setCellWidget(index, 0,
                                Targets.ranges[source_type](index, False))
                    self.setCellWidget(index, 1, ChSelect(index))
                self._set_row(index, sr, ch, event, tr)
                index += 1
            if index >= self.rowCount():
                self.insertRow(index)
                if Targets.ranges[source_type]:
                    self.setCellWidget(index, 0,
                            Targets.ranges[source_type](index, False))
                self.setCellWidget(index, 1, ChSelect(index))
            self._set_row(index, [], -1, '', '')
            for i in xrange(index + 1, self.rowCount()):
                self.removeRow(self.rowCount() - 1)
        finally:
            self.blockSignals(False)

    def _set_row(self, index, source_range, ch, event, target_range):
        if isinstance(self.cellWidget(index, 0), BoolRange) or \
                isinstance(self.cellWidget(index, 0), FloatRange):
            self.cellWidget(index, 0).range = source_range
        else:
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
            r = Targets.ranges[target_type](index)
            r.range = target_range
            self.setCellWidget(index, 3, r)
        else:
            self.setItem(index, 3, QtGui.QTableWidgetItem(str(target_range)))


