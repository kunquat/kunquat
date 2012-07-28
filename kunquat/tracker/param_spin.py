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

from PyQt4 import QtGui, QtCore


class ParamSpin(QtGui.QWidget):

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
        self._default_val = default_val

        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        lab = QtGui.QLabel(label)
        layout.addWidget(lab)

        self._spin = QtGui.QDoubleSpinBox()
        self._spin.setMinimum(float(val_range[0]))
        self._spin.setMaximum(float(val_range[1]))
        self._spin.setDecimals(decimals)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('valueChanged(double)'),
                               self._value_changed)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('editingFinished()'),
                               self._finished)
        self.set_key(key)
        layout.addWidget(self._spin)

    def set_key(self, key):
        self._key = key
        if not key:
            return
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
        self._spin.setValue(value)
        self._lock_update = False

    def sync(self):
        self.set_key(self._key)

    def _value_changed(self, value):
        if self._lock_update:
            return
        if not self._decimals:
            value = int(value)
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            self._project.set(self._key, d, immediate=False)
        else:
            self._project.set(self._key, value, immediate=False)

    def _finished(self):
        self._project.flush(self._key)


