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
                'node_focus': QtGui.QColor(0xff, 0x77, 0x22),
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
        self._focus_node = None
        self._focus_index = -1
        self.setMouseTracking(True)

    def keyPressEvent(self, ev):
        pass

    def mouseMoveEvent(self, ev):
        focus_node, index = self._node_at(ev.x() - 0.5, ev.y() - 0.5)
        if focus_node != self._focus_node:
            self._focus_node = focus_node
            self._focus_index = index
            self.update()

    def mousePressEvent(self, ev):
        pass

    def mouseReleaseEvent(self, ev):
        pass

    def _node_at(self, x, y):
        closest = self._nodes[0]
        closest_dist = math.hypot(self._view_x(closest[0]) - x,
                                  self._view_y(closest[1]) - y)
        closest_index = 0
        for i, node in enumerate(self._nodes):
            node_dist = math.hypot(self._view_x(node[0]) - x,
                                   self._view_y(node[1]) - y)
            if node_dist < closest_dist:
                closest = node
                closest_dist = node_dist
                closest_index = i
        if closest_dist < 4:
            return closest, i
        return None, -1

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setRenderHint(QtGui.QPainter.Antialiasing)
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
            start = QtCore.QPointF(self._view_x(p1[0]) + 0.5,
                                   self._view_y(p1[1]) + 0.5)
            end = QtCore.QPointF(self._view_x(p2[0]) + 0.5,
                                 self._view_y(p2[1]) + 0.5)
            paint.drawLine(start, end)

    def _paint_nodes(self, paint):
        paint.setPen(QtCore.Qt.NoPen)
        paint.setBrush(self._colours['node'])
        rect_size = 5
        for p in self._nodes:
            if p == self._focus_node:
                paint.setBrush(self._colours['node_focus'])
            rect_x = self._view_x(p[0]) - rect_size / 2 + 0.5
            rect_y = self._view_y(p[1]) - rect_size / 2 + 0.5
            rect = QtCore.QRectF(rect_x, rect_y, rect_size, rect_size)
            paint.fillRect(rect, paint.brush())
            if p == self._focus_node:
                paint.setBrush(self._colours['node'])

    def _val_x(self, x):
        return (x - self._vaxis.width) / self._layout['zoom'][0]

    def _val_y(self, y):
        return self._max[1] - ((y - self._layout['padding']) /
                               self._layout['zoom'][1])

    def _view_x(self, x):
        return self._vaxis.width + x * self._layout['zoom'][0]

    def _view_y(self, y):
        return self._layout['padding'] + \
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
                self._max[1] * self._layout['zoom'][1] + 0.5
        start = QtCore.QPointF(self._x_zero_offset + 0.5, y_pos)
        end = QtCore.QPointF(self._x_zero_offset + self._max[0] *
                             self._layout['zoom'][0] + 0.5, y_pos)
        paint.drawLine(start, end)

    @property
    def height(self):
        return self._hspace


class VAxis(Axis):

    def __init__(self, colours, fonts, min_point, max_point, layout):
        super(VAxis, self).__init__(colours, fonts,
                                    min_point, max_point, layout)

    def paint(self, paint):
        paint.setPen(self._colours['axis'])
        start = QtCore.QPointF(self._x_zero_offset + 0.5,
                               self._layout['padding'] + 0.5)
        end = QtCore.QPointF(self._x_zero_offset + 0.5,
                             self._layout['padding'] + self._max[1] *
                                     self._layout['zoom'][1] + 0.5)
        paint.drawLine(start, end)

    @property
    def width(self):
        return self._x_zero_offset


