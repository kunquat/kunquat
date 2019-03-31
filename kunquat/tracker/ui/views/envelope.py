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

from copy import deepcopy
from itertools import count, islice
import math
import time

from kunquat.tracker.ui.qt import *

from .axisrenderer import HorizontalAxisRenderer, VerticalAxisRenderer
from .iconbutton import IconButton
from .utils import lerp_val, set_glyph_rel_width, get_scaled_font


_font = QFont(QFont().defaultFamily(), 9, QFont.Bold)
set_glyph_rel_width(_font, QWidget, '23456789' * 8, 50)


AXIS_CONFIG = {
    'axis_x': {
        'height'          : 20,
        'marker_min_dist' : 6,
        'marker_min_width': 2,
        'marker_max_width': 5,
        'label_min_dist'  : 100,
        'draw_zero_label' : False,
    },
    'axis_y': {
        'width'           : 50,
        'marker_min_dist' : 6,
        'marker_min_width': 2,
        'marker_max_width': 5,
        'label_min_dist'  : 50,
        'draw_zero_label' : False,
    },
    'label_font'  : _font,
    'label_colour': QColor(0xcc, 0xcc, 0xcc),
    'line_colour' : QColor(0xcc, 0xcc, 0xcc),
}


DEFAULT_CONFIG = {
    'font'                      : _font,
    'padding'                   : 5,
    'is_square_area'            : False,
    'enable_zoom_x'             : False,
    'bg_colour'                 : QColor(0, 0, 0),
    'line_colour'               : QColor(0x66, 0x88, 0xaa),
    'line_width'                : 1,
    'node_colour'               : QColor(0xee, 0xcc, 0xaa),
    'focused_node_colour'       : QColor(0xff, 0x77, 0x22),
    'focused_node_axis_colour'  : QColor(0xff, 0x77, 0x22, 0x7f),
    'focused_node_axis_width'   : 1,
    'node_size'                 : 7,
    'node_focus_dist_max'       : 5,
    'node_remove_dist_min'      : 200,
    'loop_line_colour'          : QColor(0x77, 0x99, 0xbb),
    'focused_loop_line_colour'  : QColor(0xee, 0xaa, 0x66),
    'loop_line_width'           : 1,
    'loop_line_dash'            : [4, 4],
    'loop_handle_colour'        : QColor(0x88, 0xbb, 0xee),
    'focused_loop_handle_colour': QColor(0xff, 0xaa, 0x55),
    'loop_handle_size'          : 12,
    'loop_handle_focus_dist_max': 14,
    'disabled_colour'           : QColor(0x88, 0x88, 0x88, 0x7f),
}


class EnvelopeScrollBar(QScrollBar):

    _AREA_SPAN = 1000000
    _STEP_SIZE = 40

    def __init__(self, orientation):
        super().__init__(orientation)
        self.set_area(0, 1)

    def set_area(self, area_start_norm, area_end_norm):
        width_norm = area_end_norm - area_start_norm

        old_block = self.blockSignals(True)
        self.setMinimum(0)
        self.setMaximum(max(0, self._AREA_SPAN * (1 - width_norm)))
        self.setPageStep(self._AREA_SPAN * width_norm)
        self.setValue(area_start_norm * self._AREA_SPAN)
        self.blockSignals(old_block)

    def set_vis_area_span(self, vis_area_span):
        width_norm = self.pageStep() / self._AREA_SPAN
        self.setSingleStep(
                max(1, int(self._AREA_SPAN / (self._STEP_SIZE / vis_area_span))))

    def get_area(self):
        width_norm = self.pageStep() / self._AREA_SPAN
        area_start_norm = self.value() / self._AREA_SPAN
        area_end_norm = min(1, area_start_norm + width_norm)
        return (area_start_norm, area_end_norm)


class Envelope(QWidget):

    def __init__(self, init_config={}):
        super().__init__()

        self._zoom_in_x_button = IconButton(flat=True)
        self._zoom_out_x_button = IconButton(flat=True)

        self._toolbar = QToolBar()
        self._area = EnvelopeArea(init_config)

        self._config = DEFAULT_CONFIG.copy()
        self._config.update(init_config)

        if self._config['enable_zoom_x']:
            self._toolbar.setOrientation(Qt.Vertical)
            self._toolbar.addWidget(self._zoom_in_x_button)
            self._toolbar.addWidget(self._zoom_out_x_button)
        else:
            self._toolbar.hide()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(0)
        h.addWidget(self._toolbar)
        h.addWidget(self._area)
        self.setLayout(h)

        self._zoom_in_x_button.clicked.connect(self._zoom_in_x)
        self._zoom_out_x_button.clicked.connect(self._zoom_out_x)
        self._area.get_envelope_view().visRangeNormChanged.connect(
                self._update_zoom_buttons_enabled)

    def set_ui_model(self, ui_model):
        self._zoom_in_x_button.set_ui_model(ui_model)
        self._zoom_out_x_button.set_ui_model(ui_model)

        self._zoom_in_x_button.set_icon('zoom_in')
        self._zoom_out_x_button.set_icon('zoom_out')

        self._update_zoom_buttons_enabled()

    def unregister_updaters(self):
        self._zoom_in_x_button.unregister_updaters()
        self._zoom_out_x_button.unregister_updaters()

    def get_envelope_view(self):
        return self._area.get_envelope_view()

    def update_style(self, style_mgr):
        self._area.update_style(style_mgr)

    def _zoom_in_x(self):
        self._area.get_envelope_view().zoom_in_x()

    def _zoom_out_x(self):
        self._area.get_envelope_view().zoom_out_x()

    def _update_zoom_buttons_enabled(self):
        ev = self.get_envelope_view()

        area_start_norm, area_end_norm = ev.get_vis_area_x_norm()
        self._zoom_out_x_button.setEnabled((area_start_norm, area_end_norm) != (0, 1))

        width = ev.get_vis_area_width()
        self._zoom_in_x_button.setEnabled(width >= 0.001)


class EnvelopeArea(QAbstractScrollArea):

    def __init__(self, init_config={}):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)

        self.setHorizontalScrollBar(EnvelopeScrollBar(Qt.Horizontal))
        self.setVerticalScrollBar(EnvelopeScrollBar(Qt.Vertical))

        self._config = DEFAULT_CONFIG.copy()
        self._config.update(init_config)

        self.setHorizontalScrollBarPolicy(
                Qt.ScrollBarAlwaysOn if self._config['enable_zoom_x']
                else Qt.ScrollBarAlwaysOff)

        self.setViewport(EnvelopeView(init_config))
        self.viewport().setFocusProxy(None)

        self.viewport().visRangeNormChanged.connect(self._update_scrollbars)

    def get_envelope_view(self):
        return self.viewport()

    def update_style(self, style_mgr):
        self.viewport().update_style(style_mgr)

    def _update_scrollbars(self):
        if self._config['enable_zoom_x']:
            ev = self.get_envelope_view()
            area_start_norm, area_end_norm = ev.get_vis_area_x_norm()
            area_width = ev.get_vis_area_width()
            hs = self.horizontalScrollBar()
            hs.set_area(area_start_norm, area_end_norm)
            hs.set_vis_area_span(area_width)

    def paintEvent(self, event):
        self.viewport().paintEvent(event)

    def resizeEvent(self, event):
        self.viewport().resizeEvent(event)

    def scrollContentsBy(self, dx, dy):
        ev = self.get_envelope_view()
        if dx != 0:
            ev.show_x_area_norm(*self.horizontalScrollBar().get_area())
        if dy != 0:
            ev.show_y_area_norm(*self.verticalScrollBar().get_area())

    def mousePressEvent(self, event):
        self.viewport().mousePressEvent(event)

    def mouseMoveEvent(self, event):
        self.viewport().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        self.viewport().mouseReleaseEvent(event)

    def changeEvent(self, event):
        self.viewport().changeEvent(event)


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


class EnvelopeView(QWidget):

    envelopeChanged = Signal(name='envelopeChanged')
    visRangeNormChanged = Signal(name='visRangeNormChanged')

    def __init__(self, init_config={}):
        super().__init__()

        self._range_x = None
        self._range_y = None

        self._vis_range_x = None
        self._vis_range_y = None
        self._full_vis_range_x = None
        self._full_vis_range_y = None

        self._range_adjust_x = (False, False)
        self._range_adjust_y = (False, False)

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

        self._state = STATE_IDLE
        self._moving_index = None
        self._moving_pointer_offset = QPointF(0, 0)
        self._moving_node_vis = None

        self._moving_loop_marker = None

        self._curve_path = None
        self._curve_image = None

        self._axis_x_renderer = HorizontalAxisRenderer()
        self._axis_y_renderer = VerticalAxisRenderer()

        self._config = None
        self._axis_config = None
        self._init_config = init_config
        self._set_configs({}, {})

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setMouseTracking(True)

    def set_node_count_max(self, node_count_max):
        self._node_count_max = node_count_max

    def _get_range_bound(self, max_val):
        max_val_abs = abs(max_val)
        if 0 < max_val_abs < 1:
            range_bound_abs = 2**(math.ceil(math.log(max_val, 2)))
        else:
            range_bound_abs = math.ceil(max_val_abs)
        return signum(max_val) * range_bound_abs

    def set_x_range(self, min_x, max_x):
        assert signum(min_x) != signum(max_x)
        self._range_x = (min_x, max_x)

        new_full_vis_range = self._get_range_bound(min_x), self._get_range_bound(max_x)
        if new_full_vis_range != self._full_vis_range_x:
            self._full_vis_range_x = new_full_vis_range

        if not self._vis_range_x:
            self._vis_range_x = self._full_vis_range_x
            self._axis_x_renderer.flush_cache()

    def set_y_range(self, min_y, max_y):
        assert signum(min_y) != signum(max_y)
        self._range_y = (min_y, max_y)

        new_full_vis_range = self._get_range_bound(min_y), self._get_range_bound(max_y)
        if new_full_vis_range != self._full_vis_range_y:
            self._full_vis_range_y = new_full_vis_range

        if not self._vis_range_y:
            self._vis_range_y = self._full_vis_range_y
            self._axis_y_renderer.flush_cache()

    def show_x_area_norm(self, area_start_norm, area_end_norm):
        val_x_min, val_x_max = self._full_vis_range_x
        new_vis_start_x = lerp_val(val_x_min, val_x_max, area_start_norm)
        new_vis_end_x = lerp_val(val_x_min, val_x_max, area_end_norm)
        self._vis_range_x = new_vis_start_x, new_vis_end_x

        self._flush_vis()
        self.update()

    def show_y_area_norm(self, area_start_norm, area_end_norm):
        pass # TODO

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

        old_vt = self._get_transform_to_vis()
        old_vis_range_x = self._vis_range_x
        old_vis_range_y = self._vis_range_y

        self._nodes = [(a, b) for (a, b) in new_nodes]

        if any(self._range_adjust_x) or any(self._range_adjust_y):
            if self._focused_node:
                # Zoom out so that focused node is visible
                x, y = self._focused_node

                if not self._range_x[0] <= x <= self._range_x[1]:
                    new_range_x_min = min(self._range_x[0], x)
                    new_range_x_max = max(self._range_x[1], x)
                    self._range_x = (new_range_x_min, new_range_x_max)

                    new_full_vis_x_min = self._get_range_bound(new_range_x_min)
                    new_full_vis_x_max = self._get_range_bound(new_range_x_max)
                    self._full_vis_range_x = (new_full_vis_x_min, new_full_vis_x_max)

                while not self._vis_range_x[0] <= x <= self._vis_range_x[1]:
                    self.zoom_out_x()

                if not self._range_y[0] <= y <= self._range_y[1]:
                    new_range_y_min = min(self._range_y[0], y)
                    new_range_y_max = max(self._range_y[1], y)
                    self._range_y = (new_range_y_min, new_range_y_max)

                    new_full_vis_y_min = self._get_range_bound(new_range_y_min)
                    new_full_vis_y_max = self._get_range_bound(new_range_y_max)
                    self._full_vis_range_y = (new_full_vis_y_min, new_full_vis_y_max)

                while not self._vis_range_y[0] <= y <= self._vis_range_y[1]:
                    self.zoom_out_y()

            else:
                # Update all ranges and zoom out full
                min_x, max_x = self._range_x
                min_y, max_y = self._range_y
                for node in self._nodes:
                    x, y = node
                    min_x = min(min_x, x)
                    min_y = min(min_y, y)
                    max_x = max(max_x, x)
                    max_y = max(max_y, y)

                new_range_x = min_x, max_x
                new_range_y = min_y, max_y

                ranges_changed = False

                if new_range_x != self._range_x:
                    ranges_changed = True
                    self._vis_range_x = None
                    self.set_x_range(min_x, max_x)

                if new_range_y != self._range_y:
                    ranges_changed = True
                    self._vis_range_y = None
                    self.set_y_range(min_y, max_y)

                if ranges_changed:
                    self.visRangeNormChanged.emit()
                    self._flush_vis()

            if (self._moving_node_vis and
                    (old_vis_range_x != self._vis_range_x or
                        old_vis_range_y != self._vis_range_y)):
                assert self._focused_node

                # Get new pointer offset from node
                new_vt = self._get_transform_to_vis()
                new_moving_node_vis = new_vt.map(QPointF(*self._focused_node))
                sub_offset = self._moving_node_vis - new_moving_node_vis
                self._moving_node_vis = new_moving_node_vis
                self._moving_pointer_offset -= sub_offset

        self._curve_path = None
        self.update()

        if self._state == STATE_WAITING:
            self._state = STATE_MOVING

    def set_loop_markers(self, new_loop_markers):
        assert len(new_loop_markers) >= 2
        self._loop_markers = new_loop_markers
        self.update()

    def set_loop_enabled(self, enabled):
        if enabled != self._is_loop_enabled:
            self._is_loop_enabled = enabled
            self.update()

    def get_clear_changed(self):
        assert self._nodes_changed or self._loop_markers_changed
        nodes_changed = self._nodes_changed
        loop_markers_changed = self._loop_markers_changed
        self._nodes_changed = []
        self._loop_markers_changed = []
        return nodes_changed, loop_markers_changed

    def _flush_vis(self):
        self._curve_image = None
        self._axis_x_renderer.flush_cache()
        self._axis_y_renderer.flush_cache()

    def zoom_in_x(self):
        vis_min_x, vis_max_x = self._vis_range_x
        range_width = vis_max_x - vis_min_x
        range_centre = (vis_min_x + vis_max_x) * 0.5
        new_range_width = 2**round(math.log(range_width * 0.5, 2))

        if self._full_vis_range_x[0] == 0 and vis_min_x == 0:
            new_vis_min_x = 0
            new_vis_max_x = new_vis_min_x + new_range_width
        elif self._full_vis_range_x[1] == 0 and vis_max_x == 0:
            new_vis_max_x = 0
            new_vis_min_x = new_vis_max_x - new_range_width
        else:
            new_vis_min_x = range_centre - new_range_width * 0.5
            new_vis_max_x = range_centre + new_range_width * 0.5

        self._vis_range_x = new_vis_min_x, new_vis_max_x

        self.visRangeNormChanged.emit()

        self._flush_vis()

        self.update()

    def _get_zoom_out_range(self, cur_range, max_range):
        min_x, max_x = cur_range
        range_span = max_x - min_x
        range_centre = (min_x + max_x) * 0.5
        new_range_span = 2**round(math.log(range_span * 2, 2))

        min_x_bound, max_x_bound = max_range
        max_range_span = max_x_bound - min_x_bound

        if new_range_span < max_range_span:
            new_min_x = range_centre - new_range_span * 0.5
            new_max_x = range_centre + new_range_span * 0.5
            if new_min_x < min_x_bound:
                new_min_x = min_x_bound
                new_max_x = min_x_bound + new_range_span
            elif new_max_x > max_x_bound:
                new_max_x = max_x_bound
                new_min_x = max_x_bound - new_range_span
        else:
            new_min_x, new_max_x = max_range

        return (new_min_x, new_max_x)

    def zoom_out_x(self):
        self._vis_range_x = self._get_zoom_out_range(
                self._vis_range_x, self._full_vis_range_x)

        self.visRangeNormChanged.emit()

        self._flush_vis()
        self.update()

    def zoom_out_y(self):
        self._vis_range_y = self._get_zoom_out_range(
                self._vis_range_y, self._full_vis_range_y)

        self.visRangeNormChanged.emit()

        self._flush_vis()
        self.update()

    def get_vis_area_x_norm(self):
        vis_range_min, vis_range_max = self._vis_range_x
        full_range_min, full_range_max = self._full_vis_range_x
        vis_range_width = vis_range_max - vis_range_min
        full_range_width = full_range_max - full_range_min

        vis_area_width_norm = vis_range_width / full_range_width
        vis_area_start_norm = (vis_range_min - full_range_min) / full_range_width
        vis_area_end_norm = vis_area_start_norm + vis_area_width_norm

        return (min(max(0, vis_area_start_norm), 1), min(max(0, vis_area_end_norm), 1))

    def get_vis_area_width(self):
        vis_range_min, vis_range_max = self._vis_range_x
        return vis_range_max - vis_range_min

    def update_style(self, style_mgr):
        def get_colour(name):
            return QColor(style_mgr.get_style_param(name))

        focused_colour = get_colour('envelope_focus_colour')
        focused_axis_colour = QColor(focused_colour)
        focused_axis_colour.setAlpha(0x7f)

        disabled_colour = QColor(get_colour('bg_colour'))
        disabled_colour.setAlpha(0x7f)

        font = get_scaled_font(style_mgr, 0.8, QFont.Bold)
        set_glyph_rel_width(font, QWidget, '23456789' * 8, 50)

        config = {
            'padding'                   : style_mgr.get_scaled_size(0.5),
            'bg_colour'                 : get_colour('envelope_bg_colour'),
            'line_colour'               : get_colour('envelope_curve_colour'),
            'line_width'                : style_mgr.get_scaled_size(0.1),
            'node_colour'               : get_colour('envelope_node_colour'),
            'focused_node_colour'       : focused_colour,
            'focused_node_axis_colour'  : focused_axis_colour,
            'focused_node_axis_width'   : style_mgr.get_scaled_size(0.1),
            'node_size'                 : style_mgr.get_scaled_size(0.55),
            'node_focus_dist_max'       : style_mgr.get_scaled_size(0.55),
            'node_remove_dist_min'      : style_mgr.get_scaled_size(25),
            'loop_line_colour'          : get_colour('envelope_loop_marker_colour'),
            'focused_loop_line_colour'  : focused_colour,
            'loop_line_width'           : style_mgr.get_scaled_size(0.1),
            'loop_line_dash'            : [style_mgr.get_scaled_size(0.4)] * 2,
            'loop_handle_colour'        : get_colour('envelope_loop_marker_colour'),
            'focused_loop_handle_colour': focused_colour,
            'loop_handle_size'          : style_mgr.get_scaled_size(1.3),
            'loop_handle_focus_dist_max': style_mgr.get_scaled_size(1.5),
            'disabled_colour'           : disabled_colour,
        }

        axis_config = {
            'axis_x': {
                'height'            : style_mgr.get_scaled_size(2),
                'marker_min_dist'   : style_mgr.get_scaled_size(0.3),
                'marker_min_width'  : style_mgr.get_scaled_size(0.3),
                'marker_max_width'  : style_mgr.get_scaled_size(0.6),
                'label_min_dist'    : style_mgr.get_scaled_size(6),
            },
            'axis_y': {
                'width'             : style_mgr.get_scaled_size(5),
                'marker_min_dist'   : style_mgr.get_scaled_size(0.3),
                'marker_min_width'  : style_mgr.get_scaled_size(0.3),
                'marker_max_width'  : style_mgr.get_scaled_size(0.6),
                'label_min_dist'    : style_mgr.get_scaled_size(4),
            },
            'label_font'    : font,
            'label_colour'  : get_colour('envelope_axis_label_colour'),
            'line_colour'   : get_colour('envelope_axis_line_colour'),
        }

        self._set_configs(config, axis_config)
        self.update()

    def _set_configs(self, config, axis_config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(self._init_config)
        self._config.update(config)

        axis_x = axis_config.pop('axis_x', {})
        axis_y = axis_config.pop('axis_y', {})

        def_label_font = AXIS_CONFIG.pop('label_font', None)
        self._axis_config = deepcopy(AXIS_CONFIG)
        self._axis_config['label_font'] = def_label_font
        AXIS_CONFIG['label_font'] = def_label_font

        self._axis_config.update(axis_config)
        self._axis_config['axis_x'].update(axis_x)
        self._axis_config['axis_y'].update(axis_y)
        self._axis_x_renderer.set_config(self._axis_config, self)
        self._axis_y_renderer.set_config(self._axis_config, self)

        self._flush_vis()

    def _get_envelope_transform(self):
        t = QTransform()

        def get_offset_and_scale(vis_range):
            vis_min, vis_max = vis_range
            vis_span = vis_max - vis_min
            scale = 1 / vis_span
            return -vis_min, scale

        offset_x, scale_x = get_offset_and_scale(self._vis_range_x)
        offset_y, scale_y = get_offset_and_scale(self._vis_range_y)

        t = t.scale(scale_x, scale_y).translate(offset_x, offset_y)

        return t

    def _get_envelope_rect(self):
        padding = self._config['padding']

        axis_y_width = self._axis_config['axis_y']['width']
        axis_x_height = self._axis_config['axis_x']['height']

        width_px = self.width() - padding * 2 - axis_y_width
        height_px = self.height() - padding * 2 - axis_x_height

        if self._config['is_square_area']:
            min_dim_px = min(width_px, height_px)
            width_px = height_px = min_dim_px

        return QRect(padding + axis_y_width, padding, width_px, height_px)

    def _get_transform_to_vis(self):
        rect = self._get_envelope_rect()
        area_w = rect.width()
        area_h = rect.height()

        et = self._get_envelope_transform()

        t = et * QTransform().translate(
                rect.x(), rect.y() + area_h - 1).scale(area_w - 1, -area_h + 1)
        return t

    def _get_transform_to_env(self):
        rect = self._get_envelope_rect()

        def get_offset_and_scale(vis_range):
            vis_min, vis_max = vis_range
            vis_span = vis_max - vis_min
            return vis_min, vis_span

        offset_x, scale_x = get_offset_and_scale(self._vis_range_x)
        offset_y, scale_y = get_offset_and_scale(self._vis_range_y)

        t = QTransform().translate(offset_x, offset_y).scale(scale_x, scale_y).scale(
                1 / rect.width(), -1 / rect.height()).translate(
                -rect.x(), -rect.y() - rect.height())

        return t

    def _get_dist_to_node(self, pointer_vis_pos, node_vis_pos):
        px, py = pointer_vis_pos.x(), pointer_vis_pos.y()
        nx, ny = node_vis_pos.x(), node_vis_pos.y()
        return max(abs(px - nx), abs(py - ny))

    def _get_nearest_node_with_dist(self, vt, vis_pos):
        nearest = None
        nearest_dist = float('inf')
        for node in self._nodes:
            node_point = vt.map(QPointF(*node))
            node_dist = self._get_dist_to_node(vis_pos, node_point)
            if node_dist < nearest_dist:
                x, y = node
                min_x, max_x = self._vis_range_x
                min_y, max_y = self._vis_range_y
                if (min_x <= x <= max_x) and (min_y <= y <= max_y):
                    nearest = node
                    nearest_dist = node_dist

        return nearest, nearest_dist

    def _find_focused_node(self, vt, vis_pos):
        nearest, nearest_dist = self._get_nearest_node_with_dist(vt, vis_pos)

        max_dist = self._config['node_focus_dist_max']
        if nearest_dist <= max_dist:
            return nearest

        return None

    def _find_focused_loop_marker(self, vt, vis_pos):
        if not self._loop_markers:
            return None

        pos_x, pos_y = vis_pos.x(), vis_pos.y()

        rect = self._get_envelope_rect()

        start_index = self._loop_markers[0]
        end_index = self._loop_markers[1]

        start_x = vt.map(QPointF(*self._nodes[start_index])).x()
        end_x = vt.map(QPointF(*self._nodes[end_index])).x()
        start_y = rect.y()
        end_y = rect.y() + rect.height() - 1

        dist_max = self._config['loop_handle_focus_dist_max']
        dist_size_diff = dist_max - self._config['loop_handle_size']

        dist_to_start = abs(start_x - pos_x) + abs(start_y - pos_y)
        if (dist_to_start < dist_max) and (pos_y >= start_y - dist_size_diff):
            return 0

        dist_to_end = abs(end_x - pos_x) + abs(end_y - pos_y)
        if (dist_to_end < dist_max) and (pos_y <= end_y + dist_size_diff):
            return 1

        return None

    def _set_focused_node(self, node):
        if node != self._focused_node:
            self._focused_node = node
            self.update()

    def _set_focused_loop_marker(self, marker):
        if marker != self._focused_loop_marker:
            self._focused_loop_marker = marker
            if not self._focused_node:
                self.update()

    def mouseMoveEvent(self, event):
        vt = self._get_transform_to_vis()
        et = self._get_transform_to_env()

        pointer_vis = QPointF(event.x() - 1, event.y() - 1)

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
                node_vis = vt.map(QPointF(*self._focused_node))
                node_offset_vis = node_vis - self._moving_pointer_offset
                if self._get_dist_to_node(pointer_vis, node_offset_vis) >= remove_dist:
                    self._nodes_changed = (
                            self._nodes[:self._moving_index] +
                            self._nodes[self._moving_index + 1:])

                    new_loop_markers = [max(0, m if m < self._moving_index else m - 1)
                            for m in self._loop_markers]
                    if new_loop_markers != self._loop_markers:
                        self._loop_markers_changed = new_loop_markers

                    self.envelopeChanged.emit()

                    self._state = STATE_IDLE
                    self._focused_node = None
                    return

            # Get node bounds
            epsilon = 0.0000001

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
            new_vis = pointer_vis + self._moving_pointer_offset

            et = self._get_transform_to_env()
            new_ep = et.map(new_vis)
            new_x, new_y = new_ep.x(), new_ep.y()
            clamped_x = min(max(min_x, new_x), max_x)
            clamped_y = min(max(min_y, new_y), max_y)

            new_coords = (clamped_x, clamped_y)

            self._nodes_changed = (
                    self._nodes[:self._moving_index] +
                    [new_coords] +
                    self._nodes[self._moving_index + 1:])
            self.envelopeChanged.emit()

            self._focused_node = new_coords

            self._moving_node_vis = vt.map(QPointF(*self._focused_node))

            # Reduce pointer offset if possible
            pointer_offset_x = self._moving_pointer_offset.x()
            pointer_offset_y = self._moving_pointer_offset.y()
            if clamped_x != new_x:
                new_offset_x = self._moving_node_vis.x() - pointer_vis.x()
                if abs(new_offset_x) < abs(pointer_offset_x):
                    pointer_offset_x = new_offset_x
            if clamped_y != new_y:
                new_offset_y = self._moving_node_vis.y() - pointer_vis.y()
                if abs(new_offset_y) < abs(pointer_offset_y):
                    pointer_offset_y = new_offset_y
            self._moving_pointer_offset = QPointF(pointer_offset_x, pointer_offset_y)

        elif self._state == STATE_MOVING_MARKER:
            assert self._focused_loop_marker != None

            pointer_vis_x = pointer_vis.x()

            # Find the nearest node by x coordinate
            nearest_node_index = None
            nearest_dist = float('inf')
            for i, node in enumerate(self._nodes):
                node_vis = vt.map(QPointF(*node))
                node_vis_x = node_vis.x()
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
                self.envelopeChanged.emit()

        elif self._state == STATE_IDLE:
            focused_node = self._find_focused_node(vt, pointer_vis)
            self._set_focused_node(focused_node)

            focused_loop_marker = self._find_focused_loop_marker(vt, pointer_vis)
            self._set_focused_loop_marker(focused_loop_marker)

    def mousePressEvent(self, event):
        if event.buttons() != Qt.LeftButton:
            return

        if self._state != STATE_IDLE:
            return

        vt = self._get_transform_to_vis()
        pointer_vis = QPointF(event.x() - 1, event.y() - 1)

        focused_node = self._find_focused_node(vt, pointer_vis)
        focused_loop_marker = self._find_focused_loop_marker(vt, pointer_vis)

        if focused_node:
            self._state = STATE_MOVING
            self._set_focused_node(focused_node)
            focused_node_vis = vt.map(QPointF(*focused_node))
            self._moving_index = self._nodes.index(focused_node)
            self._moving_pointer_offset = focused_node_vis - pointer_vis

        elif focused_loop_marker != None:
            self._state = STATE_MOVING_MARKER
            self._set_focused_loop_marker(focused_loop_marker)
            self._moving_loop_marker = focused_loop_marker

        elif len(self._nodes) < self._node_count_max:
            et = self._get_transform_to_env()
            new_val_point = et.map(pointer_vis)
            new_val_x = new_val_point.x()
            new_val_y = new_val_point.y()

            epsilon = 0.0000001

            # Get x limits
            min_x = float('-inf')
            max_x = float('inf')

            if not self._range_adjust_x[0]:
                min_x = self._range_x[0]
            if not self._range_adjust_x[1]:
                max_x = self._range_x[1]

            if any(self._first_lock):
                min_x = self._nodes[0][0] + epsilon
            if any(self._last_lock):
                max_x = self._nodes[-1][0] - epsilon

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

                new_loop_markers = [(m if m < insert_pos else m + 1)
                        for m in self._loop_markers]
                if new_loop_markers != self._loop_markers:
                    self._loop_markers_changed = new_loop_markers

                self.envelopeChanged.emit()

                self._state = STATE_WAITING
                self._set_focused_node(new_node)
                self._moving_index = insert_pos
                self._moving_pointer_offset = QPointF(0, 0)

    def mouseReleaseEvent(self, event):
        self._state = STATE_IDLE

        vt = self._get_transform_to_vis()
        pointer_vis = QPointF(event.x() - 1, event.y() - 1)

        self._set_focused_node(self._find_focused_node(vt, pointer_vis))
        self._set_focused_loop_marker(self._find_focused_loop_marker(vt, pointer_vis))

        self._moving_index = None
        self._moving_pointer_offset = QPointF(0, 0)
        self._moving_node_vis = None

    def leaveEvent(self, event):
        self._set_focused_node(None)

    def _draw_envelope_curve(self, painter):
        if not self._nodes:
            return

        painter.save()

        rect = self._get_envelope_rect()
        lw = self._config['line_width']
        image_rect = rect.adjusted(-lw, -lw, lw, lw)

        is_redraw_needed = (not self._curve_image) or (not self._curve_path)

        if not self._curve_image:
            self._curve_image = QImage(
                    image_rect.width(),
                    image_rect.height(),
                    QImage.Format_ARGB32_Premultiplied)

        if not self._curve_path:
            self._curve_path = QPainterPath()
            self._curve_path.moveTo(self._nodes[0][0], self._nodes[0][1])
            for node in self._nodes[1:]:
                self._curve_path.lineTo(node[0], node[1])

        if is_redraw_needed:
            self._curve_image.fill(0)
            pw = rect.width()
            ph = rect.height()

            pp = QPainter(self._curve_image)

            # Test
            """
            pp.setPen(QColor('#fff'))
            pp.drawRect(0, 0, pw - 1, ph - 1)
            """

            lw_half = lw * 0.5
            et = self._get_envelope_transform()
            t = et * QTransform().translate(
                    lw_half + 0.5, lw_half - 0.5 + ph).scale(pw - 1, -ph + 1)
            #t = et * QTransform().translate(0, ph - 1).scale(pw - 1, -ph + 1)
            pp.setTransform(t)

            pen = QPen(self._config['line_colour'])
            pen.setCosmetic(True)
            pen.setWidthF(self._config['line_width'])
            pp.setPen(pen)
            pp.setRenderHint(QPainter.Antialiasing)
            pp.drawPath(self._curve_path)

            pp.end()

        lw_half = lw * 0.5
        painter.translate(lw_half - 0.5, lw_half - 0.5)
        painter.drawImage(image_rect, self._curve_image)

        painter.restore()

    def _draw_focus_axes(self, painter):
        if not self._nodes:
            return

        painter.save()

        rect = self._get_envelope_rect()

        vt = self._get_transform_to_vis()

        painter.setPen(self._config['focused_node_axis_colour'])

        node_point = vt.map(QPointF(*self._focused_node))
        node_x, node_y = int(round(node_point.x())), int(round(node_point.y()))

        painter.drawLine(rect.x(), node_y, rect.x() + rect.width() - 1, node_y)
        painter.drawLine(node_x, rect.y(), node_x, rect.y() + rect.height() - 1)

        painter.restore()

    def _draw_loop_markers(self, painter):
        assert len(self._loop_markers) >= 2

        painter.save()

        def get_line_colour(index):
            normal_colour = self._config['loop_line_colour']
            focused_colour = self._config['focused_loop_line_colour']
            return (focused_colour if (self._focused_loop_marker == index) and
                    no_node_focus else normal_colour)

        def get_handle_colour(index):
            normal_colour = self._config['loop_handle_colour']
            focused_colour = self._config['focused_loop_handle_colour']
            return (focused_colour if (self._focused_loop_marker == index) and
                    no_node_focus else normal_colour)

        rect = self._get_envelope_rect()
        painter.translate(QPoint(0, rect.y()))

        t = self._get_transform_to_vis()

        no_node_focus = (self._focused_node == None)

        # Get x coordinates
        start_index = self._loop_markers[0]
        end_index = self._loop_markers[1]
        start_node_x, _ = self._nodes[start_index]
        end_node_x, _ = self._nodes[end_index]
        start_x = int(round(t.map(QPointF(*self._nodes[start_index])).x()))
        end_x = int(round(t.map(QPointF(*self._nodes[end_index])).x()))

        # Draw marker lines
        pen = QPen()
        pen.setDashPattern(self._config['loop_line_dash'])

        # Make sure the focused line is drawn on top
        x_coords = start_x, end_x
        node_x_coords = start_node_x, end_node_x
        first_line_index = 1 if self._focused_loop_marker == 0 else 0
        second_line_index = 1 - first_line_index

        vis_x_min, vis_x_max = self._vis_range_x

        if vis_x_min <= node_x_coords[first_line_index] <= vis_x_max:
            pen.setColor(get_line_colour(first_line_index))
            painter.setPen(pen)
            painter.drawLine(
                    x_coords[first_line_index], 0,
                    x_coords[first_line_index], rect.height() - 1)

        if vis_x_min <= node_x_coords[second_line_index] <= vis_x_max:
            pen.setColor(get_line_colour(second_line_index))
            painter.setPen(pen)
            painter.drawLine(
                    x_coords[second_line_index], 0,
                    x_coords[second_line_index], rect.height() - 1)

        # Draw marker handles
        painter.setPen(Qt.NoPen)
        handle_size = self._config['loop_handle_size']
        focused = self._focused_loop_marker

        if vis_x_min <= start_node_x <= vis_x_max:
            painter.setBrush(get_handle_colour(0))
            painter.drawConvexPolygon(QPolygon([
                    QPoint(start_x - handle_size + 1, 0),
                    QPoint(start_x + handle_size, 0),
                    QPoint(start_x, handle_size)]))

        if vis_x_min <= end_node_x <= vis_x_max:
            painter.setBrush(get_handle_colour(1))
            painter.drawConvexPolygon(QPolygon([
                    QPoint(end_x - handle_size, rect.height() - 1),
                    QPoint(end_x + handle_size + 1, rect.height() - 1),
                    QPoint(end_x, rect.height() - 1 - handle_size)]))

        painter.restore()

    def _draw_envelope_nodes(self, painter):
        if not self._nodes:
            return

        painter.save()

        t = self._get_transform_to_vis()

        node_size = self._config['node_size']

        min_x, max_x = self._vis_range_x
        min_y, max_y = self._vis_range_y

        for node in self._nodes:
            x, y = node
            if not (min_x <= x <= max_x and min_y <= y <= max_y):
                continue

            point = t.map(QPointF(x, y))
            vis_x, vis_y = int(round(point.x())), int(round(point.y()))

            colour = self._config['node_colour']
            if node == self._focused_node:
                colour = self._config['focused_node_colour']

            painter.fillRect(
                    vis_x - node_size // 2, vis_y - node_size // 2,
                    node_size, node_size,
                    colour)

        painter.restore()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        rect = self._get_envelope_rect()
        if not rect.isValid():
            return

        padding = self._config['padding']

        vt = self._get_transform_to_vis()
        zero_point = vt.map(QPointF(0, 0))
        zero_x, zero_y = int(round(zero_point.x())), int(round(zero_point.y()))

        # Axes
        painter.save()
        painter.translate(QPoint(
            0, min(max(rect.y(), zero_y), rect.y() + rect.height() - 1)))
        self._axis_x_renderer.set_width(self.width())
        self._axis_x_renderer.set_x_offset(rect.x())
        self._axis_x_renderer.set_axis_length(rect.width())
        self._axis_x_renderer.set_val_range(self._vis_range_x)
        self._axis_x_renderer.set_draw_zero_marker_enabled(self._range_y[0] == 0)
        self._axis_x_renderer.render(painter)
        painter.restore()

        painter.save()
        painter.translate(QPoint(
            padding + 1 + min(max(0, zero_x - rect.x()), rect.width() - 1), 0))
        self._axis_y_renderer.set_height(self.height())
        self._axis_y_renderer.set_padding(padding)
        self._axis_y_renderer.set_x_offset_y(rect.y() + rect.height() - 1)
        self._axis_y_renderer.set_axis_length(rect.height())
        self._axis_y_renderer.set_val_range(self._vis_range_y)
        self._axis_y_renderer.set_draw_zero_marker_enabled(self._range_x[0] == 0)
        self._axis_y_renderer.render(painter)
        painter.restore()

        self._draw_envelope_curve(painter)

        if self._focused_node:
            self._draw_focus_axes(painter)

        if self._is_loop_enabled:
            self._draw_loop_markers(painter)

        self._draw_envelope_nodes(painter)

        # Grey out disabled widget
        if not self.isEnabled():
            painter.fillRect(
                    0, 0, self.width(), self.height(), self._config['disabled_colour'])

        end = time.time()
        elapsed = end - start
        #print('Envelope view updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, event):
        self._curve_image = None
        self.update()

    def sizeHint(self):
        return QSize(
                self._axis_config['axis_y']['width'] * 3,
                self._axis_config['axis_x']['height'] * 3)


