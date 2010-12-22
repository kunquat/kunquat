# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
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


class ParamSlider(QtGui.QWidget):

    def __init__(self,
                 project,
                 label,
                 val_range,
                 default_val,
                 key,
                 dict_key=None,
                 decimals=0,
                 orientation=QtCore.Qt.Horizontal,
                 parent=None):
        assert orientation in (QtCore.Qt.Horizontal, QtCore.Qt.Vertical)
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._factor = 10**decimals
        self._dict_key = dict_key
        self._default_val = default_val

        if orientation == QtCore.Qt.Horizontal:
            layout = QtGui.QHBoxLayout(self)
        else:
            layout = QtGui.QVBoxLayout(self)
        lab = QtGui.QLabel(label)
        layout.addWidget(lab, 0)

        self._slider = QtGui.QSlider(orientation)
        self._slider.setRange(val_range[0] * self._factor,
                              val_range[1] * self._factor)
        layout.addWidget(self._slider, 1)
        QtCore.QObject.connect(self._slider,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.value_changed)

        self._value_display = QtGui.QLabel()
        metrics = QtGui.QFontMetrics(QtGui.QFont())
        min_str = '{0:.{1}f}'.format(val_range[0], decimals)
        max_str = '{0:.{1}f}'.format(val_range[1], decimals)
        width = max(metrics.width(min_str),
                    metrics.width(max_str))
        self._value_display.setFixedWidth(width)
        layout.addWidget(self._value_display)
        self.set_key(key)

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
        self._slider.setValue(int(round(value * self._factor)))
        self._lock_update = False
        self._value_display.setText(str(value))
        self._key = key

    def value_changed(self, svalue):
        if self._lock_update:
            return
        value = svalue / self._factor
        if self._factor == 1:
            assert value == int(value)
            value = int(value)
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            self._project[self._key] = d
        else:
            self._project[self._key] = value
        self._using_default = False
        self._value_display.setText(str(value))


