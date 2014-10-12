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

import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *


DEFAULT_CONFIG = {
        'axis_x': {
            'height': 20,
            'line_min_dist' : 6,
            'line_max_width': 5,
            'line_min_width': 2,
            },
        'axis_y': {
            'width'         : 50,
            'line_min_dist' : 6,
            'line_max_width': 5,
            'line_min_width': 2,
            },
        'padding'    : 2,
        'bg_colour'  : QColor(0, 0, 0),
        'axis_colour': QColor(0xcc, 0xcc, 0xcc),
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

        self.set_x_range(0, 4)
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

    def _fill_markers_interval(
            self,
            painter,
            draw_func,
            val_start, val_stop,
            px_start, px_stop,
            dist,
            marker_dist_min,
            marker_width):
        if dist < marker_dist_min * 2:
            return

        val_center = (val_start + val_stop) / 2
        px_center = int((px_start + px_stop) / 2)

        draw_func(painter, px_center, marker_width)

        self._fill_markers_interval(
                painter,
                draw_func,
                val_start, val_center,
                px_start, px_center,
                dist / 2.0,
                marker_dist_min,
                marker_width - 1)
        self._fill_markers_interval(
                painter,
                draw_func,
                val_center, val_stop,
                px_center, px_stop,
                dist / 2.0,
                marker_dist_min,
                marker_width - 1)

    def _draw_axis_x(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(
            self._envelope_offset_x, self._axis_x_offset_y))

        # Test
        #painter.setPen(QColor(0xff, 0xff, 0xff))
        #painter.drawRect(0, 0, self._envelope_width - 1, self._config['axis_x']['height'] - 1)

        # Draw line along the axis
        painter.setPen(self._config['axis_colour'])
        painter.drawLine(0, 0, self._envelope_width - 1, 0)

        # Marker drawing callback
        def draw_marker(painter, px_center, marker_width):
            marker_width = max(marker_width, self._config['axis_x']['line_min_width'])
            marker_start = marker_width
            painter.drawLine(px_center, marker_start, px_center, 1)

        # Get initial marker width
        marker_width = self._config['axis_x']['line_max_width']

        # Draw zero marker if not obscured by the y axis
        zero_x = (self._axis_y_offset_x + self._config['axis_y']['width'] -
                self._envelope_offset_x - 1)
        if self._range_y[0] == 0:
            draw_marker(painter, zero_x, marker_width)

        # Get interval of whole number values to mark
        display_val_max = int(math.ceil(self._range_x[1]))
        display_val_min = int(math.floor(self._range_x[0]))
        px_per_whole = self._envelope_width // (display_val_max - display_val_min)

        # Draw non-zero markers
        marker_dist_min = self._config['axis_x']['line_min_dist']
        if marker_dist_min <= px_per_whole:
            # Positive side
            start_x = zero_x
            pos_width = self._envelope_width - start_x - 1

            for i in range(0, display_val_max):
                end_x = int(zero_x + ((i + 1) * pos_width / display_val_max))

                draw_marker(painter, end_x, marker_width)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        i, i + 1,
                        start_x, end_x,
                        px_per_whole,
                        marker_dist_min,
                        marker_width - 1)

                start_x = end_x

            # Negative side
            start_x = zero_x
            neg_width = start_x

            for i in range(0, -display_val_min):
                end_x = int(zero_x - ((i + 1) * neg_width / -display_val_min))

                draw_marker(painter, end_x, marker_width)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        i, i + 1,
                        start_x, end_x,
                        px_per_whole,
                        marker_dist_min,
                        marker_width - 1)

                start_x = end_x

        elif px_per_whole >= 1:
            # Skipping whole numbers
            whole_marker_interval = int(2**(math.ceil(
                math.log(marker_dist_min / float(px_per_whole), 2))))

            # Positive side
            start_x = zero_x
            pos_width = self._envelope_width - start_x - 1

            for i in range(0, display_val_max, whole_marker_interval):
                end_x = int(zero_x +
                        ((i + whole_marker_interval) * pos_width / display_val_max))

                if i + whole_marker_interval <= display_val_max:
                    draw_marker(painter, end_x, marker_width)

            # Negative side
            start_x = zero_x
            neg_width = start_x

            for i in range(0, -display_val_min, whole_marker_interval):
                end_x = int(zero_x -
                        ((i + whole_marker_interval) * neg_width / -display_val_min))

                if i + whole_marker_interval <= -display_val_min:
                    draw_marker(painter, end_x, marker_width)

        painter.restore()

    def _draw_axis_y(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(self._axis_y_offset_x, padding))

        axis_width = self._config['axis_y']['width']

        # Test
        #painter.setPen(QColor(0, 0xff, 0))
        #painter.drawRect(0, 0, self._config['axis_y']['width'] - 1, self._envelope_height - 1)

        # Draw line along the axis
        painter.setPen(self._config['axis_colour'])
        painter.drawLine(axis_width - 1, 0, axis_width - 1, self._envelope_height - 1)

        # Marker drawing callback
        def draw_marker(painter, px_center, marker_width):
            marker_width = max(marker_width, self._config['axis_y']['line_min_width'])
            marker_start = axis_width - marker_width - 1
            painter.drawLine(marker_start, px_center, axis_width - 2, px_center)

        # Get initial marker width
        marker_width = self._config['axis_y']['line_max_width']

        # Draw zero marker if not obscured by the x axis
        zero_y = self._axis_x_offset_y - padding
        if self._range_x[0] == 0:
            draw_marker(painter, zero_y, marker_width)

        # Get interval of whole number values to mark
        display_val_max = int(math.ceil(self._range_y[1]))
        display_val_min = int(math.floor(self._range_y[0]))
        px_per_whole = self._envelope_height // (display_val_max - display_val_min)

        # Draw non-zero markers
        marker_dist_min = self._config['axis_y']['line_min_dist']
        if marker_dist_min <= px_per_whole:
            # Positive side
            start_y = zero_y
            pos_height = start_y

            for i in range(0, display_val_max):
                end_y = int(zero_y - ((i + 1) * pos_height / display_val_max))

                draw_marker(painter, end_y, marker_width)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        i, i + 1,
                        start_y, end_y,
                        px_per_whole,
                        marker_dist_min,
                        marker_width - 1)

                start_y = end_y

            # Negative side
            start_y = zero_y
            neg_height = self._envelope_height - pos_height - 1

            for i in range(0, -display_val_min):
                end_y = int(zero_y + ((i + 1) * neg_height / -display_val_min))

                draw_marker(painter, end_y, marker_width)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        -i, -i - 1,
                        start_y, end_y,
                        px_per_whole,
                        marker_dist_min,
                        marker_width - 1)

                start_y = end_y

        elif px_per_whole >= 1:
            # Skipping whole numbers
            whole_marker_interval = int(2**(math.ceil(
                math.log(marker_dist_min / float(px_per_whole), 2))))

            # Positive side
            start_y = zero_y
            pos_height = start_y

            for i in range(0, display_val_max, whole_marker_interval):
                end_y = int(zero_y -
                        ((i + whole_marker_interval) * pos_height / display_val_max))

                if i + whole_marker_interval <= display_val_max:
                    draw_marker(painter, end_y, marker_width)

            # Negative side
            start_y = zero_y
            neg_height = self._envelope_height - pos_height - 1

            for i in range(0, -display_val_min, whole_marker_interval):
                end_y = int(zero_y +
                        ((i + whole_marker_interval) * neg_height / -display_val_min))

                if i + whole_marker_interval <= -display_val_min:
                    draw_marker(painter, end_y, marker_width)

        painter.restore()

    def _draw_envelope_curve(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(self._envelope_offset_x, padding))

        # Test
        #painter.setPen(QColor(0xff, 0, 0))
        #painter.drawRect(0, 0, self._envelope_width - 1, self._envelope_height - 1)

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
        #if x_range_width == y_range_height:
        #    available_width_px = min(available_width_px, available_height_px)
        #    available_height_px = available_width_px

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


