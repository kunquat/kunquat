# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division
from __future__ import print_function

from PyQt4 import QtGui, QtCore

import kunquat.editor.timestamp as ts


class TimestampSpin(QtGui.QWidget):

    ts_changed = QtCore.pyqtSignal(int, int, name='tsChanged')

    def __init__(self,
                 project,
                 label,
                 val_range,
                 default_val,
                 key,
                 dict_key=None,
                 decimals=0,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._dict_key = dict_key
        self._decimals = decimals
        self._default_val = list(default_val)

        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        lab = QtGui.QLabel(label)
        layout.addWidget(lab, 0)

        self._spin = QtGui.QDoubleSpinBox()
        self._spin.setMinimum(float(val_range[0]))
        self._spin.setMaximum(float(val_range[1]))
        self._spin.setDecimals(decimals)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('valueChanged(double)'),
                               self.value_changed)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('editingFinished()'),
                               self.finished)
        self.set_key(key)
        layout.addWidget(self._spin, 0)

    def set_key(self, key):
        value = self._default_val
        if self._dict_key:
            d = self._project[key]
            if d and self._dict_key in d:
                value = self._project[key][self._dict_key]
        else:
            actual = self._project[key]
            if actual != None:
                value = actual
        self._lock_update = True
        self._spin.setValue(float(ts.Timestamp(value)))
        self._lock_update = False
        self._key = key
        self._value = value

    def value_changed(self, fvalue):
        if self._lock_update:
            return
        value = list(ts.Timestamp(fvalue))
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            self._project.set(self._key, d)
        else:
            self._project.set(self._key, value)
        QtCore.QObject.emit(self, QtCore.SIGNAL('tsChanged(int, int)'),
                                                value[0], value[1])

    def finished(self):
        value = list(ts.Timestamp(self._spin.value()))
        if value == self._value:
            return
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            old_d = dict(d)
            old_d[self._dict_key] = self._value
            self._project.set(self._key, d, old_d)
        else:
            self._project.set(self._key, value, self._value)
        self._value = value

    def sync(self):
        self.set_key(self._key)


