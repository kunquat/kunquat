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
import re

from PyQt4 import QtCore, QtGui

import kqt_limits as lim


class Connections(QtGui.QWidget):

    def __init__(self, project, key, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._key = key

        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        self._list = CList(project, key)
        layout.addWidget(self._list)

        self.sync()

    def sync(self):
        self._list.sync()


class CList(QtGui.QTableWidget):

    def __init__(self, project, key, parent=None):
        QtGui.QTableWidget.__init__(self, 2, 2, parent)
        self._project = project
        self._key = key
        self.setHorizontalHeaderLabels(['Source', 'Destination'])
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()

    def sync(self):
        conns = self._project[self._key]
        if not conns:
            for col, row in ((a, b) for a in xrange(self.columnCount())
                                    for b in xrange(self.rowCount())):
                item = QtGui.QTableWidgetItem()
                self.setItem(row, col, item)
            return
        new_rows = len(conns) + 1 - self.rowCount()
        for _ in xrange(new_rows):
            self.insertRow(self.rowCount())
        for i, conn in enumerate(conns):
            src, dest = conn
            self.setItem(i, 0, QtGui.QTableWidgetItem(src))
            self.setItem(i, 1, QtGui.QTableWidgetItem(dest))


