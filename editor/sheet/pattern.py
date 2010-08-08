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
import timestamp as ts


class Pattern(QtGui.QWidget):

    def __init__(self, handle, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setSizePolicy(QtGui.QSizePolicy.Ignored,
                           QtGui.QSizePolicy.Ignored)
        self.handle = handle
        self.first_column = -1
        self.colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'column_border': QtGui.QColor(0xcc, 0xcc, 0xcc),
                'column_head_bg': QtGui.QColor(0x33, 0x77, 0x22),
                'column_head_text': QtGui.QColor(0xff, 0xee, 0xee),
                }
        self.fonts = {
                'column_head': QtGui.QFont('Decorative', 10),
                'ruler': QtGui.QFont('Decorative', 8),
                'trigger': QtGui.QFont('Decorative', 10),
                }
        self.beat_len = 96
        self.ruler = Ruler((16, 0), (self.colours, self.fonts))
        self.columns = [Column(num, None, (self.colours, self.fonts))
                        for num in xrange(-1, lim.COLUMNS_MAX)]
        self.view_columns = []

    def set_path(self, path):
        pass

#    def sizeHint(self):
#        return QtCore.QSize(100, 100)

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self.colours['bg'])
        paint.eraseRect(ev.rect())
        self.ruler.paint(ev, paint)
        col_pos = self.ruler.width()
        for column in self.view_columns:
            column.paint(ev, paint, col_pos)
            col_pos += column.width()
        paint.end()

    def resizeEvent(self, ev):
        self.view_columns = list(self.get_viewable_columns(ev))
        self.ruler.resize(ev)
        for column in self.view_columns:
            column.resize(ev)

    def get_viewable_columns(self, ev):
        used = self.ruler.width()
        for (width, column) in \
                ((c.width(), c) for c in self.columns[self.first_column + 1:]):
            used += width
            if used > ev.size().width():
                break
            yield column


class Ruler(object):

    def __init__(self, length, theme):
        self.length = length
        self.colours = theme[0]
        self.fonts = theme[1]
        self.height = 0
        self.start = ts.Timestamp()
        self.set_dimensions()

    def paint(self, ev, paint):
        ruler_area = QtCore.QRect(0, 0, self._width, self.height)
        real_area = ev.rect().intersect(ruler_area)
        if real_area.isEmpty() or (real_area.height() <=
                                   self.col_head_height):
            return
        paint.setPen(self.colours['column_border'])
        paint.drawLine(self._width - 1, 0,
                       self._width - 1, self.height - 1)

    def resize(self, ev):
        self.height = ev.size().height()

    def set_dimensions(self):
        self._width = QtGui.QFontMetrics(
                          self.fonts['trigger']).boundingRect(
                                                     '00.000').width() + 6
        self.col_head_height = QtGui.QFontMetrics(
                                   self.fonts['column_head']).height()

    def set_length(self, length):
        self.length = length

    def set_start(self, start):
        self.start = start

    def width(self):
        return self._width


