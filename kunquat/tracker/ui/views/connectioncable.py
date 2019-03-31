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

        min_x = min(start_x, end_x, control_end_x)
        max_x = max(start_x, end_x, control_start_x)
        min_y = min(start_y, end_y)
        max_y = max(start_y, end_y)

        self._start_offset_x = start_x - min_x
        self._area_width = max_x - min_x
        self._area_height = max_y - min_y

        self._start_point = QPointF(start_x - min_x, start_y - min_y)
        self._control_start = QPointF(control_start_x - min_x, self._start_point.y())
        self._mid_point = QPointF(
                ((start_x + end_x) / 2) - min_x,
                ((start_y + end_y) / 2) - min_y)
        self._end_point = QPointF(end_x - min_x, end_y - min_y)
        self._control_end = QPointF(control_end_x - min_x, self._end_point.y())

        self._image_offset_x = start_x - self._start_point.x()
        self._image_offset_y = min_y

        self._width = 1
        self._colour = None

    def set_width(self, width):
        self._width = width

    def set_colour(self, colour):
        self._colour = colour

    def draw_line(self):
        if not self._image:
            width = self._area_width + self._width
            height = self._area_height + self._width
            self._image = QImage(width, height, QImage.Format_ARGB32_Premultiplied)
            self._image.fill(0)

        painter = QPainter(self._image)
        line_width_offset = math.floor((self._width - 1) / 2) + 0.5
        painter.translate(line_width_offset, line_width_offset)
        painter.setRenderHint(QPainter.Antialiasing)

        # Test
        #painter.setPen(QColor(0, 0xff, 0xff))
        #painter.drawRect(0, 0, self._image.width() - 1, self._image.height() - 1)

        pen = QPen(self._colour)
        pen.setWidthF(self._width)
        painter.setPen(pen)

        path = QPainterPath()
        path.moveTo(self._start_point)
        path.quadTo(self._control_start, self._mid_point)
        path.quadTo(self._control_end, self._end_point)

        painter.drawPath(path)

        painter.setPen(QColor(0, 0, 0))
        painter.drawPoint(self._start_point)
        painter.drawPoint(self._end_point)

    def copy_line(self, painter):
        line_width_offset = math.floor((self._width - 1) / 2) + 0.5
        painter.drawImage(
                QPointF(
                    self._image_offset_x - line_width_offset,
                    self._image_offset_y - line_width_offset),
                self._image)


