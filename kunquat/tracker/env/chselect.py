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

import kunquat.tracker.kqt_limits as lim


class ChSelect(QtGui.QSpinBox):

    chChanged = QtCore.pyqtSignal(int, name='chChanged')

    def __init__(self, index, parent=None):
        QtGui.QSpinBox.__init__(self, parent)
        self._index = index
        self.setMinimum(-lim.COLUMNS_MAX + 1)
        self.setMaximum(lim.COLUMNS_MAX - 1)
        self.setValue(0)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self._changed)

    @property
    def index(self):
        return self._index

    @index.setter
    def index(self, value):
        self._index = value

    @property
    def ch(self):
        return self.value()

    @ch.setter
    def ch(self, value):
        self.blockSignals(True)
        self.setValue(value)
        self.blockSignals(False)

    """
    def allow(self, glob, ch):
        assert glob or ch
        self.blockSignals(True)
        self.setMinimum(-1)
        self.setMaximum(lim.COLUMNS_MAX - 1)
        if not glob:
            self.setMinimum(0)
        if not ch:
            self.setMaximum(-1)
        self.blockSignals(False)
    """

    def _changed(self, value):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('chChanged(int)'),
                            self.index)


