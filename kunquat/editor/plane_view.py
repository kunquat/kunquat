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

from PyQt4 import QtGui, QtCore


class PlaneView(QtGui.QWidget):

    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)

    def resizeEvent(self, ev):
        x_space = self.width() - self._vaxis.width * 2 # - self._layout['padding']
        y_space = self.height() - self._haxis.height * 2 # - self._layout['padding']
        self._layout['zoom'] = (x_space / (self._layout['visible_max'][0] -
                                           self._layout['visible_min'][0]),
                                y_space / (self._layout['visible_max'][1] -
                                           self._layout['visible_min'][1]))
        if self._aspect == 1:
            min_zoom = min(self._layout['zoom'])
            self._layout['zoom'] = (min_zoom, min_zoom)

    def _val_x(self, x):
        dist = (x - self._vaxis.width) / self._layout['zoom'][0]
        return dist + self._layout['visible_min'][0]

    def _val_y(self, y):
        #return self._max[1] - ((y - self._layout['padding']) /
        return self._max[1] - ((y - self._haxis.height) /
                               self._layout['zoom'][1])

    def _view_x(self, x):
        return self._vaxis.width + ((x - self._layout['visible_min'][0]) *
                                    self._layout['zoom'][0])

    def _view_y(self, y):
        #return self._layout['padding'] + \
        return self._haxis.height + \
               self._layout['zoom'][1] * (self._max[1] - y)


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
        #y_pos = self._layout['padding'] + \
        y_pos = self.height + \
                self._layout['visible_max'][1] * self._layout['zoom'][1] + 0.5
        start = QtCore.QPointF(self._x_zero_offset + 0.5 +
                               (self._min[0] - self._layout['visible_min'][0]) *
                               self._layout['zoom'][0], y_pos)
        end = QtCore.QPointF(self._x_zero_offset +
                             (self._max[0] - self._layout['visible_min'][0]) *
                             self._layout['zoom'][0] + 0.5, y_pos)
        if start.x() < 0:
            start.setX(0)
        if end.x() == float('inf'):
            end.setX(50000)
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
        x_pos = self._x_zero_offset + 0.5 - \
                self._layout['visible_min'][0] * self._layout['zoom'][0]
        start = QtCore.QPointF(x_pos, self._hspace + 0.5 +
                               (self._layout['visible_max'][1] - self._min[1]) *
                               self._layout['zoom'][1])
        end = QtCore.QPointF(x_pos, self._hspace +
                             (self._layout['visible_max'][1] - self._max[1]) *
                                     self._layout['zoom'][1] + 0.5)
        paint.drawLine(start, end)

    @property
    def width(self):
        return self._x_zero_offset


