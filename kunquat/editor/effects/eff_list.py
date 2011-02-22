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

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim


class EffList(QtGui.QTableWidget):

    eff_changed = QtCore.pyqtSignal(int, name='effectChanged')

    def __init__(self, project, base, parent=None):
        self._max = lim.INST_EFFECTS_MAX if base.startswith('ins') \
                                         else lim.EFFECTS_MAX
        QtGui.QTableWidget.__init__(self, self._max, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)

        self._project = project
        self._base = base
        self.setVerticalHeaderLabels([str(num) for num in
                                      xrange(self._max)])
        self.setCornerButtonEnabled(False)
        self.setTabKeyNavigation(False)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()

        QtCore.QObject.connect(self,
                    QtCore.SIGNAL('currentCellChanged(int, int, int, int)'),
                               self.cell_changed)

    def eff_changed(self, num):
        index = self.model().index(num, 0)
        select = self.selectionModel()
        select.clear()
        select.select(index, QtGui.QItemSelectionModel.Select)
        select.setCurrentIndex(index, QtGui.QItemSelectionModel.Select)

    def cell_changed(self, cur_row, cur_col, prev_row, prev_col):
        QtCore.QObject.emit(self, QtCore.SIGNAL('effectChanged(int)'),
                            cur_row)

    def sync(self):
        # TODO
        pass


