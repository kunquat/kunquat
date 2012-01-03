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


class BoolRange(QtGui.QWidget):

    rangeChanged = QtCore.pyqtSignal(int, bool, bool, name='rangeChanged')

    def __init__(self, index, allow_degen=True, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.index = index
        self._allow_degen = allow_degen
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._begin = QtGui.QCheckBox()
        self._end = QtGui.QCheckBox()
        layout.addWidget(self._begin)
        layout.addWidget(self._end)
        QtCore.QObject.connect(self._begin,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._begin_changed)
        QtCore.QObject.connect(self._end,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._end_changed)

    @property
    def range(self):
        return [self._begin.checkState() == QtCore.Qt.Checked,
                self._end.checkState() == QtCore.Qt.Checked]

    @range.setter
    def range(self, r):
        if not r:
            r = [False, True]
        assert self._allow_degen or r[0] != r[1]
        self._begin.blockSignals(True)
        self._end.blockSignals(True)
        try:
            self._begin.setCheckState(QtCore.Qt.Checked if r[0]
                                      else QtCore.Qt.Unchecked)
            self._end.setCheckState(QtCore.Qt.Checked if r[1]
                                    else QtCore.Qt.Unchecked)
        finally:
            self._begin.blockSignals(False)
            self._end.blockSignals(False)

    def _begin_changed(self, state):
        self._changed(self._begin, self._end, state)

    def _end_changed(self, state):
        self._changed(self._end, self._begin, state)

    def _changed(self, changed, other, state):
        if not self._allow_degen and \
                changed.checkState() == other.checkState():
            other.blockSignals(True)
            other.setCheckState(QtCore.Qt.Checked - state)
            other.blockSignals(False)
        r = self.range
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('rangeChanged(int, bool, bool)'),
                            self.index, r[0], r[1])


