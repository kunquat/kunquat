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
from itertools import islice, izip_longest
import math

from PyQt4 import QtGui, QtCore

import kunquat.editor.kqt_limits as lim
from kunquat.editor.plane_view import PlaneView, HAxis, VAxis


RANDOMS_MAX = 8
SAMPLES_MAX = 512


class SampleMap(QtGui.QWidget):

    def __init__(self, project, key, map_type, parent=None):
        assert map_type in ('pitch', 'hit')
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._key = key
        self._map = {}
        self._map_type = map_type
        self._active = float('-inf'), float('-inf')
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._map_view = MapView(self._map, map_type)
        random_layout = QtGui.QVBoxLayout()
        self._entries = []
        for i in xrange(RANDOMS_MAX):
            entry = Entry(i)
            self._entries.extend([entry])
            random_layout.addWidget(entry)
            QtCore.QObject.connect(entry,
                                   QtCore.SIGNAL('modified(int, float, '
                                                 'float, int)'),
                                   self._entry_modified)
            QtCore.QObject.connect(entry,
                                   QtCore.SIGNAL('finished(int)'),
                                   self._entry_finished)
            QtCore.QObject.connect(entry,
                                   QtCore.SIGNAL('removed(int)'),
                                   self._remove_entry)
        self._add_entry = QtGui.QPushButton('+')
        self._add_entry.hide()
        random_layout.addWidget(self._add_entry)
        layout.addWidget(self._map_view, 1)
        layout.addLayout(random_layout, 1)
        QtCore.QObject.connect(self._map_view,
                               QtCore.SIGNAL('activeChanged(float, float)'),
                               self._active_changed)
        QtCore.QObject.connect(self._map_view,
                               QtCore.SIGNAL('modified()'),
                               self._map_modified)
        QtCore.QObject.connect(self._map_view,
                               QtCore.SIGNAL('removed()'),
                               self._map_index_removed)
        QtCore.QObject.connect(self._map_view,
                               QtCore.SIGNAL('finished()'),
                               self._map_finished)
        QtCore.QObject.connect(self._add_entry,
                               QtCore.SIGNAL('clicked()'),
                               self._new_entry)

    def set_key(self, key):
        self._key = key
        self._map = {}
        if not key:
            return
        map_list = self._project[key]
        if map_list:
            for point, rand_list in map_list:
                self._map[tuple(point)] = rand_list
        self._map_view.set_data(self._map)

    def sync(self):
        self.set_key(self._key)

    def _entry_modified(self, index, cents, volume, sample):
        rand_list = self._map[self._active]
        rand_list[index] = cents, volume, sample
        self._project.set(self._key, self._map.items(), immediate=False)

    def _entry_finished(self, index):
        self._project.flush(self._key)

    def _map_modified(self):
        self._project.set(self._key, self._map.items(), immediate=False)

    def _map_index_removed(self):
        self._active_changed(float('-inf'), float('-inf'))
        self._map_modified()

    def _map_finished(self):
        self._project.flush(self._key)

    def _active_changed(self, pitch, volume):
        self._active = pitch, volume
        if (pitch, volume) not in self._map:
            for entry in self._entries:
                entry.set_data(None)
            self._add_entry.hide()
            return
        rand_list = self._map[(pitch, volume)]
        for entry, data in izip_longest(self._entries, rand_list):
            entry.set_data(data)
        if len(rand_list) < RANDOMS_MAX:
            self._add_entry.show()
        else:
            self._add_entry.hide()

    def _new_entry(self):
        assert self._active in self._map
        rand_list = self._map[self._active]
        assert len(rand_list) < RANDOMS_MAX
        values = 0, 0, 0
        self._entries[len(rand_list)].set_data(values)
        rand_list.extend([values])
        self._project[self._key] = self._map.items()
        if len(rand_list) == RANDOMS_MAX:
            self._add_entry.hide()

    def _remove_entry(self, index):
        assert self._active in self._map
        rand_list = self._map[self._active]
        assert index < len(rand_list)
        rand_list[index:index + 1] = []
        for entry, data in izip_longest(islice(self._entries, index, None),
                                        islice(rand_list, index, None)):
            entry.set_data(data)
        self._project[self._key] = self._map.items()
        self._add_entry.show()


class Entry(QtGui.QWidget):

    removed = QtCore.pyqtSignal(int, name='removed')
    modified = QtCore.pyqtSignal(int, float, float, int, name='modified')
    finished = QtCore.pyqtSignal(int, name='finished')

    def __init__(self, ident, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._id = ident
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._cents = QtGui.QDoubleSpinBox()
        self._cents.setMinimum(-6000)
        self._cents.setMaximum(6000)
        self._cents.setDecimals(0)
        self._volume = QtGui.QDoubleSpinBox()
        self._volume.setDecimals(2)
        self._volume.setMinimum(-48)
        self._volume.setMaximum(48)
        self._sample = QtGui.QSpinBox()
        self._sample.setMinimum(0)
        self._sample.setMaximum(SAMPLES_MAX - 1)
        self._remove = QtGui.QPushButton('Remove')
        layout.addWidget(self._cents, 1)
        layout.addWidget(self._volume, 1)
        layout.addWidget(self._sample, 1)
        layout.addWidget(self._remove, 0)
        for widget in (self._cents, self._volume):
            QtCore.QObject.connect(widget,
                                   QtCore.SIGNAL('valueChanged(double)'),
                                   self._modified)
        QtCore.QObject.connect(self._sample,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self._modified)
        for widget in (self._cents, self._volume, self._sample):
            QtCore.QObject.connect(widget,
                                   QtCore.SIGNAL('editingFinished()'),
                                   self._finished)
        QtCore.QObject.connect(self._remove,
                               QtCore.SIGNAL('clicked()'),
                               self._removed)

    def set_data(self, data):
        self.blockSignals(True)
        widgets = self._cents, self._volume, self._sample
        if data:
            for i, widget in enumerate(widgets):
                widget.setValue(data[i])
            self.show()
        else:
            self.hide()
        self.blockSignals(False)

    def _modified(self, value):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('modified(int, float, float, int)'),
                            self._id, self._cents.value(),
                            self._volume.value(), self._sample.value())

    def _finished(self):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('finished(int)'),
                            self._id)

    def _removed(self):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('removed(int)'),
                            self._id)


class MapView(PlaneView):

    activeChanged = QtCore.pyqtSignal(float, float, name='activeChanged')
    modified = QtCore.pyqtSignal(name='modified')
    removed = QtCore.pyqtSignal(name='removed')
    finished = QtCore.pyqtSignal(name='finished')

    def __init__(self, mapping, map_type, parent=None):
        assert map_type in ('pitch', 'hit')
        PlaneView.__init__(self, parent)
        self._map = mapping
        self._map_type = map_type
        self._active = None
        self._focused = None
        self._aspect = None
        self._orig_pos = None
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
        if map_type == 'pitch':
            self._min = -36, -6000
            self._max = 0, 6000
            self._step_x = 0.125
            self._step_y = 100
        elif map_type == 'hit':
            self._min = -36, 0
            self._max = 0, lim.HITS_MAX - 1
            self._step_x = 0.125
            self._step_y = 1
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
        if self._map_type == 'pitch':
            self._haxis.paint(paint)
        self._vaxis.paint(paint)
        self._paint_nodes(paint)
        paint.end()

    def set_data(self, data):
        self._map = data
        self._active = None
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('activeChanged(float, float)'),
                            float('-inf'), float('-inf'))
        self.update()

    def mouseMoveEvent(self, ev):
        if self._focused and ev.buttons() & QtCore.Qt.LeftButton:
            node = self._focused
        else:
            node = self._node_at(ev.y() - 0.5, ev.x() - 0.5)
        if node != self._focused:
            if node or not ev.buttons() & QtCore.Qt.LeftButton:
                self._focused = node
                self.update()
        if node and ev.buttons() & QtCore.Qt.LeftButton:
            if self._orig_pos:
                ovx = self._view_x(self._orig_pos[1])
                ovy = self._view_y(self._orig_pos[0])
                snap = 5
                if math.hypot(ovx - ev.x(), ovy - ev.y()) > snap:
                    self._orig_pos = None
            if not self._orig_pos:
                keep_margin = 150
                keep_top_left = \
                        QtCore.QPointF(self._view_x(self._min[0]) - keep_margin,
                                       self._view_y(self._max[1]) - keep_margin)
                keep_bottom_right = \
                        QtCore.QPointF(self._view_x(self._max[0]) + keep_margin,
                                       self._view_y(self._min[1]) + keep_margin)
                keep_rect = QtCore.QRectF(keep_top_left, keep_bottom_right)
                if not keep_rect.contains(ev.x(), ev.y()):
                    del self._map[self._focused]
                    self._focused = self._active = None
                    QtCore.QObject.emit(self, QtCore.SIGNAL('removed()'))
                    self.update()
                    return
                x, y = self._val_x(ev.x()), self._val_y(ev.y())
                x = max(self._min[0], min(x, self._max[0]))
                y = max(self._min[1], min(y, self._max[1]))
                y, x = self._round_node(y, x)
                if self._map_type == 'hit':
                    y = int(y)
                if (y, x) not in self._map:
                    data = self._map.pop(self._focused)
                    self._focused = y, x
                    self._active = self._focused
                    self._map[self._focused] = data
                    QtCore.QObject.emit(self, QtCore.SIGNAL('modified()'))
                    QtCore.QObject.emit(self,
                                        QtCore.SIGNAL('activeChanged(float,'
                                                      'float)'),
                                        y, x)
                    self.update()

    def mousePressEvent(self, ev):
        assert not self._orig_pos
        node = self._node_at(ev.y() - 0.5, ev.x() - 0.5)
        if not node:
            x, y = self._val_x(ev.x()), self._val_y(ev.y())
            x = max(self._min[0], min(x, self._max[0]))
            y = max(self._min[1], min(y, self._max[1]))
            y, x = self._round_node(y, x)
            node = y, x
            self._map[node] = [[0, 0, 0]]
        if node != self._active:
            self._active = node
            y, x = node if node else (float('-inf'), float('-inf'))
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('activeChanged(float, float)'),
                                y, x)
            self.update()
        self._orig_pos = node

    def mouseReleaseEvent(self, ev):
        node = self._node_at(ev.y() - 0.5, ev.x() - 0.5)
        if node and not self._orig_pos:
            QtCore.QObject.emit(self, QtCore.SIGNAL('finished()'))
        self._orig_pos = None

    def _round_node(self, y, x):
        y = round(y / self._step_y) * self._step_y
        x = round(x / self._step_x) * self._step_x
        return y, x

    def _node_at(self, y, x):
        closest = None
        closest_dist = float('inf')
        for ny, nx in self._map.iterkeys():
            dist = math.hypot(self._view_x(nx) - x,
                              self._view_y(ny) - y)
            if dist < closest_dist:
                closest = ny, nx
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


