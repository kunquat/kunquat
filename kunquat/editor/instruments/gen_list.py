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


class GenList(QtGui.QTableWidget):

    def __init__(self, project, parent=None):
        QtGui.QTableWidget.__init__(self, lim.GENERATORS_MAX, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self._project = project
        self.setVerticalHeaderLabels([str(num) for num in
                                      xrange(lim.GENERATORS_MAX)])
        self.setCornerButtonEnabled(False)
        self.setTabKeyNavigation(False)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()

    def inst_changed(self, num):
        index = self.model().index(num, 0)
        select = self.selectionModel()
        select.clear()
        select.select(index, QtGui.QItemSelectionModel.Select)
        select.setCurrentIndex(index, QtGui.QItemSelectionModel.Select)

    def cell_changed(self, cur_row, cur_col, prev_row, prev_col):
        pass

    def name_changed(self, num, col):
        pass

    def sync(self):
        pass


