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

from PyQt4 import QtGui, QtCore

import kunquat.editor.kqt_limits as lim
from kunquat.editor.plane_view import PlaneView, HAxis, VAxis


RANDOMS_MAX = 8
SAMPLES_MAX = 512


class SampleMap(QtGui.QWidget):

    def __init__(self, project, key, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._key = key
        self._map = {}
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._map_view = MapView()
        random_layout = QtGui.QVBoxLayout()
        for _ in xrange(RANDOMS_MAX):
            random_layout.addWidget(Entry())
        layout.addWidget(self._map_view)
        layout.addLayout(random_layout)

    def set_key(self, key):
        self._key = key
        self._map = {}
        if not key:
            return
        map_list = self._project[key]
        for point, rand_list in map_list:
            self._map[tuple(point)] = rand_list
        self._map_view.set_data(self._map)

    def sync(self):
        self.set_key(self._key)


class Entry(QtGui.QWidget):

    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._freq = QtGui.QSpinBox()
        self._volume = QtGui.QDoubleSpinBox()
        self._volume.setDecimals(2)
        self._sample = QtGui.QSpinBox()
        self._sample.setMaximum(SAMPLES_MAX - 1)
        layout.addWidget(self._freq)
        layout.addWidget(self._volume)
        layout.addWidget(self._sample)

    def set_data(self, data):
        pass


class MapView(PlaneView):

    activeChanged = QtCore.pyqtSignal(float, float, name='activeChanged')

    def __init__(self, parent=None):
        PlaneView.__init__(self, parent)
        self._map = {}
        self._active = None
        self._focused = None
        self._aspect = None
        self._colours = {
                            'bg': QtGui.QColor(0, 0, 0),
                            'axis': QtGui.QColor(0xaa, 0xaa, 0xaa),
                            'node': QtGui.QColor(0xee, 0xcc, 0xaa),
                            'node_focus': QtGui.QColor(0xff, 0x77, 0x22),
                            'text': QtGui.QColor(0xaa, 0xaa, 0xaa),
                        }
        self._fonts = {
                          'axis': QtGui.QFont('Decorative', 8),
                      }
        self.setMouseTracking(True)
        self.setAutoFillBackground(False)
        self.setAttribute(QtCore.Qt.WA_OpaquePaintEvent)
        self.setAttribute(QtCore.Qt.WA_NoSystemBackground)
        self._min = -36, -6000
        self._max = 0, 6000
        self._layout = {
                           'padding': 8,
                           'zoom': (200, 200),
                           'offset': [0, 0],
                           'visible_min': [x for x in self._min],
                           'visible_max': [x for x in self._max],
                       }
        self._haxis = HAxis(self._colours, self._fonts,
                            self._min, self._max, self._layout)
        self._vaxis = VAxis(self._colours, self._fonts,
                            self._min, self._max, self._layout)
        self._set_view()

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setRenderHint(QtGui.QPainter.Antialiasing)
        paint.setBackground(self._colours['bg'])
        paint.eraseRect(ev.rect())
        self._haxis.paint(paint)
        self._vaxis.paint(paint)
        self._paint_nodes(paint)
        paint.end()

    def set_data(self, data):
        self._map = data
        self._active = None

    def mouseMoveEvent(self, ev):
        node = self._node_at(ev.x() - 0.5, ev.y() - 0.5)
        if node != self._focused:
            self._focused = node
            self.update()

    def mousePressEvent(self, ev):
        node = self._node_at(ev.x() - 0.5, ev.y() - 0.5)
        if node != self._active:
            self._active = node
            x, y = node if node else (float('-inf'), float('-inf'))
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('activeChanged(float, float)'),
                                y, x)
            self.update()

    def _node_at(self, x, y):
        closest = None
        closest_dist = float('inf')
        for ny, nx in self._map.iterkeys():
            dist = math.hypot(self._view_x(nx) - x,
                              self._view_y(ny) - y)
            if dist < closest_dist:
                closest = nx, ny
                closest_dist = dist
        if closest_dist < 5:
            return closest
        return None

    def _paint_nodes(self, paint):
        paint.setPen(QtCore.Qt.NoPen)
        paint.setBrush(self._colours['node'])
        diameter = 6
        for y, x in self._map.iterkeys():
            rect_x = self._view_x(x) - diameter / 2 + 0.5
            rect_y = self._view_y(y) - diameter / 2 + 0.5
            if (y, x) == self._active:
                colour = 'node_focus' if (y, x) == self._focused else 'node'
                paint.setPen(self._colours[colour])
                paint.setBrush(QtCore.Qt.NoBrush)
                sur_diameter = diameter + 4
                sur_rect_x = self._view_x(x) - sur_diameter / 2 + 0.5
                sur_rect_y = self._view_y(y) - sur_diameter / 2 + 0.5
                sur_rect = QtCore.QRectF(sur_rect_x, sur_rect_y,
                                         sur_diameter, sur_diameter)
                paint.drawEllipse(sur_rect)
                paint.setPen(QtCore.Qt.NoPen)
                paint.setBrush(self._colours[colour])
            elif (y, x) == self._focused:
                paint.setBrush(self._colours['node_focus'])
            rect = QtCore.QRectF(rect_x, rect_y, diameter, diameter)
            paint.drawEllipse(rect)
            if (y, x) == self._focused:
                paint.setBrush(self._colours['node'])

    def _set_view(self):
        self._layout['offset'][0] = (-self._layout['visible_min'][0] /
                                     (self._layout['visible_max'][0] -
                                      self._layout['visible_min'][0]))
        self._layout['offset'][1] = (-self._layout['visible_min'][1] /
                                     (self._layout['visible_max'][1] -
                                      self._layout['visible_min'][1]))
        self.resizeEvent(None)


