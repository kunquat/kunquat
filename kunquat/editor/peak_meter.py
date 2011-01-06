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
from collections import deque
from Queue import LifoQueue

from PyQt4 import QtCore, QtGui


class PeakMeter(QtGui.QWidget):

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
        self._level = [LifoQueue(), LifoQueue()]
        self._prev_level = [float('-inf'), float('-inf')]
        self._update_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self._update_timer, QtCore.SIGNAL('timeout()'),
                               self.update)
        self._update_timer.setSingleShot(True)
        self._updating_ = False
        #self._update_timer.start(10)
        self._level_history = [deque(), deque()]
        self._clipped = [False, False]

    @property
    def _updating(self):
        return self._updating_

    @_updating.setter
    def _updating(self, value):
        if value:
            if not self._updating_:
                self._updating_ = True
                self._clipped = [False, False]
                self.update()
        else:
            if self._updating_:
                self._updating_ = False
                self._level = [LifoQueue(), LifoQueue()]
                self._level_history = [deque(), deque()]

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

        nframes = 0
        for ch in (0, 1):
            if self._level[ch].empty():
                level = self._prev_level[ch]
                self._updating = False
            else:
                level, nframes = self._level[ch].get()
                self._prev_level[ch] = level
            if nframes:
                history = self._level_history[ch]
                if len(history) >= 5:
                    history.popleft()
                history.append(level if level > -96 else -96)
                if len(history) >= 2:
                    level = (sum(history) - min(history)) / (len(history) - 1)
                else:
                    level = sum(history) / len(history)
            else:
                level = float('-inf')
            #if ch == 0:
            #    print(level, self._level_history[0])
            if level > self._lowest:
                grad.setColorAt(0, self._colours['low'])
                grad.setColorAt(mid_point, self._colours['mid'])
                grad.setColorAt(1, self._colours['high'])
                level = min(0, level)
                filled = ((level - self._lowest) /
                          (self._highest - self._lowest))
                paint.fillRect(0, ch * (self._thickness + self._padding),
                               width * filled, self._thickness, grad)
        wait = (nframes / self._mix_rate) * 950
        #print(self._prev_level)
        #if self._level[0].qsize() < 10:
        #    wait *= 1.5
        if self._level[0].qsize() > 20:
            self._level[0].get()
            self._level[1].get()
        if nframes:
            assert self._update_timer.isSingleShot()
            QtCore.QTimer.singleShot(wait, self, QtCore.SLOT('update()'))
            #self._update_timer.start((nframes / self._mix_rate) * 950)
        else:
            self._updating = False

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

    def set_peaks(self, dB_l, dB_r, abs_l, abs_r, nframes):
        #print(dB_l, dB_r)
        self._level[0].put((dB_l, nframes))
        self._level[1].put((dB_r, nframes))
        if abs_l > 1:
            self._clipped[0] = True
        if abs_r > 1:
            self._clipped[1] = True
        self._updating = True


