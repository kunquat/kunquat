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

from itertools import count, islice, izip
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
        'font'                      : _font,
        'padding'                   : 2,
        'bg_colour'                 : QColor(0, 0, 0),
        'axis_colour'               : QColor(0xcc, 0xcc, 0xcc),
        'line_colour'               : QColor(0x66, 0x88, 0xaa),
        'node_colour'               : QColor(0xee, 0xcc, 0xaa),
        'focused_node_colour'       : QColor(0xff, 0x77, 0x22),
        'node_size'                 : 5,
        'node_focus_dist_max'       : 3,
        'node_remove_dist_min'      : 200,
        'loop_line_colour'          : QColor(0x77, 0x99, 0xbb),
        'loop_line_dash'            : [4, 4],
        'loop_handle_colour'        : QColor(0x88, 0xbb, 0xee),
        'focused_loop_handle_colour': QColor(0xff, 0xaa, 0x55),
        'loop_handle_size'          : 10,
        'loop_handle_focus_dist_max': 11,
    }


def signum(x):
    if x > 0:
        return 1
    elif x < 0:
        return -1
    return 0


STATE_IDLE = 'idle'
STATE_MOVING = 'moving'
STATE_WAITING = 'waiting'
STATE_MOVING_MARKER = 'moving_marker'


class Envelope(QWidget):

    envelopeChanged = pyqtSignal(name='envelopeChanged')

    def __init__(self, config={}):
        QWidget.__init__(self)

        self._range_x = None
        self._range_y = None

        self._range_adjust_x = (False, False)
        self._range_adjust_y = (False, False)

        self._is_square_area = False

        self._nodes = []
        self._nodes_changed = []

        self._loop_markers = []
        self._loop_markers_changed = []
        self._is_loop_enabled = False

        self._node_count_max = 2

        self._first_lock = (False, False)
        self._last_lock = (False, False)

        self._focused_node = None
        self._focused_loop_marker = None

        self._axis_x_cache = None
        self._axis_y_cache = None

        self._ls_cache = {}
        self._ls_update_id = count()

        self._envelope_width = 0
        self._envelope_height = 0
        self._envelope_offset_x = 0

        self._state = STATE_IDLE
        self._moving_index = None
        self._moving_pointer_offset = (0, 0)
        self._moving_node_vis = None

        self._moving_loop_marker = None

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setMouseTracking(True)

    def set_node_count_max(self, node_count_max):
        self._node_count_max = node_count_max

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

    def set_square_area(self, is_square):
        if is_square != self._is_square_area:
            self._is_square_area = is_square

    def set_x_range_adjust(self, allow_neg, allow_pos):
        self._range_adjust_x = (allow_neg, allow_pos)

    def set_y_range_adjust(self, allow_neg, allow_pos):
        self._range_adjust_y = (allow_neg, allow_pos)

    def set_first_lock(self, lock_x, lock_y):
        self._first_lock = (lock_x, lock_y)

    def set_last_lock(self, lock_x, lock_y):
        self._last_lock = (lock_x, lock_y)

    def set_nodes(self, new_nodes):
        assert len(new_nodes) >= 2
        self._nodes = [(a, b) for (a, b) in new_nodes]

        self._check_update_range()

        self.update()

        if self._state == STATE_WAITING:
            self._state = STATE_MOVING

    def set_loop_markers(self, new_loop_markers):
        assert len(new_loop_markers) >= 2
        self._loop_markers = new_loop_markers
        self.update()

    def set_loop_enabled(self, is_enabled):
        self._is_loop_enabled = is_enabled

    def get_clear_changed(self):
        assert self._nodes_changed or self._loop_markers_changed
        nodes_changed = self._nodes_changed
        loop_markers_changed = self._loop_markers_changed
        self._nodes_changed = []
        self._loop_markers_changed = []
        return nodes_changed, loop_markers_changed

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        self._axis_x_cache = None
        self._axis_y_cache = None
        self._ls_cache = {}

    def _check_update_range(self):
        if not (any(self._range_adjust_x) or any(self._range_adjust_y)):
            return

        min_x, min_y = [float('inf')] * 2
        max_x, max_y = [float('-inf')] * 2
        for node in self._nodes:
            x, y = node
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)

        range_x_min, range_x_max = self._range_x
        range_y_min, range_y_max = self._range_y

        if self._range_adjust_x[0] and (min_x < self._range_x[0]):
            range_x_min = int(math.floor(min_x))
        if self._range_adjust_x[1] and (max_x > self._range_x[1]):
            range_x_max = int(math.ceil(max_x))

        if self._range_adjust_y[0] and (min_y < self._range_y[0]):
            range_y_min = int(math.floor(min_y))
        if self._range_adjust_y[1] and (max_y > self._range_y[1]):
            range_y_max = int(math.ceil(max_y))

        new_range_x = range_x_min, range_x_max
        new_range_y = range_y_min, range_y_max

        if (new_range_x != self._range_x) or (new_range_y != self._range_y):
            self._range_x = new_range_x
            self._range_y = new_range_y
            self._update_display_area()

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

    def _get_display_val_max(self, val_range):
        return int(math.ceil(val_range[1]))

    def _get_display_val_min(self, val_range):
        return int(math.floor(val_range[0]))

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
        display_val_max = self._get_display_val_max(val_range)
        display_val_min = self._get_display_val_min(val_range)
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

    def _get_ls_coords(self, nodes):
        return izip(nodes, islice(nodes, 1, None))

    def _get_zero_x_envelope(self):
        return (self._axis_y_offset_x + self._config['axis_y']['width'] -
                self._envelope_offset_x - 1)

    def _get_zero_y_envelope(self):
        return self._axis_x_offset_y - self._config['padding']

    def _get_coords_vis(self, val_coords):
        val_x, val_y = val_coords

        zero_x = self._get_zero_x_envelope()
        zero_y = self._get_zero_y_envelope()

        # Get vis x
        vis_x = zero_x

        if val_x >= 0:
            val_x_max = self._get_display_val_max(self._range_x)
            pos_width_vis = self._envelope_width - zero_x - 1

            if val_x_max != 0:
                vis_x = zero_x + pos_width_vis * val_x / val_x_max
        else:
            val_x_min = self._get_display_val_min(self._range_x)
            neg_width_vis = zero_x

            if val_x_min != 0:
                vis_x = zero_x - neg_width_vis * val_x / val_x_min

        # Get vis y
        vis_y = zero_y

        if val_y >= 0:
            val_y_max = self._get_display_val_max(self._range_y)
            pos_height_vis = zero_y

            if val_y_max != 0:
                vis_y = zero_y - pos_height_vis * val_y / val_y_max
        else:
            val_y_min = self._get_display_val_min(self._range_y)
            neg_height_vis = self._envelope_height - zero_y - 1

            if val_y_min != 0:
                vis_y = zero_y + neg_height_vis * val_y / val_y_min

        return int(vis_x), int(vis_y)

    def _draw_envelope_curve(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(self._envelope_offset_x, padding))

        # Test
        #painter.setPen(QColor(0xff, 0, 0))
        #painter.drawRect(0, 0, self._envelope_width - 1, self._envelope_height - 1)

        cur_update_id = self._ls_update_id.next()

        new_ls_count = 0

        for coords in self._get_ls_coords(self._nodes):
            if coords not in self._ls_cache:
                a, b = coords
                vis_a = self._get_coords_vis(a)
                vis_b = self._get_coords_vis(b)
                ls = LineSegment(vis_a, vis_b)
                ls.set_colour(self._config['line_colour'])
                ls.draw_line()
                self._ls_cache[coords] = ls
                new_ls_count += 1

            ls = self._ls_cache[coords]
            ls.copy_line(painter)
            ls.set_update_id(cur_update_id)

        # Remove obsolete entries from cache
        obsolete_keys = []
        for k, v in self._ls_cache.iteritems():
            if v.get_update_id() != cur_update_id:
                obsolete_keys.append(k)

        for k in obsolete_keys:
            del self._ls_cache[k]

        #if new_ls_count > 0:
        #    print('{} new line segment{}'.format(
        #        new_ls_count, '' if new_ls_count == 1 else 's'))

        painter.restore()

    def _draw_envelope_nodes(self, painter):
        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(self._envelope_offset_x, padding))

        node_size = self._config['node_size']

        for node in self._nodes:
            pos = self._get_coords_vis(node)
            x, y = pos

            colour = self._config['node_colour']
            if node == self._focused_node:
                colour = self._config['focused_node_colour']

            painter.fillRect(
                    x - node_size / 2, y - node_size / 2,
                    node_size, node_size,
                    colour)

        painter.restore()

    def _draw_loop_markers(self, painter):
        assert len(self._loop_markers) >= 2

        painter.save()

        padding = self._config['padding']
        painter.setTransform(QTransform().translate(self._envelope_offset_x, padding))

        # Get x coordinates
        start_index = self._loop_markers[0]
        end_index = self._loop_markers[1]
        start_x, _ = self._get_coords_vis(self._nodes[start_index])
        end_x, _ = self._get_coords_vis(self._nodes[end_index])

        # Draw marker lines
        pen = QPen(self._config['loop_line_colour'])
        pen.setDashPattern(self._config['loop_line_dash'])
        painter.setPen(pen)
        painter.drawLine(start_x, 0, start_x, self._envelope_height - 1)
        painter.drawLine(end_x, 0, end_x, self._envelope_height - 1)

        # Draw marker handles
        painter.setPen(Qt.NoPen)
        normal_colour = self._config['loop_handle_colour']
        focused_colour = self._config['focused_loop_handle_colour']
        handle_size = self._config['loop_handle_size']
        no_node = (self._focused_node == None)
        focused = self._focused_loop_marker

        painter.setBrush(focused_colour if (focused == 0 and no_node) else normal_colour)
        painter.drawConvexPolygon(
                QPoint(start_x - handle_size + 1, 0),
                QPoint(start_x + handle_size, 0),
                QPoint(start_x, handle_size))

        painter.setBrush(focused_colour if (focused == 1 and no_node) else normal_colour)
        painter.drawConvexPolygon(
                QPoint(end_x - handle_size, self._envelope_height),
                QPoint(end_x + handle_size + 1, self._envelope_height),
                QPoint(end_x, self._envelope_height - handle_size))

        painter.restore()

    def _get_node_val_from_env_vis(self, widget_point):
        widget_x, widget_y = widget_point

        zero_x = self._get_zero_x_envelope()
        zero_y = self._get_zero_y_envelope()

        # Get envelope x
        def get_pos_width_vis():
            return self._envelope_width - zero_x - 1
        def get_neg_width_vis():
            return zero_x

        if widget_x >= zero_x:
            pos_width_vis = get_pos_width_vis()
            val_x_max = self._get_display_val_max(self._range_x)
            if pos_width_vis <= 0:
                pos_width_vis = get_neg_width_vis()
                val_x_max = -self._get_display_val_min(self._range_x)

            val_x = (widget_x - zero_x) * val_x_max / float(pos_width_vis)
        else:
            neg_width_vis = get_neg_width_vis()
            val_x_min = self._get_display_val_min(self._range_x)
            if neg_width_vis <= 0:
                neg_width_vis = get_pos_width_vis()
                val_x_min = -self._get_display_val_max(self._range_x)

            val_x = (zero_x - widget_x) * val_x_min / float(neg_width_vis)

        # Get envelope y
        def get_pos_height_vis():
            return zero_y
        def get_neg_height_vis():
            return self._envelope_height - zero_y - 1

        if widget_y <= zero_y:
            pos_height_vis = get_pos_height_vis()
            val_y_max = self._get_display_val_max(self._range_y)
            if pos_height_vis <= 0:
                pos_height_vis = get_neg_height_vis()
                val_y_max = -self._get_display_val_min(self._range_y)

            val_y = (zero_y - widget_y) * val_y_max / float(pos_height_vis)
        else:
            neg_height_vis = get_neg_height_vis()
            val_y_min = self._get_display_val_min(self._range_y)
            if neg_height_vis <= 0:
                neg_height_vis = get_pos_height_vis()
                val_y_min = -self._get_display_val_max(self._range_y)

            val_y = (widget_y - zero_y) * val_y_min / float(neg_height_vis)

        return val_x, val_y

    def leaveEvent(self, event):
        self._set_focused_node(None)

    def _set_focused_node(self, node):
        if node != self._focused_node:
            self._focused_node = node
            self.update()

    def _set_focused_loop_marker(self, marker):
        if marker != self._focused_loop_marker:
            self._focused_loop_marker = marker
            if not self._focused_node:
                self.update()

    def _get_dist_to_node(self, pointer_vis, node_vis):
        return max(abs(pointer_vis[0] - node_vis[0]), abs(pointer_vis[1] - node_vis[1]))

    def _get_nearest_node_with_dist(self, pos_vis):
        nearest = None
        nearest_dist = float('inf')
        for node in self._nodes:
            node_vis = self._get_coords_vis(node)
            node_dist = self._get_dist_to_node(pos_vis, node_vis)
            if node_dist < nearest_dist:
                nearest = node
                nearest_dist = node_dist

        return nearest, nearest_dist

    def _find_focused_node(self, pos_vis):
        nearest, nearest_dist = self._get_nearest_node_with_dist(pos_vis)

        max_dist = self._config['node_focus_dist_max']
        if nearest_dist <= max_dist:
            return nearest

        return None

    def _find_focused_loop_marker(self, pos_vis):
        pos_x, pos_y = pos_vis

        start_index = self._loop_markers[0]
        end_index = self._loop_markers[1]

        start_x, _ = self._get_coords_vis(self._nodes[start_index])
        end_x, _ = self._get_coords_vis(self._nodes[end_index])
        start_y = 0
        end_y = self._envelope_height - 1

        dist_max = self._config['loop_handle_focus_dist_max']
        dist_size_diff = dist_max - self._config['loop_handle_size']

        dist_to_start = abs(start_x - pos_x) + abs(start_y - pos_y)
        if (dist_to_start < dist_max) and (pos_y >= start_y - dist_size_diff):
            return 0

        dist_to_end = abs(end_x - pos_x) + abs(end_y - pos_y)
        if (dist_to_end < dist_max) and (pos_y <= end_y + dist_size_diff):
            return 1

        return None

    def _get_env_vis_coords_from_widget(self, widget_x, widget_y):
        offset_widget_x = widget_x - self._envelope_offset_x
        offset_widget_y = widget_y - self._config['padding']

        return offset_widget_x, offset_widget_y

    def mouseMoveEvent(self, event):
        pointer_vis = self._get_env_vis_coords_from_widget(event.x(), event.y())

        if self._state == STATE_MOVING:
            assert self._focused_node != None

            # Check if we have moved far enough to remove the node
            is_node_locked = False
            if self._moving_index == 0:
                is_node_locked = any(self._first_lock)
            elif self._moving_index == len(self._nodes) - 1:
                is_node_locked = any(self._last_lock)

            if (len(self._nodes) > 2) and (not is_node_locked):
                remove_dist = self._config['node_remove_dist_min']
                node_vis = self._get_coords_vis(self._focused_node)
                node_offset_vis = (
                        node_vis[0] - self._moving_pointer_offset[0],
                        node_vis[1] - self._moving_pointer_offset[1])
                if self._get_dist_to_node(pointer_vis, node_offset_vis) >= remove_dist:
                    self._nodes_changed = (
                            self._nodes[:self._moving_index] +
                            self._nodes[self._moving_index + 1:])
                    QObject.emit(self, SIGNAL('envelopeChanged()'))

                    self._state = STATE_IDLE
                    self._focused_node = None
                    return

            # Get node bounds
            epsilon = 0.001

            min_x, min_y = [float('-inf')] * 2
            max_x, max_y = [float('inf')] * 2

            if not self._range_adjust_x[0]:
                min_x = self._range_x[0]
            if not self._range_adjust_x[1]:
                max_x = self._range_x[1]

            if not self._range_adjust_y[0]:
                min_y = self._range_y[0]
            if not self._range_adjust_y[1]:
                max_y = self._range_y[1]

            if self._moving_index == 0:
                # First node
                if self._first_lock[0]:
                    min_x = max_x = self._focused_node[0]
                else:
                    next_node = self._nodes[1]
                    max_x = min(max_x, next_node[0] - epsilon)

                if self._first_lock[1]:
                    min_y = max_y = self._focused_node[1]

            elif self._moving_index == len(self._nodes) - 1:
                # Last node
                if self._last_lock[0]:
                    min_x = max_x = self._focused_node[0]
                else:
                    prev_node = self._nodes[self._moving_index - 1]
                    min_x = max(min_x, prev_node[0] + epsilon)

                if self._last_lock[1]:
                    min_y = max_y = self._focused_node[1]

            else:
                # Middle node
                prev_node = self._nodes[self._moving_index - 1]
                min_x = max(min_x, prev_node[0] + epsilon)

                next_node = self._nodes[self._moving_index + 1]
                max_x = min(max_x, next_node[0] - epsilon)

            # Get new coordinates
            new_x_vis = pointer_vis[0] + self._moving_pointer_offset[0]
            new_y_vis = pointer_vis[1] + self._moving_pointer_offset[1]

            new_x, new_y = self._get_node_val_from_env_vis((new_x_vis, new_y_vis))
            new_x = min(max(min_x, new_x), max_x)
            new_y = min(max(min_y, new_y), max_y)

            new_coords = (new_x, new_y)

            self._nodes_changed = (
                    self._nodes[:self._moving_index] +
                    [new_coords] +
                    self._nodes[self._moving_index + 1:])
            QObject.emit(self, SIGNAL('envelopeChanged()'))

            self._focused_node = new_coords

            self._moving_node_vis = self._get_coords_vis(self._focused_node)

        elif self._state == STATE_MOVING_MARKER:
            assert self._focused_loop_marker != None

            pointer_vis_x, _ = pointer_vis

            # Find the nearest node by x coordinate
            nearest_node_index = None
            nearest_dist = float('inf')
            for i, node in enumerate(self._nodes):
                node_vis = self._get_coords_vis(node)
                node_vis_x, _ = node_vis
                dist_x = abs(pointer_vis_x - node_vis_x)
                if dist_x < nearest_dist:
                    nearest_node_index = i
                    nearest_dist = dist_x

            # Clamp to boundaries
            new_loop_markers = list(self._loop_markers)
            if self._focused_loop_marker == 0:
                new_loop_markers[0] = min(max(
                    0, nearest_node_index), new_loop_markers[1])
            else:
                new_loop_markers[1] = min(max(
                    new_loop_markers[0], nearest_node_index), len(self._nodes) - 1)

            if new_loop_markers != self._loop_markers:
                self._loop_markers_changed = new_loop_markers
                QObject.emit(self, SIGNAL('envelopeChanged()'))

        elif self._state == STATE_IDLE:
            focused_node = self._find_focused_node(pointer_vis)
            self._set_focused_node(focused_node)

            focused_loop_marker = self._find_focused_loop_marker(pointer_vis)
            self._set_focused_loop_marker(focused_loop_marker)

    def mousePressEvent(self, event):
        if event.buttons() != Qt.LeftButton:
            return

        if self._state != STATE_IDLE:
            return

        pointer_vis = self._get_env_vis_coords_from_widget(event.x(), event.y())
        focused_node = self._find_focused_node(pointer_vis)
        focused_loop_marker = self._find_focused_loop_marker(pointer_vis)

        if focused_node:
            self._state = STATE_MOVING
            self._set_focused_node(focused_node)
            focused_node_vis = self._get_coords_vis(focused_node)
            self._moving_index = self._nodes.index(focused_node)
            self._moving_pointer_offset = (
                    focused_node_vis[0] - pointer_vis[0],
                    focused_node_vis[1] - pointer_vis[1])

        elif focused_loop_marker != None:
            self._state = STATE_MOVING_MARKER
            self._set_focused_loop_marker(focused_loop_marker)
            self._moving_loop_marker = focused_loop_marker

        elif len(self._nodes) < self._node_count_max:
            new_val_x, new_val_y = self._get_node_val_from_env_vis(pointer_vis)

            epsilon = 0.001

            # Get x limits
            min_x = float('-inf')
            max_x = float('inf')

            if not self._range_adjust_x[0]:
                min_x = self._range_x[0]
            if not self._range_adjust_x[1]:
                max_x = self._range_x[1]

            insert_pos = 0
            for i, node in enumerate(self._nodes):
                insert_pos = i
                cur_x, _ = node
                if cur_x < new_val_x:
                    min_x = cur_x + epsilon
                else:
                    max_x = cur_x - epsilon
                    break
            else:
                insert_pos = len(self._nodes)

            # Get y limits
            min_y = float('-inf')
            max_y = float('inf')

            if not self._range_adjust_y[0]:
                min_y = self._range_y[0]
            if not self._range_adjust_y[1]:
                max_y = self._range_y[1]

            if min_x <= max_x and min_y <= max_y:
                new_val_x = min(max(min_x, new_val_x), max_x)
                new_val_y = min(max(min_y, new_val_y), max_y)
                new_node = (new_val_x, new_val_y)

                self._nodes_changed = (
                        self._nodes[:insert_pos] +
                        [new_node] +
                        self._nodes[insert_pos:])
                QObject.emit(self, SIGNAL('envelopeChanged()'))

                self._state = STATE_WAITING
                self._set_focused_node(new_node)
                self._moving_index = insert_pos
                self._moving_pointer_offset = (0, 0)

    def mouseReleaseEvent(self, event):
        self._state = STATE_IDLE
        self._set_focused_node(None)
        self._set_focused_loop_marker(None)

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        # Test
        #painter.setPen(QColor(0xff, 0, 0xff))
        #painter.drawRect(QRect(0, 0, self.width() - 1, self.height() - 1))

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

        if self._is_loop_enabled:
            self._draw_loop_markers(painter)

        self._draw_envelope_nodes(painter)

        end = time.time()
        elapsed = end - start
        print('Envelope view updated in {:.2f} ms'.format(elapsed * 1000))

    def _get_display_x_range_width(self):
        r = self._range_x
        return self._get_display_val_max(r) - self._get_display_val_min(r)

    def _get_display_y_range_height(self):
        r = self._range_y
        return self._get_display_val_max(r) - self._get_display_val_min(r)

    def _update_display_area(self):
        # Get total area available
        padding = self._config['padding']

        available_width_px = self.width() - padding * 2
        available_height_px = self.height() - padding * 2

        x_range_width = self._get_display_x_range_width()
        y_range_height = self._get_display_y_range_height()

        axis_y_width = self._config['axis_y']['width']
        axis_x_height = self._config['axis_x']['height']

        axis_y_offset_x_px = 0
        axis_x_offset_y_px = available_height_px - axis_x_height

        envelope_offset_x_px = 0
        envelope_width_px = available_width_px
        envelope_height_px = available_height_px

        # Get left border of the envelope
        x_left_width = abs(self._get_display_val_min(self._range_x))
        x_left_width_px = int(round(available_width_px * x_left_width / x_range_width))
        if x_left_width_px < axis_y_width:
            extra_space = axis_y_width - x_left_width_px
            envelope_offset_x_px = extra_space
            envelope_width_px -= extra_space
        elif x_left_width_px > axis_y_width:
            axis_y_offset_x_px = x_left_width_px - axis_y_width

        # Get bottom border of the envelope
        y_down_height = abs(self._get_display_val_min(self._range_y))
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

        # Make the envelope area square if configured accordingly
        if self._is_square_area:
            if envelope_width_px > envelope_height_px:
                envelope_width_px = envelope_height_px
                new_left_width_px = int(round(
                    envelope_width_px * x_left_width / x_range_width))
                fix_shift = x_left_width_px - new_left_width_px
                envelope_offset_x_px += fix_shift
                left_shift = min(envelope_offset_x_px, axis_y_offset_x_px)
                envelope_offset_x_px -= left_shift
                axis_y_offset_x_px -= left_shift
            elif envelope_height_px > envelope_width_px:
                envelope_height_px = envelope_width_px
                new_down_height_px = int(round(
                    envelope_height_px * y_down_height / y_range_height))
                axis_x_offset_y_px = envelope_height_px - new_down_height_px

        # Clear caches
        self._axis_x_cache = None
        self._axis_y_cache = None
        self._ls_cache = {}

        envelope_offset_x_shift = (
                (envelope_offset_x_px + padding) - self._envelope_offset_x)

        # Set final values
        self._axis_y_offset_x = axis_y_offset_x_px + padding
        self._axis_x_offset_y = axis_x_offset_y_px + padding
        self._envelope_offset_x = envelope_offset_x_px + padding
        self._envelope_width = envelope_width_px
        self._envelope_height = envelope_height_px

        if self._state == STATE_MOVING:
            assert self._focused_node
            assert self._moving_node_vis

            # Get new pointer offset from node
            new_coords_vis = self._get_coords_vis(self._focused_node)
            sub_offset_x = self._moving_node_vis[0] - new_coords_vis[0]
            sub_offset_y = self._moving_node_vis[1] - new_coords_vis[1]
            mp_offset_x, mp_offset_y = self._moving_pointer_offset
            self._moving_pointer_offset = (
                    mp_offset_x - sub_offset_x + envelope_offset_x_shift,
                    mp_offset_y - sub_offset_y)

    def resizeEvent(self, event):
        self._update_display_area()
        self.update()


class LineSegment():

    def __init__(self, from_point, to_point):
        from_x, from_y = from_point
        to_x, to_y = to_point
        assert from_x <= to_x

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


