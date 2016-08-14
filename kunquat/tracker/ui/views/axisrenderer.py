# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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
        self._cache = None
        self._width = 0
        self._height = 0
        self._num_space = QRect()
        self._axis_length = 0
        self._val_range = [0, 0]
        self._get_display_val_min = lambda r: r[0]
        self._get_display_val_max = lambda r: r[1]
        self._draw_zero_marker = True

    def set_config(self, config, containing_widget):
        self._config = config
        self.flush_cache()

        fm = QFontMetrics(self._config['label_font'], containing_widget)
        self._num_space = fm.boundingRect('-00.000')

    def set_display_range_rules(self, get_min_val, get_max_val):
        self._get_display_val_min = get_min_val
        self._get_display_val_max = get_max_val

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

    def get_width(self):
        return self._width

    def set_height(self, height):
        if self._height != height:
            self._height = height
            self.flush_cache()

    def set_axis_length(self, axis_length):
        if self._axis_length != axis_length:
            self._axis_length = axis_length
            self.flush_cache()

    def get_axis_length(self):
        return self._axis_length

    def set_val_range(self, val_range):
        if self._val_range != val_range:
            self._val_range = val_range
            self.flush_cache()

    def get_val_range(self):
        return self._val_range

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
            val_start, val_stop,
            px_start, px_stop,
            dist,
            marker_dist_min,
            marker_width,
            draw_label_func,
            label_dist_min):
        if dist < marker_dist_min * 2:
            return

        val_center = (val_start + val_stop) / 2.0
        px_center = int((px_start + px_stop) / 2)

        draw_marker_func(painter, px_center, marker_width)

        if dist >= label_dist_min * 2:
            draw_label_func(painter, px_center, val_center)

        self._fill_markers_interval(
                painter,
                draw_marker_func,
                val_start, val_center,
                px_start, px_center,
                dist / 2.0,
                marker_dist_min,
                marker_width - 1,
                draw_label_func,
                label_dist_min)

        self._fill_markers_interval(
                painter,
                draw_marker_func,
                val_center, val_stop,
                px_center, px_stop,
                dist / 2.0,
                marker_dist_min,
                marker_width - 1,
                draw_label_func,
                label_dist_min)

    def _draw_markers_and_labels(self, painter, **params):
        draw_marker         = params['draw_marker']
        draw_label          = params['draw_label']

        val_range           = params['val_range']

        init_marker_width   = params['init_marker_width']
        zero_px             = params['zero_px']
        axis_length         = params['axis_length']
        marker_dist_min     = params['marker_dist_min']
        label_dist_min      = params['label_dist_min']
        draw_zero_label     = params['draw_zero_label']

        if self._draw_zero_marker:
            draw_marker(painter, zero_px, init_marker_width)

        if draw_zero_label:
            draw_label(painter, zero_px, 0)

        # Get interval of labels to mark
        display_val_min = self._get_display_val_min(val_range)
        display_val_max = self._get_display_val_max(val_range)
        px_per_whole = axis_length / float(display_val_max - display_val_min)

        whole_num_interval = 1
        if px_per_whole > 0:
            whole_num_interval = max(1, int(2**(math.ceil(
                math.log(label_dist_min / px_per_whole, 2)))))

        if marker_dist_min <= px_per_whole:
            # Positive side
            start_px = zero_px
            pos_len = axis_length - start_px - 1

            for i in range(0, int(math.ceil(display_val_max))):
                end_px = int(zero_px + ((i + 1) * pos_len / display_val_max))

                draw_marker(painter, end_px, init_marker_width)

                end_val = i + 1
                if (end_val < display_val_max) and (end_val % whole_num_interval == 0):
                    draw_label(painter, end_px, end_val)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        i, end_val,
                        start_px, end_px,
                        px_per_whole,
                        marker_dist_min,
                        init_marker_width - 1,
                        draw_label,
                        label_dist_min)

                start_px = end_px

            # Negative side
            start_px = zero_px
            neg_len = start_px

            for i in range(0, -int(math.floor(display_val_min))):
                end_px = int(zero_px - ((i + 1) * neg_len / -display_val_min))

                draw_marker(painter, end_px, init_marker_width)

                end_val = -(i + 1)
                if (end_val > display_val_min) and (-end_val % whole_num_interval == 0):
                    draw_label(painter, end_px, end_val)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        -i, end_val,
                        start_px, end_px,
                        px_per_whole,
                        marker_dist_min,
                        init_marker_width - 1,
                        draw_label,
                        label_dist_min)

                start_px = end_px

        elif px_per_whole > 0:
            # Skipping whole numbers
            whole_marker_interval = int(2**(math.ceil(
                math.log(marker_dist_min / px_per_whole, 2))))

            # Positive side
            start_px = zero_px
            pos_len = axis_length - start_px - 1

            range_stop = display_val_max - whole_marker_interval + 1
            for i in range(0, range_stop, whole_marker_interval):
                end_val = i + whole_marker_interval
                end_px = int(zero_px + end_val * pos_len / display_val_max)

                draw_marker(painter, end_px, init_marker_width)

                if end_val < display_val_max and end_val % whole_num_interval == 0:
                    draw_label(painter, end_px, end_val)

            # Negative side
            start_px = zero_px
            neg_len = start_px

            range_stop = -display_val_min - whole_marker_interval + 1
            for i in range(0, range_stop, whole_marker_interval):
                end_val = -(i + whole_marker_interval)
                end_px = int(zero_px - end_val * neg_len / display_val_min)

                draw_marker(painter, end_px, init_marker_width)

                if end_val > display_val_min and -end_val % whole_num_interval == 0:
                    draw_label(painter, end_px, end_val)

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
        def draw_marker(painter, px_center, marker_width):
            painter.setPen(self._config['line_colour'])
            marker_width = max(marker_width, self._config['axis_x']['marker_min_width'])
            marker_start = marker_width
            painter.drawLine(px_center, marker_start, px_center, 1)

        # Label drawing callback
        painter.setFont(self._config['label_font'])
        text_option = QTextOption(Qt.AlignHCenter | Qt.AlignTop)

        def draw_label(painter, px_center, num):
            painter.setPen(self._config['label_colour'])
            marker_width = self._config['axis_x']['marker_max_width']

            # Text
            numi = int(num)
            text = str(numi) if num == numi else str(round(num, 3))

            # Draw
            width = self._config['axis_x']['label_min_dist']
            rect = QRectF(
                    px_center - width / 2.0, marker_width - 1,
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
        def draw_marker(painter, px_center, marker_width):
            painter.setPen(self._config['line_colour'])
            px_y = ruler_to_y(px_center)
            marker_width = max(marker_width, self._config['axis_y']['marker_min_width'])
            marker_start = axis_width - marker_width - 1
            painter.drawLine(marker_start, px_y, axis_width - 2, px_y)

        # Label drawing callback
        painter.setFont(self._config['label_font'])
        text_option = QTextOption(Qt.AlignRight | Qt.AlignVCenter)

        def draw_label(painter, px_center, num):
            painter.setPen(self._config['label_colour'])
            marker_width = self._config['axis_y']['marker_max_width']
            px_y = ruler_to_y(px_center)

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


