# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from kunquat.tracker.ui.qt import *


class ConnectionCable():

    def __init__(self, start_point, end_point):
        start_x, start_y = start_point
        end_x, end_y = end_point

        length_x = abs(end_x - start_x)
        length_y = abs(end_y - start_y)
        line_length = math.sqrt((length_x * length_x) + (length_y * length_y))
        control_dist = line_length / 3
        control_start_x = start_x + control_dist
        control_end_x = end_x - control_dist

        self._image = None
        self._focus_map = None

        min_x = min(start_x, end_x, control_end_x)
        max_x = max(start_x, end_x, control_start_x)
        min_y = min(start_y, end_y)
        max_y = max(start_y, end_y)

        self._start_point_abs = QPointF(start_x, start_y)
        self._texture_translation = QPointF(start_x - min_x, start_y - min_y)

        self._area_width = max_x - min_x
        self._area_height = max_y - min_y

        self._control_start = QPointF(control_dist, 0)
        self._mid_point = QPointF(
                ((start_x + end_x) / 2) - start_x,
                ((start_y + end_y) / 2) - start_y)
        self._end_point = QPointF(end_x - start_x, end_y - start_y)
        self._control_end = QPointF(control_end_x - start_x, end_y - start_y)

        self._width = 1
        self._colour = None
        self._focus_dist = 5

    def set_width(self, width):
        self._width = width
        self._image = None

    def set_colour(self, colour):
        self._colour = colour
        self._image = None

    def set_focus_dist(self, dist):
        self._focus_dist = dist

    def draw_cable(self, painter):
        painter.save()

        painter.translate(self._start_point_abs)
        painter.setRenderHint(QPainter.Antialiasing)

        # Test
        #painter.setPen(QColor(0, 0xff, 0xff))
        #painter.drawRect(0, 0, self._image.width() - 1, self._image.height() - 1)

        path = QPainterPath()
        path.moveTo(QPointF(0, 0))
        path.quadTo(self._control_start, self._mid_point)
        path.quadTo(self._control_end, self._end_point)

        painter.setBrush(Qt.NoBrush)
        painter.drawPath(path)

        painter.restore()

    def make_cable(self):
        if not self._image:
            width = self._area_width + self._width
            height = self._area_height + self._width
            self._image = QImage(width, height, QImage.Format_ARGB32_Premultiplied)
            self._image.fill(0)

            painter = QPainter(self._image)

            lwo_amount = self._width / 2
            lwo = QPointF(lwo_amount, lwo_amount)
            painter.translate(self._texture_translation + lwo - self._start_point_abs)

            pen = QPen(self._colour)
            pen.setWidthF(self._width)
            painter.setPen(pen)

            self.draw_cable(painter)

            painter.end()

        if not self._focus_map:
            width = self._area_width + self._focus_dist * 2
            height = self._area_height + self._focus_dist * 2
            self._focus_map = QImage(width, height, QImage.Format_Mono)
            self._focus_map.fill(0)

            painter = QPainter(self._focus_map)

            lwo_amount = self._focus_dist
            lwo = QPointF(lwo_amount, lwo_amount)
            painter.translate(self._texture_translation + lwo - self._start_point_abs)

            pen = QPen(Qt.color0)
            pen.setWidthF(self._focus_dist * 2)
            pen.setCapStyle(Qt.FlatCap)
            painter.setPen(pen)

            self.draw_cable(painter)

            painter.end()

    def copy_cable(self, painter):
        lwo_amount = ((self._width - 1) / 2)
        lwo = QPointF(lwo_amount, lwo_amount)
        painter.drawImage(
                -self._texture_translation + self._start_point_abs - lwo, self._image)

    def is_near_point(self, point):
        assert self._focus_map

        tfm = QTransform()
        lwo = QPointF(self._focus_dist, self._focus_dist)
        shift = self._texture_translation + lwo - self._start_point_abs
        tfm = tfm.translate(shift.x(), shift.y())
        img_point = tfm.map(QPointF(*point))

        x = int(img_point.x())
        y = int(img_point.y())
        width = self._focus_map.width()
        height = self._focus_map.height()
        if not ((0 <= x < width) and (0 <= y < height)):
            return False

        colour = self._focus_map.pixelColor(x, y)
        return (colour.red() != 0)

    def debug_show_focus_map(self, painter):
        lwo_amount = self._focus_dist
        lwo = QPointF(lwo_amount, lwo_amount)
        painter.drawImage(
                -self._texture_translation + self._start_point_abs - lwo,
                self._focus_map)


