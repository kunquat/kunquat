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

        self._list_area = CListArea(project, key)
        layout.addWidget(self._list_area)

        self.sync()

    def sync(self):
        self._list_area.sync()


class CListArea(QtGui.QScrollArea):

    def __init__(self, project, key, parent=None):
        QtGui.QScrollArea.__init__(self, parent)
        self._project = project
        self._list = CList(project, key)
        self.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.setWidget(self._list)
        self.setWidgetResizable(True)

    def sync(self):
        self._list.sync()


class CList(QtGui.QWidget):

    def __init__(self, project, key, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding,
                           QtGui.QSizePolicy.MinimumExpanding)
        self._project = project
        self._key = key
        self._layout = QtGui.QVBoxLayout(self)
        self._layout.setSizeConstraint(QtGui.QLayout.SetMinimumSize)
        self._layout.addWidget(Connection(key), 1)

    def sync(self):
        conns = self._project[self._key]
        if not conns:
            for i in xrange(len(self._layout)):
                item = self._layout.itemAt(i)
                item.widget().edge = None
            return
        add_count = len(conns) + 1 - len(self._layout)
        if add_count > 0:
            for _ in xrange(add_count):
                self._layout.addWidget(Connection(self._key), 1)
        for i, conn in enumerate(conns):
            item = self._layout.itemAt(i)
            item.widget().edge = conn


class Connection(QtGui.QWidget):

    def __init__(self, key, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding,
                           QtGui.QSizePolicy.MinimumExpanding)
        layout = QtGui.QHBoxLayout(self)
        layout.setSizeConstraint(QtGui.QLayout.SetMinimumSize)
        self._src = QtGui.QLineEdit()
        self._src.setValidator(ConnValidator(key))
        layout.addWidget(self._src, 1)
        self._dest = QtGui.QLineEdit()
        self._dest.setValidator(ConnValidator(key))
        layout.addWidget(self._dest, 1)

    @property
    def edge(self):
        src = self._src.text()
        dest = self._dest.text()
        if not (src and dest):
            return None
        return [src, dest]

    @edge.setter
    def edge(self, pair):
        if not pair:
            self._src.setText('')
            self._dest.setText('')
            return
        src, dest = pair
        self._src.setText(src)
        self._dest.setText(dest)


class ConnValidator(QtGui.QValidator):

    def __init__(self, key):
        super(ConnValidator, self).__init__()
        self._key = key

    def validate(self, field, pos):
        return QtGui.QValidator.Acceptable, pos


