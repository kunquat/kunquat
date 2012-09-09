# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011-2012
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


class DSPList(QtGui.QTableWidget):

    dspChanged = QtCore.pyqtSignal(int, name='dspChanged')

    def __init__(self, project, base, parent=None):
        QtGui.QTableWidget.__init__(self, lim.DSPS_MAX, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self._project = project
        self._cur_dsp = 0
        self.setVerticalHeaderLabels([str(num) for num in
                                      xrange(lim.DSPS_MAX)])
        self.setCornerButtonEnabled(False)
        self.setTabKeyNavigation(False)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()
        self._base = base

    def init(self):
        self.set_base(self._base)
        QtCore.QObject.connect(self,
                QtCore.SIGNAL('currentCellChanged(int, int, int, int)'),
                self._cell_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._name_changed)

    def set_base(self, base):
        self._base = base
        self.blockSignals(True)
        name_base = '{0}dsp_{{0:02x}}/m_name.json'.format(self._base)
        for i in xrange(lim.DSPS_MAX):
            name_key = name_base.format(i)
            name = self._project[name_key]
            item = self.item(i, 0)
            if name:
                if not item:
                    item = QtGui.QTableWidgetItem(name)
                    self.setItem(i, 0, item)
                else:
                    item.setText(name)
            elif item:
                item.setText('')
        self.blockSignals(False)

    def sync(self):
        self.set_base(self._base)

    def _cell_changed(self, cur_row, cur_col, prev_row, prev_col):
        if cur_row == self._cur_dsp:
            return
        self._cur_dsp = cur_row
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('dspChanged(int)'),
                            self._cur_dsp)

    def _name_changed(self, num, col):
        assert num >= 0
        assert num < lim.DSPS_MAX
        item = self.item(num, 0)
        dsp_name = '{0}dsp_{1:02x}/m_name.json'.format(self._base,
                                                       self._cur_dsp)
        if item:
            self._project[dsp_name] = unicode(item.text())
        else:
            self._project[dsp_name] = None


