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

from PyQt4 import QtCore, QtGui


class FloatRange(QtGui.QWidget):

    rangeChanged = QtCore.pyqtSignal(int, float, float, name='rangeChanged')

    def __init__(self, index, allow_degen=True, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.index = index
        self._allow_degen = allow_degen
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._begin = QtGui.QDoubleSpinBox()
        self._begin.setDecimals(3)
        self._begin.setMinimum(float('-inf'))
        self._begin.setMaximum(float('inf'))
        self._end = QtGui.QDoubleSpinBox()
        self._end.setDecimals(3)
        self._end.setMinimum(float('-inf'))
        self._end.setMaximum(float('inf'))
        layout.addWidget(self._begin)
        layout.addWidget(self._end)
        QtCore.QObject.connect(self._begin,
                               QtCore.SIGNAL('valueChanged(double)'),
                               self._begin_changed)
        QtCore.QObject.connect(self._end,
                               QtCore.SIGNAL('valueChanged(double)'),
                               self._end_changed)
        self._degen_fix = None

    @property
    def range(self):
        return [self._begin.value(), self._end.value()]

    @range.setter
    def range(self, r):
        if not r:
            r = [0, 1]
        assert self._allow_degen or r[0] != r[1]
        self._begin.blockSignals(True)
        self._end.blockSignals(True)
        try:
            self._begin.setValue(r[0])
            self._end.setValue(r[1])
        finally:
            self._begin.blockSignals(False)
            self._end.blockSignals(False)

    def _fix_val(self, value):
        return value - 1 if value > 0 else value + 1

    def _unfix_val(self, value):
        return value + 1 if value >= 0 else value - 1

    def _begin_changed(self, value):
        self._changed(value, self._begin, self._end)

    def _end_changed(self, value):
        self._changed(value, self._end, self._begin)

    def _changed(self, value, changed, other):
        if not self._allow_degen:
            if changed.value() == other.value():
                fval = other.value()
                if self._degen_fix == other:
                    fval = self._unfix_val(fval)
                    self._degen_fix = None
                else:
                    fval = self._fix_val(fval)
                    self._degen_fix = other
                other.blockSignals(True)
                other.setValue(fval)
                other.blockSignals(False)
            elif self._degen_fix == other and \
                    self._unfix_val(other.value()) != changed.value():
                other.blockSignals(True)
                other.setValue(self._unfix_val(self._end.value()))
                other.blockSignals(False)
                self._degen_fix = None
            if self._degen_fix == changed:
                self._degen_fix = None
        r = self.range
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('rangeChanged(int, double, double)'),
                            self.index, r[0], r[1])


