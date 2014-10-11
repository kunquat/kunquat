# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *


DEFAULT_CONFIG = {
        'padding': 2,
        'axis_x': {
            'height': 20,
            },
        'axis_y': {
            'width': 50,
            },
        'bg_colour': QColor(0, 0, 0),
    }


def signum(x):
    if x > 0:
        return 1
    elif x < 0:
        return -1
    return 0


class Envelope(QWidget):

    def __init__(self, config={}):
        QWidget.__init__(self)

        self._range_x = None
        self._range_y = None

        self.set_x_range(0, 2)
        self.set_y_range(0, 1)

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_x_range(self, min_x, max_x):
        assert signum(min_x) != signum(max_x)
        self._range_x = (min_x, max_x)

    def set_y_range(self, min_y, max_y):
        assert signum(min_y) != signum(max_y)
        self._range_y = (min_y, max_y)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

    def _get_x_range_width(self):
        return (self._range_x[1] - self._range_x[0])

    def _get_y_range_height(self):
        return (self._range_y[1] - self._range_y[0])

    def _is_vis_state_valid(self):
        return ((self._axis_y_offset_x >= 0) and
                (self._axis_x_offset_y >= 0) and
                (self._envelope_offset_x >= 0) and
                (self._envelope_width > 0) and
                (self._envelope_height > 0))

    def _draw_axis_x(self, painter):
        painter.save()

        painter.setTransform(QTransform().translate(
            self._envelope_offset_x, self._axis_x_offset_y))

        painter.setPen(QColor(0xff, 0xff, 0xff))
        painter.drawRect(0, 0, self._envelope_width - 1, self._config['axis_x']['height'] - 1)

        painter.restore()

    def _draw_axis_y(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(self._axis_y_offset_x, padding))

        painter.setPen(QColor(0, 0xff, 0))
        painter.drawRect(0, 0, self._config['axis_y']['width'] - 1, self._envelope_height - 1)

        painter.restore()

    def _draw_envelope_curve(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(self._envelope_offset_x, padding))

        painter.setPen(QColor(0xff, 0, 0))
        painter.drawRect(0, 0, self._envelope_width - 1, self._envelope_height - 1)

        painter.restore()

    def _draw_envelope_nodes(self, painter):
        painter.save()
        painter.restore()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        painter.setPen(QColor(0xff, 0, 0xff))
        painter.drawRect(QRect(0, 0, self.width() - 1, self.height() - 1))

        if not self._is_vis_state_valid():
            return

        self._draw_axis_y(painter)
        self._draw_axis_x(painter)
        self._draw_envelope_curve(painter)
        self._draw_envelope_nodes(painter)

        end = time.time()
        elapsed = end - start
        print('Envelope view updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, event):
        # Get total area available
        padding = self._config['padding']

        available_width_px = self.width() - padding * 2
        available_height_px = self.height() - padding * 2

        x_range_width = self._get_x_range_width()
        y_range_height = self._get_y_range_height()

        # Make the envelope area square if the ranges match in length
        if x_range_width == y_range_height:
            available_width_px = min(available_width_px, available_height_px)
            available_height_px = available_width_px

        axis_y_width = self._config['axis_y']['width']
        axis_x_height = self._config['axis_x']['height']

        axis_y_offset_x_px = 0
        axis_x_offset_y_px = available_height_px - axis_x_height

        envelope_offset_x_px = 0
        envelope_width_px = available_width_px
        envelope_height_px = available_height_px

        # Get left border of the envelope
        x_left_width = abs(self._range_x[0])
        x_left_width_px = int(round(available_width_px * x_left_width / x_range_width))
        if x_left_width_px < axis_y_width:
            extra_space = axis_y_width - x_left_width_px
            envelope_offset_x_px = extra_space
            envelope_width_px -= extra_space
        elif x_left_width_px > axis_y_width:
            axis_y_offset_x_px = x_left_width_px - axis_y_width

        # Get bottom border of the envelope
        y_down_height = abs(self._range_y[0])
        y_down_height_px = int(round(
            available_height_px * y_down_height / y_range_height))
        if y_down_height_px < axis_x_height:
            extra_space = axis_x_height - y_down_height_px
            envelope_height_px -= extra_space
        elif y_down_height_px > axis_x_height:
            axis_x_offset_y_px -= (y_down_height_px - axis_x_height)

        # Make sure the envelope area overlaps with axes
        if envelope_offset_x_px == axis_y_offset_x_px + axis_y_width:
            envelope_offset_x_px -= 1
            envelope_width_px += 1
        if envelope_height_px == axis_x_offset_y_px:
            envelope_height_px += 1

        # Set final values
        self._axis_y_offset_x = axis_y_offset_x_px + padding
        self._axis_x_offset_y = axis_x_offset_y_px + padding
        self._envelope_offset_x = envelope_offset_x_px + padding
        self._envelope_width = envelope_width_px
        self._envelope_height = envelope_height_px

        self.update()


