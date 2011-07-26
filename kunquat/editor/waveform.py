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
import math

from PyQt4 import QtCore, QtGui


class Waveform(QtGui.QWidget):

    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._colours = {
                            'bg': QtGui.QColor(0, 0, 0),
                            'center': QtGui.QColor(0x44, 0x44, 0x44),
                            'wave': QtGui.QColor(0x44, 0xaa, 0x22),
                        }
        self._data = [0, 0]
        #self._data = [math.sin(x * 2 * math.pi / 100) for x in xrange(1000)]
        self.setAutoFillBackground(False)
        self.setAttribute(QtCore.Qt.WA_OpaquePaintEvent)
        self.setAttribute(QtCore.Qt.WA_NoSystemBackground)
        self.fast = False

    def set_data(self, data):
        self._data = data if data else [0]
        if len(self._data) == 1:
            self._data.extend([self._data[0]])
        self.update()

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self._colours['bg'])
        paint.eraseRect(ev.rect())
        self._paint_center(paint)
        self._paint_wave(paint)
        paint.end()

    def _paint_center(self, paint):
        paint.setPen(self._colours['center'])
        y = (self.height() - 1) / 2
        start = QtCore.QPointF(0, y)
        end = QtCore.QPointF(self.width() - 1, y)
        paint.drawLine(start, end)

    def _paint_wave(self, paint):
        paint.setPen(self._colours['wave'])
        if len(self._data) <= self.width() or self.fast:
            self._paint_wave_in(paint)
        else:
            self._paint_wave_out(paint)

    def _paint_wave_in(self, paint):
        max_x = self.width() - 1
        max_sample = len(self._data) - 1
        max_y = self.height() - 1
        center = max_y / 2
        line = QtGui.QPolygonF()
        if len(self._data) > self.width() * 2:
            step = int(len(self._data) // self.width())
            view = self._data[::step]
        else:
            step = 1
            view = self._data
        for x, sample in enumerate(view):
            x *= step
            x = max_x * x / max_sample
            y = max_y * (-sample / 2) + center
            line.append(QtCore.QPointF(x, y))
        paint.drawPolyline(line)

    def _paint_wave_out(self, paint):
        max_x = self.width()
        max_y = self.height() - 1
        center = max_y / 2
        offsets = (int((x / max_x) * len(self._data))
                   for x in xrange(1, self.width() + 1))
        first = 0
        last_min_y = center
        last_max_y = center
        for x in xrange(self.width()):
            last = offsets.next()
            area = self._data[first:last]
            assert area
            min_sample, max_sample = min(area), max(area)
            min_sample_y = max_y * (-min_sample / 2) + center
            max_sample_y = max_y * (-max_sample / 2) + center
            if min_sample_y < last_max_y - 1:
                min_sample_y = last_max_y - 1
            elif max_sample_y > last_min_y + 1:
                max_sample_y = last_min_y + 1
            start = QtCore.QPointF(x, min_sample_y)
            end = QtCore.QPointF(x, max_sample_y)
            paint.drawLine(start, end)
            first = last
            last_min_y, last_max_y = min_sample_y, max_sample_y


