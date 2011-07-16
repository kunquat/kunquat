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


class ParamSlider(QtGui.QWidget):

    def __init__(self,
                 project,
                 label,
                 val_range,
                 default_val,
                 key,
                 dict_key=None,
                 decimals=0,
                 unit='',
                 orientation=QtCore.Qt.Horizontal,
                 parent=None):
        assert orientation in (QtCore.Qt.Horizontal, QtCore.Qt.Vertical)
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._dict_key = dict_key
        self._suffix = ' ' + unit if unit else ''

        if orientation == QtCore.Qt.Horizontal:
            layout = QtGui.QHBoxLayout(self)
        else:
            layout = QtGui.QVBoxLayout(self)
        lab = QtGui.QLabel(label)
        layout.addWidget(lab, 0)

        self._slider = KSlider(orientation)

        self._value_display = QtGui.QLabel()
        self.set_constraints({
                                'range': val_range,
                                'default': default_val,
                                'decimals': decimals
                             })
        layout.addWidget(self._slider, 1)
        layout.addWidget(self._value_display)
        QtCore.QObject.connect(self._slider,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.value_changed)
        QtCore.QObject.connect(self._slider,
                               QtCore.SIGNAL('editingFinished()'),
                               self.finished)
        self.set_key(key)

    def set_constraints(self, constraints):
        self._lock_update = True
        try:
            decimals = int(constraints['decimals'])
            if decimals < 0:
                decimals = 0
        except (KeyError, TypeError, ValueError):
            decimals = 0
        self._factor = 10**decimals
        try:
            r = constraints['range']
            val_range = float(r[0]), float(r[1])
            if val_range[0] > val_range[1]:
                raise ValueError
        except (IndexError, KeyError, TypeError, ValueError):
            val_range = -100, 100
        try:
            self._default_val = float(constraints['default']) if decimals \
                                else int(constraints['default'])
        except (KeyError, TypeError, ValueError):
            self._default_val = val_range[0]
        self._slider.setRange(val_range[0] * self._factor,
                              val_range[1] * self._factor)
        min_str = '{0:.{1}f}'.format(val_range[0], decimals) + self._suffix
        max_str = '{0:.{1}f}'.format(val_range[1], decimals) + self._suffix
        metrics = QtGui.QFontMetrics(QtGui.QFont())
        width = max(metrics.width(min_str),
                    metrics.width(max_str))
        self._value_display.setFixedWidth(width)
        self._lock_update = False

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
        self._slider.setValue(int(round(value * self._factor)))
        self._lock_update = False
        self._value_display.setText(str(value) + self._suffix)
        self._value = value

    def sync(self):
        self.set_key(self._key)

    def value_changed(self, svalue):
        if not self._key or self._lock_update:
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
            self._project.set(self._key, d, immediate=False)
        else:
            self._project.set(self._key, value, immediate=False)
        self._value_display.setText(str(value) + self._suffix)

    def finished(self):
        self._value = self._slider.value() / self._factor
        self._project.flush(self._key)


class KSlider(QtGui.QSlider):

    finished = QtCore.pyqtSignal(name='editingFinished')

    def __init__(self, orientation, parent=None):
        QtGui.QSlider.__init__(self, orientation, parent)

    def keyReleaseEvent(self, ev):
        QtGui.QSlider.keyReleaseEvent(self, ev)
        if ev.isAutoRepeat():
            return
        #QtCore.QObject.emit(self, QtCore.SIGNAL('sliderReleased()'))

    def focusOutEvent(self, ev):
        QtGui.QSlider.focusOutEvent(self, ev)
        QtCore.QObject.emit(self, QtCore.SIGNAL('editingFinished()'))


