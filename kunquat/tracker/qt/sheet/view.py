# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *
from utils import *
import tstamp


class View(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self._lengths = []
        self._col_width = DEFAULT_CONFIG['col_width']
        self._first_col = 0
        self._px_offset = 0
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']

        self._visible_cols = 0

        self._caches = [ColumnCache(i) for i in xrange(COLUMN_COUNT)]

    def set_config(self, config):
        self._config = config

    def set_first_column(self, first_col):
        if self._first_col != first_col:
            self._first_col = first_col
            self.update()

    def _set_pattern_heights(self):
        self._heights = get_pat_heights(self._lengths, self._px_per_beat)
        self._start_heights = get_pat_start_heights(self._heights)

    def set_px_offset(self, offset):
        if self._px_offset != offset:
            self._px_offset = offset
            self._set_pattern_heights()
            self.update()

    def set_px_per_beat(self, px_per_beat):
        if self._px_per_beat != px_per_beat:
            self._px_per_beat = px_per_beat
            self._set_pattern_heights()
            self.update()

    def resizeEvent(self, ev):
        max_visible_cols = get_max_visible_cols(self.width(), self._col_width)
        first_col = clamp_start_col(self._first_col, max_visible_cols)
        visible_cols = get_visible_cols(first_col, max_visible_cols)

        update_rect = None

        if first_col != self._first_col:
            update_rect = QRect(0, 0, self.width(), self.height())
        elif visible_cols > self._visible_cols:
            x_offset = self._visible_cols * self._col_width
            width = self.width() - x_offset
            update_rect = QRect(x_offset, 0, width, self.height())

        self._first_col = first_col
        self._visible_cols = visible_cols

        if ev.size().height() > ev.oldSize().height():
            update_rect = QRect(0, 0, self.width(), self.height())

        if update_rect:
            self.update(update_rect)

    def paintEvent(self, ev):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['canvas_bg_colour'])

        rect = ev.rect()

        # See which columns should be drawn
        draw_col_start = rect.x() // self._col_width
        draw_col_stop = 1 + (rect.x() + rect.width() - 1) // self._col_width
        draw_col_stop = min(draw_col_stop, COLUMN_COUNT - self._first_col)

        # Draw columns
        buffer_create_count = 0
        for rel_col_index in xrange(draw_col_start, draw_col_stop):
            x_offset = rel_col_index * self._col_width
            tfm = QTransform().translate(x_offset, 0)
            painter.setTransform(tfm)
            buffer_create_count += self._draw_column(
                    painter,
                    self._caches[self._first_col + rel_col_index])

        painter.setTransform(QTransform())

        # Fill horizontal trailing blank
        hor_trail_start = draw_col_stop * self._col_width
        if hor_trail_start < self.width():
            width = self.width() - hor_trail_start
            painter.eraseRect(QRect(hor_trail_start, 0, width, self.height()))

        if buffer_create_count == 0:
            pass # TODO: update was easy, predraw a likely next buffer

        end = time.time()
        elapsed = end - start
        print('View updated in {:.2f} ms'.format(elapsed * 1000))

    def _draw_column(self, painter, cache):
        # Testing
        painter.eraseRect(0, 0, self._col_width, self.height())
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._col_width - 1, self.height() - 1)
        painter.drawText(QPoint(2, 12), str(cache._num))

        return 0


class ColumnCache():

    def __init__(self, num):
        self._num = num

    def set_column_width(self, width):
        self._width = width

    def set_column_offset(self, offset):
        self._col_offset = offset

    def set_height(self, height):
        self._height = height

    def paint(self, painter):
        painter.setBrushOrigin(QPoint(self._col_offset * self._width, 0))

        # Testing
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, self._width, self._height))
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._width - 1, self._height - 1)
        painter.drawText(QPoint(2, 10), str(self._num))


