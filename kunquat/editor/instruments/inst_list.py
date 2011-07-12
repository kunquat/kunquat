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

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim


class InstList(QtGui.QTableWidget):

    def __init__(self, project, instrument_spin, parent=None):
        QtGui.QTableWidget.__init__(self, lim.INSTRUMENTS_MAX, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)

        self._project = project
        self._instrument_spin = instrument_spin
        self.setVerticalHeaderLabels([str(num) for num in
                                      xrange(lim.INSTRUMENTS_MAX)])
        self.setCornerButtonEnabled(False)
        self.setTabKeyNavigation(False)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()

        QtCore.QObject.connect(instrument_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.inst_changed)
        QtCore.QObject.connect(self,
                    QtCore.SIGNAL('currentCellChanged(int, int, int, int)'),
                               self.cell_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self.name_changed)
        self._signal = False

    def inst_changed(self, num):
        index = self.model().index(num, 0)
        select = self.selectionModel()
        select.clear()
        select.select(index, QtGui.QItemSelectionModel.Select)
        select.setCurrentIndex(index, QtGui.QItemSelectionModel.Select)

    def cell_changed(self, cur_row, cur_col, prev_row, prev_col):
        if self._signal:
            return
        self._signal = True
        self._instrument_spin.setValue(cur_row)
        self._signal = False

    def name_changed(self, num, col):
        assert num >= 0
        assert num < lim.INSTRUMENTS_MAX
        if self._signal:
            return
        item = self.item(num, 0)
        key = 'ins_{0:02x}/kqti{1}/m_name.json'.format(num,
                                                       lim.FORMAT_VERSION)
        if item:
            self._project[key] = str(item.text())
        else:
            self._project[key] = None

    def sync(self):
        self._signal = True
        for i in xrange(lim.INSTRUMENTS_MAX):
            name = self._project['ins_{0:02x}/kqti{1}/m_name.json'.format(i,
                                                        lim.FORMAT_VERSION)]
            if name:
                item = self.item(i, 0)
                if not item:
                    item = QtGui.QTableWidgetItem(name)
                    self.setItem(i, 0, item)
                else:
                    item.setText(name)
            else:
                item = self.item(i, 0)
                if item:
                    item.setText('')
        self._signal = False


