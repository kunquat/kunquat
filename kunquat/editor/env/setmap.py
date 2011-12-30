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

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim
from typeselect import TypeSelect


class SetMap(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._source = SetSource()
        layout.addWidget(self._source)
        self._targets = Targets()
        layout.addWidget(self._targets)
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


class SetSource(QtGui.QTableWidget):

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 2, parent)
        self.setHorizontalHeaderLabels(['Type', 'Name'])
        self.horizontalHeader().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()

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

    def _set_row(self, index, var_type, var_name):
        select = self.cellWidget(index, 0)
        #QtCore.QObject.disconnect(
        select.index = index
        select.type_format = var_type
        self.setItem(index, 1, QtGui.QTableWidgetItem(var_name))
        #QtCore.QObject.connect(


class Targets(QtGui.QTableWidget):

    def __init__(self, parent=None):
        QtGui.QTableWidget.__init__(self, 0, 4, parent)
        self.setHorizontalHeaderLabels(['Source range',
                                        'Channel',
                                        'Event',
                                        'Target range'])
        for i in (1, 2, 3):
            self.horizontalHeader().setResizeMode(i, QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()


