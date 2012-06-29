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

from __future__ import division
from __future__ import print_function
#from Queue import Queue

from PyQt4 import QtCore, QtGui


class PeakMeter(QtGui.QWidget):

    SILENCE = -384

    def __init__(self, lowest, highest, mix_rate=48000, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'low': QtGui.QColor(0x11, 0xbb, 0x11),
                'mid': QtGui.QColor(0xdd, 0xcc, 0x33),
                'high': QtGui.QColor(0xee, 0x22, 0x11),
                'clip': QtGui.QColor(0xff, 0x33, 0x22),
                }
        self.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding,
                           QtGui.QSizePolicy.Fixed)
        self._mix_rate = mix_rate
        self._thickness = 4
        self._padding = 2
        self._clip_width = 20
        self._lowest = lowest
        self._highest = highest
        self._level = [float('-inf')] * 2
        self._clipped = [False] * 2
        self._hold_limit = mix_rate
        self._holds = [[PeakMeter.SILENCE, 0], [PeakMeter.SILENCE, 0]]
        self._nframes = 0

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self._colours['bg'])
        paint.eraseRect(ev.rect())

        dim_colours = {}
        for level in ('low', 'mid', 'high', 'clip'):
            bg = self._colours['bg']
            amount = 0.45
            r, g, b = bg.red(), bg.green(), bg.blue()
            r += (self._colours[level].red() - r) * amount
            g += (self._colours[level].green() - g) * amount
            b += (self._colours[level].blue() - b) * amount
            dim_colours[level] = QtGui.QColor(r, g, b)

        width = self.width() - self._clip_width
        grad = QtGui.QLinearGradient(0, 0, width, 0)
        mid_point = (-6 - self._lowest) / (self._highest - self._lowest)
        grad.setColorAt(0, dim_colours['low'])
        grad.setColorAt(mid_point, dim_colours['mid'])
        grad.setColorAt(1, dim_colours['high'])

        left_start = 0
        right_start = self._thickness + self._padding
        for y in (left_start, right_start):
            paint.fillRect(0, y, width, self._thickness, grad)

        for ch, level in enumerate(self._level):
            y_offset = ch * (self._thickness + self._padding)
            grad.setColorAt(0, self._colours['low'])
            grad.setColorAt(mid_point, self._colours['mid'])
            grad.setColorAt(1, self._colours['high'])
            if level > self._lowest:
                level = min(0, level)
                filled = ((level - self._lowest) /
                          (self._highest - self._lowest))
                paint.fillRect(0, y_offset,
                               width * filled, self._thickness, grad)
            hold = self._holds[ch]
            if hold[0] <= level or self._nframes - hold[1] >= self._hold_limit:
                hold[0] = level
                hold[1] = self._nframes
            peak_pos = ((hold[0] - self._lowest) /
                        (self._highest - self._lowest)) * width
            if peak_pos > 0:
                peak_width = self._thickness * 2
                peak_pos -= peak_width
                peak_width += min(0, peak_pos)
                peak_pos = max(0, peak_pos)
                paint.fillRect(peak_pos, y_offset, peak_width,
                               self._thickness, grad)

        for ch in (0, 1):
            if self._clipped[ch]:
                clip_colour = self._colours['clip']
            else:
                clip_colour = dim_colours['clip']
            paint.fillRect(width, ch * (self._thickness + self._padding),
                           self.width() - width, self._thickness, clip_colour)

        paint.end()

    def resizeEvent(self, ev):
        self.update()

    def sizeHint(self):
        return QtCore.QSize(self._clip_width * 5,
                            self._thickness * 2 + self._padding)

    def reset(self):
        self._clipped = [False] * 2
        self._holds = [[PeakMeter.SILENCE, 0], [PeakMeter.SILENCE, 0]]
        self._nframes = 0

    def set_peaks(self, dB_l, dB_r, abs_l, abs_r, nframes):
        if abs_l > 1:
            self._clipped[0] = True
        if abs_r > 1:
            self._clipped[1] = True
        if not nframes:
            self._holds = [[PeakMeter.SILENCE, 0], [PeakMeter.SILENCE, 0]]
        self._level[0] = dB_l
        self._level[1] = dB_r
        self._nframes += nframes
        self.update()


