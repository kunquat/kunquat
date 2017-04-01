# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from PySide.QtCore import *
from PySide.QtGui import *


class AbstractAxisRenderer():

    def __init__(self):
        self._config = None
        self._cache = None
        self._width = 0
        self._height = 0
        self._axis_length = 0
        self._val_range = (0, 0)

        self._draw_zero_marker = True

    def set_config(self, config, containing_widget):
        self._config = config
        self.flush_cache()

        fm = QFontMetrics(self._config['label_font'], containing_widget)
        self._num_space = fm.boundingRect('-00.000')

    def set_draw_zero_marker_enabled(self, enabled):
        if self._draw_zero_marker != enabled:
            self._draw_zero_marker = enabled
            self.flush_cache()

    def flush_cache(self):
        self._cache = None

    def set_width(self, width):
        if self._width != width:
            self._width = width
            self.flush_cache()

    def set_height(self, height):
        if self._height != height:
            self._height = height
            self.flush_cache()

    def set_axis_length(self, length):
        if self._axis_length != length:
            self._axis_length = length
            self.flush_cache()

    def set_val_range(self, val_range):
        if self._val_range != val_range:
            self._val_range = val_range
            self.flush_cache()

    def render(self, painter):
        if (self._width <= 0) or (self._height <= 0):
            return

        if not self._cache:
            self._cache = QImage(self._width, self._height, QImage.Format_ARGB32)
            self._cache.fill(0)
            axis_painter = QPainter(self._cache)
            self._draw_axis(axis_painter)
        painter.drawImage(QPoint(0, 0), self._cache)

    def _fill_markers_interval(
            self,
            painter,
            draw_marker_func,
            full_val_range,
            val_start, val_stop,
            px_start, px_stop,
            dist,
            marker_dist_min,
            marker_width,
            draw_label_func,
            label_dist_min):
        if dist < marker_dist_min * 2:
            return

        val_centre = (val_start + val_stop) * 0.5
        px_centre = int((px_start + px_stop) / 2)

        if full_val_range[0] <= val_centre <= full_val_range[1]:
            draw_marker_func(painter, px_centre, marker_width)
            if dist >= label_dist_min * 2:
                draw_label_func(painter, px_centre, val_centre)

        if val_centre >= full_val_range[0]:
            self._fill_markers_interval(
                    painter,
                    draw_marker_func,
                    full_val_range,
                    val_start, val_centre,
                    px_start, px_centre,
                    dist * 0.5,
                    marker_dist_min,
                    marker_width - 1,
                    draw_label_func,
                    label_dist_min)

        if val_centre <= full_val_range[1]:
            self._fill_markers_interval(
                    painter,
                    draw_marker_func,
                    full_val_range,
                    val_centre, val_stop,
                    px_centre, px_stop,
                    dist * 0.5,
                    marker_dist_min,
                    marker_width - 1,
                    draw_label_func,
                    label_dist_min)

    def _draw_markers_and_labels(self, painter, **params):
        draw_marker         = params['draw_marker']
        draw_label          = params['draw_label']

        val_range           = params['val_range']

        init_marker_width   = params['init_marker_width']
        marker_dist_min     = params['marker_dist_min']
        label_dist_min      = params['label_dist_min']

        val_min, val_max = val_range
        val_range_width = (val_max - val_min)

        def get_pos_norm(value):
            return (value - val_min) / val_range_width

        px_per_whole = self._axis_length / val_range_width

        whole_num_interval = 1
        if px_per_whole > 0:
            whole_num_interval = max(1, int(2**(math.ceil(
                math.log(label_dist_min / px_per_whole, 2)))))

        if marker_dist_min <= px_per_whole:
            min_whole = int(math.floor(val_range[0]))
            max_whole = int(math.ceil(val_range[1]))

            start_px = get_pos_norm(min_whole) * (self._axis_length - 1)

            for i in range(min_whole, max_whole + 1):
                end_px = get_pos_norm(i + 1) * (self._axis_length - 1)

                if (val_min <= i <= val_max) and ((i != 0) or self._draw_zero_marker):
                    draw_marker(painter, start_px, init_marker_width)
                    if i % whole_num_interval == 0:
                        draw_label(painter, start_px, i)

                if i < max_whole:
                    self._fill_markers_interval(
                            painter,
                            draw_marker,
                            val_range,
                            i, i + 1,
                            start_px, end_px,
                            px_per_whole,
                            marker_dist_min,
                            init_marker_width - 1,
                            draw_label,
                            label_dist_min)

                start_px = end_px

        elif px_per_whole > 0:
            # Skipping whole numbers
            marker_interval = int(2**(math.ceil(
                math.log(marker_dist_min / px_per_whole, 2))))

            min_whole = (int(math.floor(val_range[0] / marker_interval)) *
                    marker_interval)
            max_whole = (int(math.ceil(val_range[1] / marker_interval)) *
                    marker_interval)

            start_px = get_pos_norm(min_whole) * (self._axis_length - 1)

            for i in range(min_whole, max_whole + marker_interval, marker_interval):
                end_px = get_pos_norm(i + 1) * (self._axis_length - 1)

                draw_marker(painter, start_px, init_marker_width)

                if i % whole_num_interval == 0:
                    draw_label(painter, start_px, i)

                start_px = end_px

    # Protected callbacks

    def _draw_axis(self, painter):
        raise NotImplementedError


class HorizontalAxisRenderer(AbstractAxisRenderer):

    def __init__(self):
        super().__init__()

        self._x_offset = 0
        self._axis_y_offset_x = 0

    def set_config(self, config, containing_widget):
        super().set_config(config, containing_widget)
        self.set_height(config['axis_x']['height'])

    def set_x_offset(self, x_offset):
        if self._x_offset != x_offset:
            self._x_offset = x_offset
            self.flush_cache()

    def set_y_offset_x(self, y_offset_x):
        if self._axis_y_offset_x != y_offset_x:
            self._axis_y_offset_x = y_offset_x
            self.flush_cache()

    def _draw_axis(self, painter):
        painter.save()

        painter.setTransform(QTransform().translate(self._x_offset, 0))

        # Draw line along the axis
        painter.setPen(self._config['line_colour'])
        painter.drawLine(0, 0, self._axis_length - 1, 0)

        # Marker drawing callback
        def draw_marker(painter, px_centre, marker_width):
            painter.setPen(self._config['line_colour'])
            marker_width = max(marker_width, self._config['axis_x']['marker_min_width'])
            marker_start = marker_width
            painter.drawLine(px_centre, marker_start, px_centre, 1)

        # Label drawing callback
        painter.setFont(self._config['label_font'])
        text_option = QTextOption(Qt.AlignHCenter | Qt.AlignTop)

        def draw_label(painter, px_centre, num):
            painter.setPen(self._config['label_colour'])
            marker_width = self._config['axis_x']['marker_max_width']

            # Text
            numi = int(num)
            text = str(numi) if num == numi else str(round(num, 3))

            # Draw
            width = self._config['axis_x']['label_min_dist']
            rect = QRectF(
                    px_centre - width / 2.0, marker_width - 1,
                    width, self._num_space.height())
            painter.drawText(rect, text, text_option)

        zero_x = (self._axis_y_offset_x + self._config['axis_y']['width'] -
                self._x_offset - 1)

        self._draw_markers_and_labels(
                painter,
                draw_marker=draw_marker,
                draw_label=draw_label,
                val_range=self._val_range,
                init_marker_width=self._config['axis_x']['marker_max_width'],
                zero_px=zero_x,
                axis_length=self._axis_length,
                marker_dist_min=self._config['axis_x']['marker_min_dist'],
                label_dist_min=self._config['axis_x']['label_min_dist'],
                draw_zero_label=self._config['axis_x']['draw_zero_label'])

        painter.restore()


class VerticalAxisRenderer(AbstractAxisRenderer):

    def __init__(self):
        super().__init__()

        self._padding = 0
        self._axis_x_offset_y = 0

    def set_config(self, config, containing_widget):
        super().set_config(config, containing_widget)
        self.set_width(config['axis_y']['width'])

    def set_padding(self, padding):
        if self._padding != padding:
            self._padding = padding
            self.flush_cache()

    def set_x_offset_y(self, x_offset_y):
        if self._axis_x_offset_y != x_offset_y:
            self._axis_x_offset_y = x_offset_y
            self.flush_cache()

    def _draw_axis(self, painter):
        painter.save()

        painter.setTransform(QTransform().translate(0, self._padding))

        axis_width = self._config['axis_y']['width']

        # Draw line along the axis
        painter.setPen(self._config['line_colour'])
        painter.drawLine(axis_width - 1, 0, axis_width - 1, self._axis_length - 1)

        # Ruler location transform
        def ruler_to_y(ruler_px):
            return self._axis_length - ruler_px - 1

        # Marker drawing callback
        def draw_marker(painter, px_centre, marker_width):
            painter.setPen(self._config['line_colour'])
            px_y = ruler_to_y(px_centre)
            marker_width = max(marker_width, self._config['axis_y']['marker_min_width'])
            marker_start = axis_width - marker_width - 1
            painter.drawLine(marker_start, px_y, axis_width - 2, px_y)

        # Label drawing callback
        painter.setFont(self._config['label_font'])
        text_option = QTextOption(Qt.AlignRight | Qt.AlignVCenter)

        def draw_label(painter, px_centre, num):
            painter.setPen(self._config['label_colour'])
            marker_width = self._config['axis_y']['marker_max_width']
            px_y = ruler_to_y(px_centre)

            # Text
            numi = int(num)
            text = str(numi) if num == numi else str(round(num, 3))

            # Draw
            height = self._config['axis_y']['label_min_dist']
            rect = QRectF(
                    0,
                    px_y - self._num_space.height() / 2,
                    self._config['axis_y']['width'] - marker_width - 2,
                    self._num_space.height())
            painter.drawText(rect, text, text_option)

        zero_y = self._axis_length - self._axis_x_offset_y + self._padding - 1

        self._draw_markers_and_labels(
                painter,
                draw_marker=draw_marker,
                draw_label=draw_label,
                val_range=self._val_range,
                init_marker_width=self._config['axis_y']['marker_max_width'],
                zero_px=zero_y,
                axis_length=self._axis_length,
                marker_dist_min=self._config['axis_y']['marker_min_dist'],
                label_dist_min=self._config['axis_y']['label_min_dist'],
                draw_zero_label=self._config['axis_y']['draw_zero_label'])

        painter.restore()


