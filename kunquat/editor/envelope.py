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

from PyQt4 import QtCore, QtGui

import kqt_limits as lim
import kunquat


class Envelope(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'axis': QtGui.QColor(0xaa, 0xaa, 0xaa),
                'curve': QtGui.QColor(0x66, 0x88, 0xaa),
                'node': QtGui.QColor(0xee, 0xcc, 0xaa),
                'node_cur': QtGui.QColor(0xcc, 0xff, 0xcc),
                'text': QtGui.QColor(0xaa, 0xaa, 0xaa),
                }
        self._fonts = {
                'axis': QtGui.QFont('Decorative', 8),
                }
        self._layout = {
                'padding': 8,
                'zoom': (200, 200),
                }
        self._min = (0, 0)
        self._max = (1, 1)
        self._first_locked = (True, False)
        self._last_locked = (False, False)
        self._step = (0.001, 0.001)
        self._nodes_max = 16
        self._marks = []
        self._nodes = [(0, 0), (0.4, 0.8), (1, 1)]
        self._smooth = False

        self._haxis = HAxis(self._colours, self._fonts,
                            self._min, self._max, self._layout)
        self._vaxis = VAxis(self._colours, self._fonts,
                            self._min, self._max, self._layout)
        self._update_transform()

    def keyPressEvent(self, ev):
        pass

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self._colours['bg'])
        paint.eraseRect(ev.rect())
        self._haxis.paint(paint)
        self._vaxis.paint(paint)
        if not self._smooth:
            self._paint_linear_curve(paint)
        self._paint_nodes(paint)
        paint.end()

    def _paint_linear_curve(self, paint):
        paint.setPen(self._colours['curve'])
        for p1, p2 in zip(self._nodes[:-1], self._nodes[1:]):
            paint.drawLine(self._trans_x(p1[0]), self._trans_y(p1[1]),
                           self._trans_x(p2[0]), self._trans_y(p2[1]))

    def _paint_nodes(self, paint):
        paint.setPen(QtCore.Qt.NoPen)
        paint.setBrush(self._colours['node'])
        rect_size = 5
        for p in self._nodes:
            rect_x = self._trans_x(p[0]) - rect_size // 2
            rect_y = self._trans_y(p[1]) - rect_size // 2
            paint.fillRect(rect_x, rect_y, rect_size, rect_size,
                           paint.brush())

    def _update_transform(self):
        self._trans_x = lambda x: self._vaxis.width + x * \
                                  self._layout['zoom'][0]
        self._trans_y = lambda y: self._layout['padding'] + \
                                  self._layout['zoom'][1] * (self._max[1] - y)

    def resizeEvent(self, ev):
        pass


class Axis(object):

    def __init__(self, colours, fonts, min_point, max_point, layout):
        self._colours = colours
        self._fonts = fonts
        self._min = min_point
        self._max = max_point
        self._layout = layout
        self._mark_len = 5
        space = QtGui.QFontMetrics(
                self._fonts['axis']).boundingRect('00.000')
        self._x_zero_offset = space.width() + self._layout['padding'] + \
                self._mark_len
        self._hspace = space.height() + self._layout['padding'] + \
                self._mark_len


class HAxis(Axis):

    def __init__(self, colours, fonts, min_point, max_point, layout):
        super(HAxis, self).__init__(colours, fonts,
                                    min_point, max_point, layout)

    def paint(self, paint):
        paint.setPen(self._colours['axis'])
        y_pos = self._layout['padding'] + \
                self._max[1] * self._layout['zoom'][1]
        paint.drawLine(self._x_zero_offset, y_pos,
                       self._x_zero_offset + self._max[0] *
                               self._layout['zoom'][0], y_pos)

    @property
    def height(self):
        return self._hspace


class VAxis(Axis):

    def __init__(self, colours, fonts, min_point, max_point, layout):
        super(VAxis, self).__init__(colours, fonts,
                                    min_point, max_point, layout)

    def paint(self, paint):
        paint.setPen(self._colours['axis'])
        paint.drawLine(self._x_zero_offset, self._layout['padding'],
                       self._x_zero_offset,
                       self._layout['padding'] + self._max[1] *
                               self._layout['zoom'][1])

    @property
    def width(self):
        return self._x_zero_offset


