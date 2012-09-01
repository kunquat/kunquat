# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function
import copy
from itertools import takewhile
import math

from PyQt4 import QtCore, QtGui

import kqt_limits as lim
import kunquat
from plane_view import PlaneView, HAxis, VAxis


class Envelope(PlaneView):

    nodesChanged = QtCore.pyqtSignal(name='nodesChanged')

    def __init__(self,
                 project,
                 x_range,
                 y_range,
                 x_lock,
                 y_lock,
                 default_val,
                 nodes_max,
                 key,
                 dict_key=None,
                 step=(0.0001, 0.0001),
                 mark_modes=None,
                 parent=None):
        assert x_range[0] < x_range[1]
        assert y_range[0] < y_range[1]
        assert len(x_lock) == 2
        assert len(y_lock) == 2
        assert len(default_val) >= 2
        assert all(x_range[0] <= n[0] <= x_range[1] and
                   y_range[0] <= n[1] <= y_range[1]
                   for n in default_val)
        assert step[0] > 0
        assert step[1] > 0
        assert not mark_modes or all(mode in ('x_dashed',)
                                     for mode in mark_modes)
        PlaneView.__init__(self, parent)
        self._project = project
        self._key = key
        self._dict_key = dict_key
        self._colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'axis': QtGui.QColor(0xaa, 0xaa, 0xaa),
                'curve': QtGui.QColor(0x66, 0x88, 0xaa),
                'node': QtGui.QColor(0xee, 0xcc, 0xaa),
                'node_focus': QtGui.QColor(0xff, 0x77, 0x22),
                'mark': QtGui.QColor(0x77, 0x99, 0xbb),
                'text': QtGui.QColor(0xaa, 0xaa, 0xaa),
                }
        self._fonts = {
                'axis': QtGui.QFont('Decorative', 8),
                }
        self._visible_min = [0, 0]
        self._visible_max = [1, 1]
        self._aspect = None
        self._layout = {
                'padding': 8,
                'zoom': (200, 200),
                'offset': [0, 0],
                'visible_min': self._visible_min,
                'visible_max': self._visible_max,
                }
        self.set_constraints({
                                'x_range': x_range,
                                'y_range': y_range,
                                'first_locked': (x_lock[0], y_lock[0]),
                                'last_locked': (x_lock[1], y_lock[1]),
                                'nodes_max': nodes_max,
                                'default': { 'nodes':
                                        copy.deepcopy(default_val) },
                             })
        #self._min = x_range[0], y_range[0]
        #self._max = x_range[1], y_range[1]
        #self._first_locked = x_lock[0], y_lock[0]
        #self._last_locked = x_lock[1], y_lock[1]
        self._step = step
        #self._nodes_max = nodes_max

        self._mark_modes = mark_modes
        mark_count = min(len(mark_modes), 4) if mark_modes else 0
        self._marks = [None] * mark_count
        self._mark_visible = [False] * mark_count

        #self._default_val = copy.deepcopy(default_val)
        #self._nodes = default_val
        #self._smooth = False
        #self._haxis = HAxis(self._colours, self._fonts,
        #                    self._min, self._max, self._layout)
        #self._vaxis = VAxis(self._colours, self._fonts,
        #                    self._min, self._max, self._layout)
        self._focus_node = None
        self._focus_index = -1
        self._focus_old_node = None
        self._new_node = False
        self._drag = False
        self._drag_offset = (0, 0)
        self.setMouseTracking(True)
        self.setAutoFillBackground(False)
        self.setAttribute(QtCore.Qt.WA_OpaquePaintEvent)
        self.setAttribute(QtCore.Qt.WA_NoSystemBackground)

    def init(self):
        self._set_view()

    def set_key(self, key):
        self._key = key
        value = {}
        if self._dict_key:
            d = self._project[key]
            if d and self._dict_key in d:
                value = self._project[key][self._dict_key]
        else:
            actual = self._project[key]
            if actual != None:
                value = actual
        old_node_count = self.node_count()
        self._nodes = value['nodes'] if 'nodes' in value \
                                     else copy.deepcopy(self._default_val)
        if old_node_count != self.node_count():
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('nodesChanged()'))
        self._marks = value['marks'] if 'marks' in value else []
        """
        x_range = self._min[0], self._max[0]
        if x_range[0] < -1 or x_range[1] > 1:
            x_range = (min(n[0] for n in self._nodes),
                       max(n[0] for n in self._nodes))
        y_range = self._min[1], self._max[1]
        if y_range[0] < -1 or y_range[1] > 1:
            y_range = (min(n[1] for n in self._nodes),
                       max(n[1] for n in self._nodes))
        self._visible_min = x_range[0], y_range[0]
        self._visible_max = x_range[1], y_range[1]
        """
        self._set_view()
        self.resizeEvent(None)
        self.update()

    def sync(self):
        self.set_key(self._key)

    def set_constraints(self, constraints):
        try:
            x_range = constraints['x_range']
            x_min = float(x_range[0])
            x_max = float(x_range[1])
            if x_min > x_max:
                raise ValueError
        except (IndexError, KeyError, TypeError, ValueError):
            x_min, x_max = 0, 1
        try:
            y_range = constraints['y_range']
            y_min = float(y_range[0])
            y_max = float(y_range[1])
            if y_min > y_max:
                raise ValueError
        except (IndexError, KeyError, TypeError, ValueError):
            y_min, y_max = 0, 1
        self._min = x_min, y_min
        self._max = x_max, y_max
        try:
            first_locked = constraints['first_locked']
            self._first_locked = bool(first_locked[0]), bool(first_locked[1])
        except (IndexError, KeyError, TypeError, ValueError):
            self._first_locked = False, False
        try:
            last_locked = constraints['last_locked']
            self._last_locked = bool(last_locked[0]), bool(last_locked[1])
        except (IndexError, KeyError, TypeError, ValueError):
            self._last_locked = False, False
        try:
            env = constraints['default']
            self._default_val = [(float(x), float(y))
                                 for x, y in env['nodes']]
        except (IndexError, KeyError, TypeError, ValueError):
            self._default_val = [[0, 1], [1, 0]]
        self._nodes = self._default_val
        try:
            env = constraints['default']
            self._smooth = bool(env['smooth'])
        except (KeyError, TypeError, ValueError):
            self._smooth = False
        try:
            self._nodes_max = int(constraints['nodes_max'])
            if self._nodes_max < 2:
                self._nodes_max = 16
        except (KeyError, TypeError, ValueError):
            self._nodes_max = 16
        self._haxis = HAxis(self._colours, self._fonts,
                            self._min, self._max, self._layout)
        self._vaxis = VAxis(self._colours, self._fonts,
                            self._min, self._max, self._layout)

    def set_mark(self, index, value):
        assert value == None or 0 <= value < len(self._nodes)
        self._marks.extend([None] * (index - len(self._marks) + 1))
        self._marks[index] = value
        self._value_changed()
        self.update()

    def set_mark_display(self, index, value):
        assert 0 <= index < len(self._mark_visible)
        self._mark_visible[index] = value
        self.update()

    def node_count(self):
        return len(self._nodes)

    def keyPressEvent(self, ev):
        pass

    def mouseMoveEvent(self, ev):
        if self._drag:
            keep_margin = 150
            keep_top_left = \
                    QtCore.QPointF(self._view_x(self._min[0]) - keep_margin,
                                   self._view_y(self._max[1]) - keep_margin)
            keep_bottom_right = \
                    QtCore.QPointF(self._view_x(self._max[0]) + keep_margin,
                                   self._view_y(self._min[1]) + keep_margin)
            keep_rect = QtCore.QRectF(keep_top_left, keep_bottom_right)
            if not keep_rect.contains(ev.x(), ev.y()) and \
                    0 < self._focus_index < len(self._nodes) - 1:
                self._focus_node = None
                self._nodes[self._focus_index:self._focus_index + 1] = []
                self._marks = [m if m == None or m < self._focus_index
                                 else m - 1 for m in self._marks]
                self._focus_index = -1
                self._drag = False
                if self._new_node:
                    self._project.cancel(self._key)
                else:
                    self._value_changed()
                    self.finished()
                QtCore.QObject.emit(self, QtCore.SIGNAL('nodesChanged()'))
                self._new_node = False
                self._focus_old_node = None
                self._slow_update()
                return
            pos = (self._val_x(ev.x() + self._drag_offset[0]),
                   self._val_y(ev.y() + self._drag_offset[1]))
            self._move_node(self._focus_index, pos)
            self._focus_node = self._nodes[self._focus_index]
            self._slow_update()
            return
        focus_node, index = self._node_at(ev.x() - 0.5, ev.y() - 0.5)
        if focus_node != self._focus_node:
            self._focus_node = focus_node
            self._focus_index = index
            self._slow_update()

    def mousePressEvent(self, ev):
        focus_node, index = self._node_at(ev.x() - 0.5, ev.y() - 0.5)
        if not focus_node:
            if len(self._nodes) == self._nodes_max:
                return
            focus_node = self._val_x(ev.x()), self._val_y(ev.y())
            if not self._min[0] <= focus_node[0] <= self._max[0] or \
                    not self._min[1] <= focus_node[1] <= self._max[1]:
                return
            index = sum(1 for _ in takewhile(lambda n: n[0] < focus_node[0],
                                             self._nodes))
            for i, x in ((index, focus_node[0]),
                         (index + 1, focus_node[0] + self._step[0]),
                         (index - 1, focus_node[0] - self._step[0])):
                if not 0 <= i < len(self._nodes) or \
                        abs(x - self._nodes[i][0]) >= self._step[0]:
                    ic = max(i, index)
                    if ic == 0 and self._first_locked[0]:
                        continue
                    if ic == len(self._nodes) and self._last_locked[0]:
                        continue
                    index = ic
                    focus_node = (x, focus_node[1])
                    break
            else:
                return
            if not self._min[0] <= focus_node[0] <= self._max[0]:
                return
            if focus_node[0] > self._nodes[-1][0] and self._last_locked[1]:
                focus_node = focus_node[0], self._nodes[-1][1]
            self._nodes[index:index] = [focus_node]
            self._new_node = True
            self._marks = [m if m == None or m < index else m + 1
                           for m in self._marks]
            self._value_changed()
            QtCore.QObject.emit(self, QtCore.SIGNAL('nodesChanged()'))
        else:
            self._focus_old_node = focus_node
        self._focus_node = focus_node
        self._focus_index = index
        self._drag = True
        self._drag_offset = (self._view_x(focus_node[0]) - ev.x(),
                             self._view_y(focus_node[1]) - ev.y())
        self.update()

    def mouseReleaseEvent(self, ev):
        if self._drag:
            pos = (self._val_x(ev.x() + self._drag_offset[0]),
                   self._val_y(ev.y() + self._drag_offset[1]))
            self._move_node(self._focus_index, pos)
            self._drag = False
            self._new_node = False
            self.finished()
            self._focus_old_node = None
            self.update()

    def _move_node(self, index, pos):
        min_x, min_y = self._min
        max_x, max_y = self._max
        if index > 0:
            min_x = self._nodes[index - 1][0] + self._step[0]
        if index < len(self._nodes) - 1:
            max_x = self._nodes[index + 1][0] - self._step[1]
        if index == 0:
            if self._first_locked[0]:
                min_x = max_x = self._nodes[0][0]
            if self._first_locked[1]:
                min_y = max_y = self._nodes[0][1]
        elif index == len(self._nodes) - 1:
            if self._last_locked[0]:
                min_x = max_x = self._nodes[-1][0]
            if self._last_locked[1]:
                min_y = max_y = self._nodes[-1][1]
        pos = (max(min_x, min(max_x, pos[0])),
               max(min_y, min(max_y, pos[1])))
        self._nodes[index] = pos
        self._value_changed()
        #self._adjust_view(pos)
        self._set_view()

    def _value_changed(self):
        value = { 'nodes': self._nodes }
        if self._marks:
            value['marks'] = [(i if i != None else -1) for i in self._marks]
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            self._project.set(self._key, d, immediate=False)
        else:
            self._project.set(self._key, value, immediate=False)

    def finished(self):
        self._project.flush(self._key)

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
            return closest, closest_index
        return None, -1

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setRenderHint(QtGui.QPainter.Antialiasing)
        paint.setBackground(self._colours['bg'])
        paint.eraseRect(ev.rect())
        self._haxis.paint(paint)
        self._vaxis.paint(paint)
        if not self._smooth or len(self._nodes) <= 2:
            self._paint_linear_curve(paint)
        else:
            self._paint_nurbs_curve(paint)
        self._paint_marks(paint)
        self._paint_nodes(paint)
        paint.end()

    def _paint_linear_curve(self, paint):
        paint.setPen(self._colours['curve'])
        line = QtGui.QPolygonF()
        for p in self._nodes:
            line.append(QtCore.QPointF(self._view_x(p[0]) + 0.5,
                                       self._view_y(p[1]) + 0.5))
        paint.drawPolyline(line)

    def _paint_nurbs_curve(self, paint):
        paint.setPen(self._colours['curve'])
        nurbs = Nurbs(2, self._nodes)
        start_x = int(self._view_x(self._nodes[0][0]))
        end_x = int(self._view_x(self._nodes[-1][0]))
        curve_width = min(end_x - start_x, 100)
        line = QtGui.QPolygonF()
        for pos in xrange(curve_width + 1):
            x, y = nurbs.get_point(pos / curve_width)
            x, y = self._view_x(x), self._view_y(y)
            line.append(QtCore.QPointF(x + 0.5, y + 0.5))
        paint.drawPolyline(line)

    def _paint_marks(self, paint):
        for i, mark in enumerate(self._marks):
            if mark != None and self._mark_visible[i]:
                if self._mark_modes[i] == 'x_dashed':
                    pen = QtGui.QPen(self._colours['mark'])
                    pen.setDashPattern([4, 4])
                    paint.setPen(pen)
                    node = self._nodes[mark]
                    node_x = self._view_x(node[0]) + 0.5
                    max_y = self._view_y(self._max[1]) + 0.5
                    if max_y == float('inf'):
                        max_y = 10000
                    axis_y = self._view_y(0) + 0.5
                    paint.drawLine(QtCore.QPointF(node_x, axis_y),
                                   QtCore.QPointF(node_x, max_y))

    def _paint_nodes(self, paint):
        paint.setPen(QtCore.Qt.NoPen)
        paint.setBrush(self._colours['node'])
        rect_size = 5
        for p in self._nodes + [self._focus_node]:
            if not p:
                continue
            if p == self._focus_node:
                paint.setBrush(self._colours['node_focus'])
            rect_x = self._view_x(p[0]) - rect_size / 2 + 0.5
            rect_y = self._view_y(p[1]) - rect_size / 2 + 0.5
            rect = QtCore.QRectF(rect_x, rect_y, rect_size, rect_size)
            paint.fillRect(rect, paint.brush())
            if p == self._focus_node:
                paint.setBrush(self._colours['node'])

    def _slow_update(self):
        self.update()

    def _set_view(self):
        step_factor = 1.03
        if self._min[0] >= -1 and self._max[0] <= 1:
            if (self._min[0], self._max[0]) == (self._min[1], self._max[1]):
                self._aspect = 1
            self._visible_min[0] = self._min[0]
            self._visible_max[0] = self._max[0]
        else:
            negative = max(0, -self._nodes[0][0])
            positive = max(0, self._nodes[-1][0])
            if negative > 0:
                negative = step_factor**math.ceil(
                                   math.log(negative, step_factor))
            if positive > 0:
                positive = step_factor**math.ceil(
                                   math.log(positive, step_factor))
            if not negative and not positive:
                positive += 1
            self._visible_min[0] = -negative
            self._visible_max[0] = positive
        if self._min[1] >= -1 and self._max[1] <= 1:
            self._visible_min[1] = self._min[1]
            self._visible_max[1] = self._max[1]
        else:
            negative = max(0, -min(n[1] for n in self._nodes))
            positive = max(0, max(n[1] for n in self._nodes))
            if negative > 0:
                negative = step_factor**math.ceil(
                                   math.log(negative, step_factor))
            if positive > 0:
                positive = step_factor**math.ceil(
                                   math.log(positive, step_factor))
            if not negative and not positive:
                positive += 1
            self._visible_min[1] = -negative
            self._visible_max[1] = positive
        #print(self._visible_min, self._visible_max)
        self._layout['offset'][0] = (-self._visible_min[0] /
                                (self._visible_max[0] - self._visible_min[0]))
        self._layout['offset'][1] = (-self._visible_min[1] /
                                (self._visible_max[1] - self._visible_min[1]))
        self.resizeEvent(None)


class Nurbs(object):

    def __init__(self, degree, controls):
        self._controls = controls
        self._degree = degree
        self._order = self._degree + 1
        self._knots = [0] * self._order
        self._knots.extend(xrange(1, len(self._controls) - 1))
        self._knots.extend([len(self._controls) - 2] * (self._order + 1))

    def get_point(self, pos):
        u = pos * self._knots[-1]
        point_x, point_y = (0, 0)
        contrib_sum = 0
        for i, control in enumerate(self._controls):
            contrib = self._basis(i, self._degree, u)
            contrib_sum += contrib
            val_x = control[0] * contrib
            val_y = control[1] * contrib
            point_x += val_x
            point_y += val_y
        if contrib_sum < 1:
            last_x, last_y = self._controls[-1]
            point_x += (1 - contrib_sum) * last_x
            point_y += (1 - contrib_sum) * last_y
        return point_x, point_y

    def get_y(self, x):
        ci = sum(1 for _ in takewhile(lambda n: n[0] < x, self._controls))
        ci = max(0, ci - 1)
        if ci >= len(self._controls) - 1:
            u = self._knots[-1]
        else:
            remainder = (x - self._controls[ci][0]) / \
                    (self._controls[ci + 1][0] - self._controls[ci][0])
            u = ci + remainder
        u = self._knots[-1] * (x - self._controls[0][0]) / \
                              (self._controls[-1][0] - self._controls[0][0])
        y = 0
        contrib_sum = 0
        for i, control in enumerate(self._controls):
            contrib = self._basis(i, self._degree, u)
            contrib_sum += contrib
            val = control[1] * contrib
            y += val
        if contrib_sum < 1:
            y += (1 - contrib_sum) * self._controls[-1][1]
        return y

    def _basis(self, ci, deg, u):
        if not deg:
            return 1 if self._knots[ci] <= u < self._knots[ci + 1] else 0
        return self._ramp_up(ci, deg, u) * self._basis(ci, deg - 1, u) + \
            self._ramp_down(ci + 1, deg, u) * self._basis(ci + 1, deg - 1, u)

    def _ramp_up(self, ci, deg, u):
        if ci + deg >= len(self._knots) or \
                self._knots[ci] == self._knots[ci + deg]:
            return 0
        return (u - self._knots[ci]) / (self._knots[ci + deg] -
                                        self._knots[ci])

    def _ramp_down(self, ci, deg, u):
        if ci + deg >= len(self._knots) or \
                self._knots[ci] == self._knots[ci + deg]:
            return 0
        return (self._knots[ci + deg] - u) / (self._knots[ci + deg] -
                                              self._knots[ci])


