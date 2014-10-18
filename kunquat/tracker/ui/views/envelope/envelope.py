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


_font = QFont(QFont().defaultFamily(), 9)
_font.setWeight(QFont.Bold)


DEFAULT_CONFIG = {
        'axis_x': {
            'height': 20,
            'line_min_dist' : 6,
            'line_max_width': 5,
            'line_min_width': 2,
            'num_min_dist'  : 100,
            },
        'axis_y': {
            'width'         : 50,
            'line_min_dist' : 6,
            'line_max_width': 5,
            'line_min_width': 2,
            'num_min_dist'  : 50,
            },
        'font'       : _font,
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

        self._axis_x_cache = None
        self._axis_y_cache = None

        self._envelope_width = 0
        self._envelope_height = 0

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_x_range(self, min_x, max_x):
        assert signum(min_x) != signum(max_x)
        new_range = (min_x, max_x)
        if new_range != self._range_x:
            self._range_x = new_range
            self._axis_x_cache = None

    def set_y_range(self, min_y, max_y):
        assert signum(min_y) != signum(max_y)
        new_range = (min_y, max_y)
        if new_range != self._range_y:
            self._range_y = new_range
            self._axis_y_cache = None

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        self._axis_x_cache = None
        self._axis_y_cache = None

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
            draw_marker_func,
            val_start, val_stop,
            px_start, px_stop,
            dist,
            marker_dist_min,
            marker_width,
            draw_num_func,
            num_dist_min):
        if dist < marker_dist_min * 2:
            return

        val_center = (val_start + val_stop) / 2.0
        px_center = int((px_start + px_stop) / 2)

        draw_marker_func(painter, px_center, marker_width)

        if dist >= num_dist_min * 2:
            draw_num_func(painter, px_center, val_center)

        self._fill_markers_interval(
                painter,
                draw_marker_func,
                val_start, val_center,
                px_start, px_center,
                dist / 2.0,
                marker_dist_min,
                marker_width - 1,
                draw_num_func,
                num_dist_min)

        self._fill_markers_interval(
                painter,
                draw_marker_func,
                val_center, val_stop,
                px_center, px_stop,
                dist / 2.0,
                marker_dist_min,
                marker_width - 1,
                draw_num_func,
                num_dist_min)

    def _draw_markers_and_numbers(self, painter, **params):
        # Get params
        draw_marker         = params['draw_marker']
        draw_number         = params['draw_number']

        val_range           = params['val_range']

        init_marker_width   = params['init_marker_width']
        zero_px             = params['zero_px']
        draw_zero_marker    = params['draw_zero_marker']
        axis_len            = params['axis_len']
        marker_dist_min     = params['marker_dist_min']
        num_dist_min        = params['num_dist_min']

        if draw_zero_marker:
            draw_marker(painter, zero_px, init_marker_width)

        # Get interval of whole number values to mark
        display_val_max = int(math.ceil(val_range[1]))
        display_val_min = int(math.floor(val_range[0]))
        px_per_whole = axis_len // (display_val_max - display_val_min)

        whole_num_interval = 1
        if px_per_whole >= 1:
            whole_num_interval = max(1, int(2**(math.ceil(
                math.log(num_dist_min / float(px_per_whole), 2)))))

        if marker_dist_min <= px_per_whole:
            # Positive side
            start_px = zero_px
            pos_len = axis_len - start_px - 1

            for i in xrange(0, display_val_max):
                end_px = int(zero_px + ((i + 1) * pos_len / display_val_max))

                draw_marker(painter, end_px, init_marker_width)

                end_val = i + 1
                if (end_val < display_val_max) and (end_val % whole_num_interval == 0):
                    draw_number(painter, end_px, end_val)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        i, end_val,
                        start_px, end_px,
                        px_per_whole,
                        marker_dist_min,
                        init_marker_width - 1,
                        draw_number,
                        num_dist_min)

                start_px = end_px

            # Negative side
            start_px = zero_px
            neg_len = start_px

            for i in xrange(0, -display_val_min):
                end_px = int(zero_px - ((i + 1) * neg_len / -display_val_min))

                draw_marker(painter, end_px, init_marker_width)

                end_val = -(i + 1)
                if (end_val > display_val_min) and (-end_val % whole_num_interval == 0):
                    draw_number(painter, end_px, end_val)

                self._fill_markers_interval(
                        painter,
                        draw_marker,
                        -i, end_val,
                        start_px, end_px,
                        px_per_whole,
                        marker_dist_min,
                        init_marker_width - 1,
                        draw_number,
                        num_dist_min)

                start_px = end_px

        elif px_per_whole >= 1:
            # Skipping whole numbers
            whole_marker_interval = int(2**(math.ceil(
                math.log(marker_dist_min / float(px_per_whole), 2))))

            # Positive side
            start_px = zero_px
            pos_len = axis_len - start_px - 1

            range_stop = display_val_max - whole_marker_interval + 1
            for i in xrange(0, range_stop, whole_marker_interval):
                end_val = i + whole_marker_interval
                end_px = int(zero_px + end_val * pos_len / display_val_max)

                draw_marker(painter, end_px, init_marker_width)

                if end_val < display_val_max and end_val % whole_num_interval == 0:
                    draw_number(painter, end_px, end_val)

            # Negative side
            start_px = zero_px
            neg_len = start_px

            range_stop = -display_val_min - whole_marker_interval + 1
            for i in xrange(0, range_stop, whole_marker_interval):
                end_val = -(i + whole_marker_interval)
                end_px = int(zero_px - end_val * neg_len / display_val_min)

                draw_marker(painter, end_px, init_marker_width)

                if end_val > display_val_min and -end_val % whole_num_interval == 0:
                    draw_number(painter, end_px, end_val)

    def _draw_axis_x(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(
            self._envelope_offset_x, 0)) #self._axis_x_offset_y))

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

        # Number drawing callback
        painter.setFont(self._config['font'])
        text_option = QTextOption(Qt.AlignHCenter | Qt.AlignTop)
        fm = QFontMetrics(self._config['font'], self)
        num_space = fm.boundingRect('-00.000')

        def draw_number(painter, px_center, num):
            marker_width = self._config['axis_x']['line_max_width']

            # Text
            numi = int(num)
            text = str(numi) if num == numi else str(round(num, 3))

            # Draw
            width = self._config['axis_x']['num_min_dist']
            rect = QRectF(
                    px_center - width / 2.0, marker_width - 1,
                    width, num_space.height())
            painter.drawText(rect, text, text_option)

        zero_x = (self._axis_y_offset_x + self._config['axis_y']['width'] -
                self._envelope_offset_x - 1)

        self._draw_markers_and_numbers(
                painter,
                draw_marker=draw_marker,
                draw_number=draw_number,
                val_range=self._range_x,
                init_marker_width=self._config['axis_x']['line_max_width'],
                zero_px=zero_x,
                draw_zero_marker=self._range_y[0] == 0,
                axis_len=self._envelope_width,
                marker_dist_min=self._config['axis_x']['line_min_dist'],
                num_dist_min=self._config['axis_x']['num_min_dist'])

        painter.restore()

    def _draw_axis_y(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(0, padding))

        axis_width = self._config['axis_y']['width']

        # Test
        #painter.setPen(QColor(0, 0xff, 0))
        #painter.drawRect(0, 0, self._config['axis_y']['width'] - 1, self._envelope_height - 1)

        # Draw line along the axis
        painter.setPen(self._config['axis_colour'])
        painter.drawLine(axis_width - 1, 0, axis_width - 1, self._envelope_height - 1)

        # Ruler location transform
        def ruler_to_y(ruler_px):
            return self._envelope_height - ruler_px - 1

        # Marker drawing callback
        def draw_marker(painter, px_center, marker_width):
            px_y = ruler_to_y(px_center)
            marker_width = max(marker_width, self._config['axis_y']['line_min_width'])
            marker_start = axis_width - marker_width - 1
            painter.drawLine(marker_start, px_y, axis_width - 2, px_y)

        # Number drawing callback
        painter.setFont(self._config['font'])
        text_option = QTextOption(Qt.AlignRight | Qt.AlignVCenter)
        fm = QFontMetrics(self._config['font'], self)
        num_space = fm.boundingRect('-00.000')

        def draw_number(painter, px_center, num):
            marker_width = self._config['axis_y']['line_max_width']
            px_y = ruler_to_y(px_center)

            # Text
            numi = int(num)
            text = str(numi) if num == numi else str(round(num, 3))

            # Draw
            height = self._config['axis_y']['num_min_dist']
            rect = QRectF(
                    0,
                    px_y - num_space.height() / 2,
                    self._config['axis_y']['width'] - marker_width - 2,
                    num_space.height())
            painter.drawText(rect, text, text_option)

        zero_y = self._envelope_height - self._axis_x_offset_y + padding - 1

        self._draw_markers_and_numbers(
                painter,
                draw_marker=draw_marker,
                draw_number=draw_number,
                val_range=self._range_y,
                init_marker_width=self._config['axis_y']['line_max_width'],
                zero_px=zero_y,
                draw_zero_marker=self._range_x[0] == 0,
                axis_len=self._envelope_height,
                marker_dist_min=self._config['axis_y']['line_min_dist'],
                num_dist_min=self._config['axis_y']['num_min_dist'])

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

        padding = self._config['padding']

        # Axes
        if not self._axis_x_cache:
            self._axis_x_cache = QImage(
                    self.width(),
                    self._config['axis_x']['height'],
                    QImage.Format_ARGB32)
            self._axis_x_cache.fill(0)
            axis_painter = QPainter(self._axis_x_cache)
            self._draw_axis_x(axis_painter)
        painter.drawImage(QPoint(0, self._axis_x_offset_y), self._axis_x_cache)

        if not self._axis_y_cache:
            self._axis_y_cache = QImage(
                    self._config['axis_y']['width'],
                    self.height(),
                    QImage.Format_ARGB32)
            self._axis_y_cache.fill(0)
            axis_painter = QPainter(self._axis_y_cache)
            self._draw_axis_y(axis_painter)
        painter.drawImage(QPoint(self._axis_y_offset_x, 0), self._axis_y_cache)

        # Graph
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

        # Clear axis caches if out of date
        if self._envelope_width != envelope_width_px:
            self._axis_x_cache = None
        if self._envelope_height != envelope_height_px:
            self._axis_y_cache = None

        # Set final values
        self._axis_y_offset_x = axis_y_offset_x_px + padding
        self._axis_x_offset_y = axis_x_offset_y_px + padding
        self._envelope_offset_x = envelope_offset_x_px + padding
        self._envelope_width = envelope_width_px
        self._envelope_height = envelope_height_px

        self.update()


