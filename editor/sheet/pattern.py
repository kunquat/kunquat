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

from __future__ import division
import math

from PyQt4 import Qt, QtGui, QtCore

from column import Column
from cursor import Cursor
import kqt_limits as lim
import timestamp as ts


class Pattern(QtGui.QWidget):

    def __init__(self, handle, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setSizePolicy(QtGui.QSizePolicy.Ignored,
                           QtGui.QSizePolicy.Ignored)
        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.handle = handle
        self.first_column = -1
        self.colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'column_border': QtGui.QColor(0xcc, 0xcc, 0xcc),
                'column_head_bg': QtGui.QColor(0x33, 0x77, 0x22),
                'column_head_fg': QtGui.QColor(0xff, 0xee, 0xee),
                'cursor_bg': QtGui.QColor(0xff, 0x66, 0x22, 0x77),
                'cursor_line': QtGui.QColor(0xff, 0xee, 0x88),
                'ruler_bg': QtGui.QColor(0x11, 0x22, 0x55),
                'ruler_fg': QtGui.QColor(0xaa, 0xcc, 0xff),
                'trigger_fg': QtGui.QColor(0xcc, 0xcc, 0xcc),
                }
        self.fonts = {
                'column_head': QtGui.QFont('Decorative', 10),
                'ruler': QtGui.QFont('Decorative', 8),
                'trigger': QtGui.QFont('Decorative', 10),
                }
        self.length = ts.Timestamp(8)
        self.beat_len = 96
        self.view_start = ts.Timestamp(0)
        self.ruler = Ruler((self.colours, self.fonts))
        self.ruler.set_length(self.length)
        self.ruler.set_beat_len(self.beat_len)
        self.ruler.set_view_start(self.view_start)
        mock_triggers = [
                    [[0, 0], ['Cn+', [300]]],
                    [[1, 0], ['Cn+', [0]]],
                    [[1, 0], ['C.f', [-6]]],
                ]
        self.columns = [Column(num, None, (self.colours, self.fonts))
                        for num in xrange(-1, lim.COLUMNS_MAX)]
        self.columns[0].set_triggers(mock_triggers)
        for col in self.columns:
            col.set_length(self.length)
            col.set_beat_len(self.beat_len)
            col.set_view_start(self.view_start)
        self.cursor = Cursor(self.length, self.beat_len)
        self.columns[0].set_cursor(self.cursor)
        self.cursor_col = -1
        self.view_columns = []
        self.width = 0
        self.height = 0
        self.cursor_center_area = 0.3
        self.zoom_factor = 1.5

    def set_path(self, path):
        pass

#    def sizeHint(self):
#        return QtCore.QSize(100, 100)

    def follow_cursor_horizontal(self):
        if not self.view_columns or self.cursor_col in self.view_columns:
            return False
        elif self.cursor_col < self.view_columns[0].get_num():
            self.first_column = self.cursor_col
            self.view_columns = list(self.get_viewable_columns(self.width))
            return True
        elif self.cursor_col > self.view_columns[-1].get_num():
            left_cols = list(self.get_viewable_columns(self.width,
                                                       self.cursor_col, -1))
            if left_cols:
                self.first_column = left_cols[-1].get_num()
                self.view_columns = list(self.get_viewable_columns(self.width))
            return True
        return False

    def follow_cursor_vertical(self):
        col_head_height = QtGui.QFontMetrics(
                              self.fonts['column_head']).height()
        trigger_height = QtGui.QFontMetrics(self.fonts['trigger']).height()
        pix_view_start = self.view_start * self.beat_len
        pix_pat_end = (self.length - self.view_start) * self.beat_len
        pix_pat_len = self.length * self.beat_len
        cur_view_pos = self.cursor.get_pix_pos() - pix_view_start
        area_height = self.height - col_head_height
        neg_pix_shift = cur_view_pos - area_height * (0.5 -
                                           (self.cursor_center_area / 2))
        neg_pix_shift += trigger_height / 2
        pos_pix_shift = cur_view_pos - area_height * (0.5 +
                                           (self.cursor_center_area / 2))
        pos_pix_shift += trigger_height / 2
        if neg_pix_shift < 0:
            pix_view_start = max(0, pix_view_start + neg_pix_shift)
        elif pos_pix_shift > 0:
            pix_view_start += pos_pix_shift
            if pix_pat_len - pix_view_start < area_height - trigger_height:
                pix_view_start = pix_pat_len - area_height + trigger_height
                pix_view_start = max(0, pix_view_start)
        if neg_pix_shift < 0 or pos_pix_shift > 0:
            self.view_start = ts.Timestamp(pix_view_start / self.beat_len)
            self.ruler.set_view_start(self.view_start)
            for col in self.columns:
                col.set_view_start(self.view_start)
            return True
        return False

    def keyPressEvent(self, ev):
        if ev.modifiers() == QtCore.Qt.ControlModifier:
            if ev.key() == QtCore.Qt.Key_Up:
                self.zoom(self.zoom_factor)
            elif ev.key() == QtCore.Qt.Key_Down:
                self.zoom(1 / self.zoom_factor)
            return

        if ev.key() == QtCore.Qt.Key_Left:
            if self.cursor_col > -1:
                self.columns[self.cursor_col + 1].set_cursor()
                self.columns[self.cursor_col].set_cursor(self.cursor)
                self.cursor.set_col(self.cursor_col - 1)
                self.cursor_col -= 1
                self.follow_cursor_horizontal()
                self.update()
            else:
                assert self.cursor_col == -1
        elif ev.key() == QtCore.Qt.Key_Right:
            if self.cursor_col < lim.COLUMNS_MAX - 1:
                self.columns[self.cursor_col + 1].set_cursor()
                self.columns[self.cursor_col + 2].set_cursor(self.cursor)
                self.cursor.set_col(self.cursor_col + 1)
                self.cursor_col += 1
                self.follow_cursor_horizontal()
                self.update()
            else:
                assert self.cursor_col == lim.COLUMNS_MAX - 1
        elif ev.key() in (QtCore.Qt.Key_Up, QtCore.Qt.Key_Down,
                          QtCore.Qt.Key_PageUp, QtCore.Qt.Key_PageDown,
                          QtCore.Qt.Key_Home, QtCore.Qt.Key_End):
            self.cursor.key_press(ev)
            self.follow_cursor_vertical()
            self.update()
#        else:
#            print('press:', ev.key())

    def keyReleaseEvent(self, ev):
        if ev.isAutoRepeat():
            return
        if ev.key() in (QtCore.Qt.Key_Up, QtCore.Qt.Key_Down):
            self.cursor.key_release(ev)
#        else:
#            print('release:', ev.key())

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
        self.width = ev.size().width()
        self.height = ev.size().height()
        self.view_columns = list(self.get_viewable_columns(self.width))
        self.ruler.resize(ev)
        for column in self.columns:
            column.resize(self.height)

    def get_viewable_columns(self, total_width, start=None, direction=1):
        if start == None:
            start = self.first_column
        assert direction in (1, -1)
        used = self.ruler.width()
        for (width, column) in ((c.width(), c) for c in \
                           self.columns[start + 1::direction]):
            used += width
            if used > total_width:
                break
            yield column

    def zoom(self, factor):
        pix_view_start = self.view_start * self.beat_len
        cur_view_pos = self.cursor.get_pix_pos() - pix_view_start
        self.beat_len = min(max(1, self.beat_len * factor),
                            lim.TIMESTAMP_BEAT * 3)
        self.cursor.set_beat_len(self.beat_len)
        self.view_start = ts.Timestamp((self.cursor.get_pix_pos() -
                                        cur_view_pos) / self.beat_len)
        if self.view_start < 0:
            self.view_start = ts.Timestamp()
        pix_view_start = self.view_start * self.beat_len
        pix_pat_len = self.length * self.beat_len
        col_head_height = QtGui.QFontMetrics(
                              self.fonts['column_head']).height()
        area_height = self.height - col_head_height
        trigger_height = QtGui.QFontMetrics(self.fonts['trigger']).height()
        if pix_pat_len - pix_view_start < area_height - trigger_height:
            self.follow_cursor_vertical()
        self.ruler.set_beat_len(self.beat_len)
        self.ruler.set_view_start(self.view_start)
        for col in self.columns:
            col.set_beat_len(self.beat_len)
            col.set_view_start(self.view_start)
        self.update()


class Ruler(object):

    def __init__(self, theme):
        self.colours = theme[0]
        self.fonts = theme[1]
        self.height = 0
        self.view_start = ts.Timestamp()
        self.beat_div_base = 2
        self.set_dimensions()

    def get_viewable_positions(self, interval):
        view_end = float(self.view_start) + (self.ruler_height +
                                             self.num_height) / self.beat_len
        view_end = min(view_end, float(self.length))
        error = interval / 2
        pos = math.ceil(float(self.view_start) / interval) * interval
        while pos < view_end:
            if abs(pos - round(pos)) < error:
                pos = round(pos)
            yield pos
            pos += interval

    def paint(self, ev, paint):
        ruler_area = QtCore.QRect(0, 0, self._width, self.height)
        real_area = ev.rect().intersect(ruler_area)
        if real_area.isEmpty() or self.ruler_height <= 0:
            return

        view_start = float(self.view_start)
        view_len = self.ruler_height / self.beat_len
        view_end = view_start + view_len

        # paint background, including start and end borders if visible
        bg_start = self.col_head_height
        bg_end = min(self.height, (float(self.length) - view_start) *
                                   self.beat_len + self.col_head_height)
        paint.setBrush(self.colours['ruler_bg'])
        paint.setPen(QtCore.Qt.NoPen)
        paint.drawRect(0, bg_start, self._width, bg_end - bg_start)
        paint.setPen(self.colours['ruler_fg'])
        if self.view_start == 0:
            paint.drawLine(0, self.col_head_height,
                           self._width - 2, self.col_head_height)
        if bg_end < self.height:
            paint.drawLine(0, bg_end, self._width - 2, bg_end)

        # resolve intervals
        line_min_time = self.line_min_dist / self.beat_len
        line_interval = self.beat_div_base**math.ceil(
                                math.log(line_min_time, self.beat_div_base))
        num_min_time = self.num_min_dist / self.beat_len
        num_interval = self.beat_div_base**math.ceil(
                               math.log(num_min_time, self.beat_div_base))

        # paint ruler lines
        for line_pos in set(self.get_viewable_positions(
                line_interval)).difference(
                        self.get_viewable_positions(num_interval)):
            self.paint_line(line_pos, paint)

        # paint ruler beat numbers
        paint.setFont(self.fonts['ruler'])
        for num_pos in self.get_viewable_positions(num_interval):
            self.paint_number(num_pos, paint)

        # paint right border
        paint.setPen(self.colours['column_border'])
        paint.drawLine(self._width - 1, 0,
                       self._width - 1, self.height - 1)

    def paint_line(self, pos, paint):
        view_start = float(self.view_start)
        view_len = self.ruler_height / self.beat_len
        view_end = view_start + view_len
        y = self.col_head_height + (pos - view_start) * self.beat_len
        if y < self.col_head_height or y >= self.height:
            return
        if pos == 0 or pos == float(self.length):
            return
        x = self._width - 3
        paint.drawLine(x, y, self._width - 1, y)

    def paint_number(self, pos, paint):
        view_start = float(self.view_start)
        view_len = self.ruler_height / self.beat_len
        view_end = view_start + view_len
        y = self.col_head_height + (pos - view_start) * self.beat_len
        if y < self.col_head_height:
            return
        if pos == 0 or pos == float(self.length):
            return
        x = self._width - 6
        if y < self.height:
            paint.drawLine(x, y, self._width - 1, y)
        y -= self.num_height // 2
        if y >= self.height:
            return
        rect = QtCore.QRectF(0, y, self._width - 8, self.num_height)
        text = str(round(pos, 3))
        text = str(int(pos)) if pos == int(pos) else str(round(pos, 3))
        paint.drawText(rect, text, QtGui.QTextOption(QtCore.Qt.AlignRight))

    def resize(self, ev):
        self.height = ev.size().height()
        self.set_dimensions()

    def set_beat_len(self, length):
        self.beat_len = float(length)

    def set_dimensions(self):
        space = QtGui.QFontMetrics(
                    self.fonts['ruler']).boundingRect('00.000')
        self._width = space.width() + 8
        self.num_height = space.height()
        self.num_min_dist = space.height() * 2.0
        self.line_min_dist = 4.0
        self.col_head_height = QtGui.QFontMetrics(
                                   self.fonts['column_head']).height()
        self.ruler_height = self.height - self.col_head_height

    def set_length(self, length):
        self.length = length

    def set_view_start(self, start):
        self.view_start = start

    def width(self):
        return self._width


