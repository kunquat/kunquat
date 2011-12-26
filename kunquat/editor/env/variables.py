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
from typeselect import TypeSelect


class Variables(QtGui.QTableWidget):

    tsre = re.compile('\(?([0-9]*) *, *([0-9]*)\)?$')

    def __init__(self,
                 project,
                 key,
                 parent=None):
        QtGui.QTableWidget.__init__(self, 0, 4, parent)
        self._project = project
        self._update_names = {
                    'bool': '',
                    'int': '',
                    'float': '',
                }
        self._project.set_callback('>.Bn', self._update_bool_name)
        self._project.set_callback('>.In', self._update_int_name)
        self._project.set_callback('>.Fn', self._update_float_name)
        self._project.set_callback('>.B', self._update_bool)
        self._project.set_callback('>.I', self._update_int)
        self._project.set_callback('>.F', self._update_float)
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
            cb_cur = BoolMod(self._project)
            QtCore.QObject.connect(cb_init,
                                   QtCore.SIGNAL('clicked()'),
                                   self._flatten)
            if var_init:
                cb_init.setCheckState(QtCore.Qt.Checked)
            cb_cur.name = var_name
            cb_cur.value = var_init
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
        name = self.item(index, 1).text()
        if value or name:
            self._set_row(index, str(value) if value else None, name)
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
        if col == 1:
            self._type_changed(row, self.cellWidget(row, 0).type_format)
            return
        if col < 3:
            self._flatten()
            return
        assert col == 3
        var_type = self.cellWidget(row, 0).type_format
        if not var_type:
            return
        name_item = self.item(row, 1)
        name = name_item.text() if name_item else ''
        if var_type == 'bool':
            pass
        elif var_type == 'int':
            value_item = self.item(row, 3)
            if not value_item:
                return
            try:
                value = int(value_item.text())
            except ValueError:
                return
            self._project.handle.fire(-1,
                    '[">.In", ["{}"]]'.format(name))
            self._project.handle.fire(-1,
                    '[">.I", [{:d}]]'.format(value))
        elif var_type == 'float':
            value_item = self.item(row, 3)
            if not value_item:
                return
            try:
                value = float(value_item.text())
            except ValueError:
                return
            self._project.handle.fire(-1,
                    '[">.Fn", ["{}"]]'.format(name))
            self._project.handle.fire(-1,
                    '[">.F", [{:.17f}]]'.format(value))

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
        self._project[self._key] = var_list

    def _update_bool_name(self, ch, event):
        self._update_names['bool'] = event[1][0]

    def _update_int_name(self, ch, event):
        self._update_names['int'] = event[1][0]

    def _update_float_name(self, ch, event):
        self._update_names['float'] = event[1][0]

    def _update_value(self, ch, event, type_desc):
        for i in xrange(self.rowCount()):
            if self.cellWidget(i, 0).type_format == type_desc and \
                    self.item(i, 1).text() == self._update_names[type_desc]:
                if type_desc == 'bool':
                    self.cellWidget(i, 3).value = (QtCore.Qt.Checked if
                                        event[1][0] else QtCore.Qt.Unchecked)
                elif type_desc in ('int', 'float'):
                    self.blockSignals(True)
                    self.item(i, 3).setText(str(event[1][0]))
                    self.blockSignals(False)


    def _update_bool(self, ch, event):
        self._update_value(ch, event, 'bool')

    def _update_int(self, ch, event):
        self._update_value(ch, event, 'int')

    def _update_float(self, ch, event):
        self._update_value(ch, event, 'float')


class BoolMod(QtGui.QCheckBox):

    def __init__(self, project, parent=None):
        QtGui.QCheckBox.__init__(self, parent)
        self._project = project
        self.name = ''
        self.setCheckState(QtCore.Qt.Unchecked)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._changed)

    @property
    def value(self):
        return self.checkState() == QtCore.Qt.Checked

    @value.setter
    def value(self, value):
        self.blockSignals(True)
        self.setCheckState(QtCore.Qt.Checked if value
                           else QtCore.Qt.Unchecked)
        self.blockSignals(False)

    def _changed(self, value):
        if not self.name:
            return
        self._project.handle.fire(-1,
                '[">.Bn", ["{}"]]'.format(self.name))
        self._project.handle.fire(-1,
                '[">.B", [{}]]'.format('true' if self.isChecked()
                                              else 'false'))


