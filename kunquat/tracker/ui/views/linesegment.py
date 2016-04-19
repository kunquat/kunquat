# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *


class LineSegment():

    def __init__(self, from_point, to_point):
        from_x, from_y = from_point
        to_x, to_y = to_point
        if from_x > to_x:
            from_x, to_x = to_x, from_x
            from_y, to_y = to_y, from_y

        self._update_id = None

        width = to_x - from_x + 1
        height = abs(to_y - from_y) + 1

        self._offset_x = from_x
        self._offset_y = min(from_y, to_y)

        self._is_ascending = from_y > to_y

        self._image = QImage(width, height, QImage.Format_ARGB32_Premultiplied)
        self._image.fill(0)

        self._colour = None

    def set_colour(self, colour):
        self._colour = colour

    def draw_line(self):
        painter = QPainter(self._image)
        painter.translate(0.5, 0.5)
        painter.setRenderHint(QPainter.Antialiasing)

        # Test
        #painter.setPen(QColor(0, 0xff, 0xff))
        #painter.drawRect(0, 0, self._image.width() - 1, self._image.height() - 1)

        painter.setPen(self._colour)

        y1 = 0
        y2 = self._image.height() - 1
        if self._is_ascending:
            y1, y2 = y2, y1

        painter.drawLine(0, y1, self._image.width() - 1, y2)

    def copy_line(self, painter):
        painter.save()
        painter.translate(self._offset_x, self._offset_y)
        painter.drawImage(0, 0, self._image)
        painter.restore()

    def set_update_id(self, update_id):
        self._update_id = update_id

    def get_update_id(self):
        return self._update_id


