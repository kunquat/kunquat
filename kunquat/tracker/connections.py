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
import kunquat


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
        QtCore.QObject.connect(self._list,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._changed)

        self._sync = False
        self.sync()

    def _changed(self, row, col):
        if self._sync:
            return
        item = self._list.item(row, col)
        other = self._list.item(row, 1 - col)
        if bool(item.text()) == bool(other and other.text()):
            conns = self._flatten()
            try:
                self._project[self._key] = conns
            except kunquat.KunquatFormatError as e:
                print(e)
            finally:
                self.sync()

    def _flatten(self):
        conns = []
        for row in xrange(self._list.rowCount()):
            src_item = self._list.item(row, 0)
            dest_item = self._list.item(row, 1)
            if src_item and dest_item:
                src = src_item.text()
                dest = dest_item.text()
                if src and dest:
                    conns.append([str(src), str(dest)])
        return conns

    def set_key(self, key):
        self._key = key
        self._sync = True
        try:
            self._list.set_key(key)
        finally:
            self._sync = False

    def sync(self):
        self.set_key(self._key)


class CList(QtGui.QTableWidget):

    def __init__(self, project, key, parent=None):
        QtGui.QTableWidget.__init__(self, 1, 2, parent)
        self._project = project
        self._key = key
        self.setHorizontalHeaderLabels(['Source', 'Destination'])
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.verticalHeader().hide()

    def set_key(self, key):
        self._key = key
        conns = self._project[self._key]
        if not conns:
            while self.rowCount() > 1:
                self.removeRow(0)
            if self.rowCount() == 0:
                self.insertRow(0)
            self.setItem(0, 0, QtGui.QTableWidgetItem())
            self.setItem(0, 1, QtGui.QTableWidgetItem())
            return
        new_rows = len(conns) + 1 - self.rowCount()
        for _ in xrange(new_rows):
            self.insertRow(self.rowCount())
        for _ in xrange(-new_rows):
            self.removeRow(self.rowCount() - 1)
        for i, conn in enumerate(conns):
            src, dest = conn
            self.setItem(i, 0, QtGui.QTableWidgetItem(src))
            self.setItem(i, 1, QtGui.QTableWidgetItem(dest))
        self.setItem(self.rowCount() - 1, 0, QtGui.QTableWidgetItem(''))
        self.setItem(self.rowCount() - 1, 1, QtGui.QTableWidgetItem(''))

    def sync(self):
        self.set_key(self._key)


