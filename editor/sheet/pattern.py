#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import Qt, QtGui, QtCore

from column import Column
import kqt_limits as lim


class Pattern(QtGui.QWidget):

    def __init__(self, handle, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setSizePolicy(QtGui.QSizePolicy.Ignored,
                           QtGui.QSizePolicy.Ignored)
        self.handle = handle
        self.first_column = 0
        self.columns = [Column(num, None) for num in xrange(-1,
                                                            lim.COLUMNS_MAX)]
        self.view_columns = []
        self.colours = {
                "background": QtGui.QColor(0, 0, 0)
                }

    def set_path(self, path):
        pass

#    def sizeHint(self):
#        return QtCore.QSize(100, 100)

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self.colours["background"])
        paint.eraseRect(ev.rect())
        col_pos = 0
        for column in self.view_columns:
            column.paint(ev, paint, col_pos)
            col_pos += column.width
        paint.end()

    def resizeEvent(self, ev):
        self.view_columns = list(self._get_viewable_columns(ev))
        for column in self.view_columns:
            column.resize(ev)

    def _get_viewable_columns(self, ev):
        used = 0
        for (width, column) in \
                ((c.width, c) for c in self.columns[self.first_column:]):
            used += width
            if used >= ev.size().width():
                break
            yield column


