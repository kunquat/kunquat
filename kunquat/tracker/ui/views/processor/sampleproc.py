# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy
import math
import time

from kunquat.tracker.ui.qt import *

import kunquat.tracker.config as config
from kunquat.tracker.ui.model.procparams.sampleparams import SampleImportError
from kunquat.tracker.ui.views.audiounit.hitselector import HitSelector
from kunquat.tracker.ui.views.axisrenderer import HorizontalAxisRenderer, VerticalAxisRenderer
from kunquat.tracker.ui.views.confirmdialog import ConfirmDialog
from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from kunquat.tracker.ui.views.utils import lerp_val, set_glyph_rel_width, get_scaled_font
from kunquat.tracker.ui.views.varprecspinbox import VarPrecSpinBox
from .processorupdater import ProcessorUpdater
from .prociconbutton import ProcessorIconButton
from .prockeyboardmapper import ProcessorKeyboardMapper
from .sampleview import SampleView
from . import utils


class SampleProc(QTabWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Sample synthesis'

    def __init__(self):
        super().__init__()

        self._note_map_editor = NoteMapEditor()
        self._hit_map_editor = HitMapEditor()
        self._samples = Samples()

        self.add_to_updaters(
                self._note_map_editor, self._hit_map_editor, self._samples)

        self.addTab(self._note_map_editor, 'Note map')
        self.addTab(self._hit_map_editor, 'Hit map')
        self.addTab(self._samples, 'Samples')


class NoteMapEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._note_map = NoteMap()
        self._note_map_entry = NoteMapEntry()

        self.add_to_updaters(self._note_map, self._note_map_entry)

        h = QHBoxLayout()
        h.setContentsMargins(4, 4, 4, 4)
        h.setSpacing(4)
        h.addWidget(self._note_map, 1)
        h.addWidget(self._note_map_entry, 2)
        self.setLayout(h)


class RandomListMap(QWidget, ProcessorUpdater):

    _DEFAULT_CONFIG = {
        'padding'                  : 5,
        'bg_colour'                : QColor(0, 0, 0),
        'point_colour'             : QColor(0xee, 0xcc, 0xaa),
        'focused_point_colour'     : QColor(0xff, 0x77, 0x22),
        'focused_point_axis_colour': QColor(0xff, 0x77, 0x22, 0x7f),
        'point_size'               : 7,
        'point_focus_dist_max'     : 8,
        'selected_highlight_colour': QColor(0xff, 0xff, 0xdd),
        'selected_highlight_size'  : 11,
        'selected_highlight_width' : 2,
        'move_snap_dist'           : 15,
        'remove_dist_min'          : 200,
    }

    _FONT = QFont(QFont().defaultFamily(), 9, QFont.Bold)
    set_glyph_rel_width(_FONT, QWidget, '23456789' * 8, 50)

    _AXIS_CONFIG = {
        'axis_x': {
            'height'          : 20,
            'marker_min_dist' : 6,
            'marker_min_width': 2,
            'marker_max_width': 5,
            'label_min_dist'  : 30,
            'draw_zero_label' : True,
        },
        'axis_y': {
            'width'           : 50,
            'marker_min_dist' : 6,
            'marker_min_width': 2,
            'marker_max_width': 5,
            'label_min_dist'  : 50,
            'draw_zero_label' : True,
        },
        'label_font'  : _FONT,
        'label_colour': QColor(0xcc, 0xcc, 0xcc),
        'line_colour' : QColor(0xcc, 0xcc, 0xcc),
    }

    _STATE_IDLE = 'idle'
    _STATE_MOVING = 'moving'

    def __init__(self):
        super().__init__()
        self._axis_x_renderer = HorizontalAxisRenderer()
        self._axis_x_renderer.set_config(self._AXIS_CONFIG.copy(), self)
        self._axis_x_renderer.set_val_range([-36, 0])

        self._axis_y_renderer = VerticalAxisRenderer()
        self._axis_y_renderer.set_config(self._AXIS_CONFIG.copy(), self)
        y_range = [-6000, 6000] if self._has_pitch_axis() else [0, 0]
        self._axis_y_renderer.set_val_range(y_range)

        self._focused_point = None

        self._state = self._STATE_IDLE

        self._is_start_snapping_active = False
        self._moving_pointer_offset = (0, 0)

        self._config = {}
        self._axis_config = {}
        self._set_configs({}, {})

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setMouseTracking(True)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def _on_setup(self):
        for signal in self._get_update_signals():
            self.register_action(signal, self.update)
        self.register_action('signal_style_changed', self._update_style)

        self._update_style()

    def _set_configs(self, config, axis_config):
        self._config = self._DEFAULT_CONFIG.copy()
        self._config.update(config)

        axis_x = axis_config.pop('axis_x', {})
        axis_y = axis_config.pop('axis_y', {})

        def_font = self._AXIS_CONFIG.pop('label_font', None)
        final_axis_config = deepcopy(self._AXIS_CONFIG)
        self._AXIS_CONFIG['label_font'] = def_font
        final_axis_config['label_font'] = def_font

        final_axis_config.update(axis_config)
        final_axis_config['axis_x'].update(axis_x)
        final_axis_config['axis_y'].update(axis_y)

        if not self._has_pitch_axis():
            padding = self._config['padding']
            x_axis_height = final_axis_config['axis_x']['height']
            self.setFixedHeight(padding * 2 + x_axis_height)

        self._axis_config = final_axis_config

        self._axis_x_renderer.set_config(final_axis_config, self)
        self._axis_y_renderer.set_config(final_axis_config, self)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        if not style_mgr.is_custom_style_enabled():
            self._set_configs({}, {})
            self.update()
            return

        def get_colour(param):
            return QColor(style_mgr.get_style_param(param))

        focus_colour = get_colour('sample_map_focus_colour')
        focus_axis_colour = QColor(focus_colour)
        focus_axis_colour.setAlpha(0x7f)

        font = get_scaled_font(style_mgr, 0.8, QFont.Bold)
        set_glyph_rel_width(font, QWidget, '23456789' * 8, 50)

        config = {
            'padding'               : style_mgr.get_scaled_size_param('large_padding'),
            'bg_colour'             : get_colour('sample_map_bg_colour'),
            'point_colour'          : get_colour('sample_map_point_colour'),
            'focused_point_colour'  : focus_colour,
            'focused_point_axis_colour' : focus_axis_colour,
            'point_size'                : style_mgr.get_scaled_size(0.7),
            'point_size_focus_dist_max' : style_mgr.get_scaled_size(0.9),
            'selected_highlight_colour' : get_colour('sample_map_selected_colour'),
            'selected_highlight_size'   : style_mgr.get_scaled_size(1.1),
            'selected_highlight_width'  : style_mgr.get_scaled_size(0.2),
            'move_snap_dist'            : style_mgr.get_scaled_size(1.6),
            'remove_dist_min'           : style_mgr.get_scaled_size(25),
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
            'label_colour'  : get_colour('sample_map_axis_label_colour'),
            'line_colour'   : get_colour('sample_map_axis_line_colour'),
        }

        self._set_configs(config, axis_config)
        self.update()

    def _get_area_offset(self):
        padding = self._config['padding']
        offset_x = padding + self._axis_y_renderer.get_width() - 1
        offset_y = padding
        return offset_x, offset_y

    def _map_range(self, val, src_range, target_range):
        if src_range[0] == src_range[1]:
            return target_range[0]
        start_diff = val - src_range[0]
        pos_norm = (val - src_range[0]) / float(src_range[1] - src_range[0])
        return lerp_val(target_range[0], target_range[1], min(max(0, pos_norm), 1))

    def _get_vis_coords(self, point):
        cents, dB = point

        cents_range = self._axis_y_renderer.get_val_range()
        y_range = max(0, self._axis_y_renderer.get_axis_length() - 1), 0
        area_y = self._map_range(cents, cents_range, y_range)

        dB_range = self._axis_x_renderer.get_val_range()
        x_range = 0, self._axis_x_renderer.get_axis_length() - 1
        area_x = self._map_range(dB, dB_range, x_range)

        offset_x, offset_y = self._get_area_offset()
        x = area_x + offset_x
        y = area_y + offset_y

        return x, y

    def _get_point_coords(self, vis_coords):
        abs_x, abs_y = vis_coords
        offset_x, offset_y = self._get_area_offset()
        x, y = abs_x - offset_x, abs_y - offset_y

        y_range = max(0, self._axis_y_renderer.get_axis_length() - 1), 0
        cents_range = self._axis_y_renderer.get_val_range()
        point_y = self._map_range(y, y_range, cents_range)

        x_range = 0, self._axis_x_renderer.get_axis_length() - 1
        dB_range = self._axis_x_renderer.get_val_range()
        point_x = self._map_range(x, x_range, dB_range)

        return [point_y, point_x]

    def _coords_dist(self, a, b):
        ax, ay = a
        bx, by = b
        dx, dy = ax - bx, ay - by
        return math.sqrt((dx * dx) + (dy * dy))

    def _get_nearest_point_with_dist(self, x, y):
        nearest_dist = float('inf')
        nearest_point = None
        for point in self._get_all_points():
            dist = self._coords_dist(self._get_vis_coords(point), (x, y))
            if dist < nearest_dist:
                nearest_point = point
                nearest_dist = dist

        return nearest_point, nearest_dist

    def mouseMoveEvent(self, event):
        if self._state == self._STATE_IDLE:
            point, dist = self._get_nearest_point_with_dist(event.x() - 1, event.y() - 1)
            if dist <= self._config['point_focus_dist_max']:
                self._focused_point = point
            else:
                self._focused_point = None
            self.update()
        elif self._state == self._STATE_MOVING:
            point = self._get_selected_point()
            if point:
                point_vis_coords = self._get_vis_coords(point)
                adjusted_x = event.x() - self._moving_pointer_offset[0]
                adjusted_y = event.y() - self._moving_pointer_offset[1]
                dist = self._coords_dist((adjusted_x, adjusted_y), point_vis_coords)

                remove_dist_min = self._config['remove_dist_min']
                keep_area_x = [-remove_dist_min, self.width() + remove_dist_min]
                keep_area_y = [-remove_dist_min, self.height() + remove_dist_min]
                if not ((keep_area_x[0] <= event.x() <= keep_area_x[1]) and
                        (keep_area_y[0] <= event.y() <= keep_area_y[1])):
                    self._remove_point(point)
                    self._set_selected_point(None)
                    self._focused_point = None
                    self._state = self._STATE_IDLE
                    self._updater.signal_update(self._get_selection_signal_type())
                    return

                if self._is_start_snapping_active:
                    if dist >= self._config['move_snap_dist']:
                        self._is_start_snapping_active = False

                if not self._is_start_snapping_active:
                    new_point = self._get_point_coords((adjusted_x, adjusted_y))
                    if new_point not in self._get_all_points():
                        self._move_point(point, new_point)
                        self._set_selected_point(new_point)
                        self._focused_point = new_point
                        self._updater.signal_update(self._get_move_signal_type())

            else:
                self._state = self._STATE_IDLE

    def mousePressEvent(self, event):
        if self._state == self._STATE_IDLE:
            x, y = event.x(), event.y()
            point, dist = self._get_nearest_point_with_dist(x - 1, y - 1)
            if dist <= self._config['point_focus_dist_max']:
                self._set_selected_point(point)

                self._state = self._STATE_MOVING
                self._is_start_snapping_active = True
                point_vis = self._get_vis_coords(point)
                self._moving_pointer_offset = (x - point_vis[0], y - point_vis[1])

                self._updater.signal_update(self._get_selection_signal_type())
            else:
                new_point = self._get_point_coords((x, y))
                pitch_range = self._axis_y_renderer.get_val_range()
                force_range = self._axis_x_renderer.get_val_range()
                if (pitch_range[0] <= new_point[0] <= pitch_range[1] and
                        force_range[0] <= new_point[1] <= force_range[1] and
                        new_point != point):
                    self._add_point(new_point)
                    self._set_selected_point(new_point)

                    self._state = self._STATE_MOVING
                    self._is_start_snapping_active = True
                    self._moving_pointer_offset = (0, 0)

                    self._updater.signal_update(self._get_selection_signal_type())

    def mouseReleaseEvent(self, event):
        self._state = self._STATE_IDLE

    def leaveEvent(self, event):
        self._focused_point = None
        self.update()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        padding = self._config['padding']

        # Axes
        axis_x_height = self._axis_config['axis_x']['height']
        axis_x_top = self.height() - padding - axis_x_height
        axis_y_width = self._axis_config['axis_y']['width']
        axis_x_length = self.width() - axis_y_width - (padding * 2) + 1
        axis_y_length = self.height() - axis_x_height - (padding * 2) + 1

        painter.setTransform(QTransform().translate(0, axis_x_top))
        self._axis_x_renderer.set_width(self.width())
        self._axis_x_renderer.set_x_offset(padding + axis_y_width - 1)
        self._axis_x_renderer.set_axis_length(axis_x_length)
        self._axis_x_renderer.render(painter)

        if self._has_pitch_axis():
            painter.setTransform(QTransform().translate(padding, 0))
            self._axis_y_renderer.set_height(self.height())
            self._axis_y_renderer.set_padding(padding)
            self._axis_y_renderer.set_x_offset_y(axis_x_top - 1)
            self._axis_y_renderer.set_axis_length(axis_y_length)
            self._axis_y_renderer.render(painter)

        # Note map points
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setPen(Qt.NoPen)
        painter.setTransform(QTransform())
        point_size = self._config['point_size']
        point_offset = -(point_size // 2)
        selected_point = self._get_selected_point()
        selected_found = False
        for point in self._get_all_points():
            if point == selected_point:
                selected_found = True
            if point == self._focused_point:
                painter.setBrush(self._config['focused_point_colour'])
            else:
                painter.setBrush(self._config['point_colour'])
            x, y = self._get_vis_coords(point)
            rect = QRect(x + point_offset, y + point_offset, point_size, point_size)
            painter.drawEllipse(rect)

        if selected_found:
            painter.save()
            pen = QPen(self._config['selected_highlight_colour'])
            pen.setWidth(self._config['selected_highlight_width'])
            painter.setPen(pen)
            painter.setBrush(Qt.NoBrush)

            highlight_size = self._config['selected_highlight_size']
            highlight_offset = -(highlight_size // 2)
            x, y = self._get_vis_coords(selected_point)
            painter.translate(highlight_offset, highlight_offset)
            painter.drawEllipse(QRect(x, y, highlight_size, highlight_size))

            painter.restore()

        end = time.time()
        elapsed = end - start
        #print('Note map view updated in {:.2f} ms'.format(elapsed * 1000))

    def minimumSizeHint(self):
        return QSize(200, 200)

    # Protected callbacks

    def _has_pitch_axis(self):
        raise NotImplementedError

    def _get_selection_signal_type(self):
        raise NotImplementedError

    def _get_move_signal_type(self):
        raise NotImplementedError

    def _get_update_signals(self):
        raise NotImplementedError

    def _get_all_points(self):
        raise NotImplementedError

    def _get_selected_point(self):
        raise NotImplementedError

    def _set_selected_point(self, point):
        raise NotImplementedError

    def _add_point(self, point):
        raise NotImplementedError

    def _move_point(self, old_point, new_point):
        raise NotImplementedError

    def _remove_point(self, point):
        raise NotImplementedError


class NoteMap(RandomListMap):

    def __init__(self):
        super().__init__()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _has_pitch_axis(self):
        return True

    def _get_selection_signal_type(self):
        return 'signal_sample_note_map_selection_{}'.format(self._proc_id)

    def _get_move_signal_type(self):
        return 'signal_sample_note_map_move_{}'.format(self._proc_id)

    def _get_update_signals(self):
        return set([self._get_selection_signal_type(), self._get_move_signal_type()])

    def _get_all_points(self):
        sample_params = self._get_sample_params()
        return sample_params.get_note_map_points()

    def _get_selected_point(self):
        sample_params = self._get_sample_params()
        return sample_params.get_selected_note_map_point()

    def _set_selected_point(self, point):
        sample_params = self._get_sample_params()
        sample_params.set_selected_note_map_point(point)

    def _add_point(self, point):
        sample_params = self._get_sample_params()
        sample_params.add_note_map_point(point)

    def _move_point(self, old_point, new_point):
        sample_params = self._get_sample_params()
        sample_params.move_note_map_point(old_point, new_point)

    def _remove_point(self, point):
        sample_params = self._get_sample_params()
        sample_params.remove_note_map_point(point)


class TightLabel(QLabel):

    def __init__(self, text):
        super().__init__(text)
        self.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)


class NoteMapEntry(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self.setEnabled(False)

        self._pitch = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._pitch.setRange(-6000, 6000)
        self._force = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._force.setRange(-36, 0)

        self._random_list = NoteRandomList()

        self.add_to_updaters(self._random_list)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(TightLabel('Pitch:'))
        h.addWidget(self._pitch)
        h.addSpacing(2)
        h.addWidget(TightLabel('Force:'))
        h.addWidget(self._force)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addLayout(h)
        v.addWidget(self._random_list)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action(self._get_selection_signal_type(), self._update_all)
        self.register_action(self._get_move_signal_type(), self._update_all)

        self._pitch.valueChanged.connect(self._move)
        self._force.valueChanged.connect(self._move)

        self._update_all()

    def _get_selection_signal_type(self):
        return 'signal_sample_note_map_selection_{}'.format(self._proc_id)

    def _get_move_signal_type(self):
        return 'signal_sample_note_map_move_{}'.format(self._proc_id)

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        sample_params = self._get_sample_params()
        selected_point = sample_params.get_selected_note_map_point()
        if selected_point not in sample_params.get_note_map_points():
            self.setEnabled(False)
            return

        self.setEnabled(True)

        new_pitch, new_force = selected_point

        old_block = self._pitch.blockSignals(True)
        if self._pitch.value() != new_pitch:
            self._pitch.setValue(new_pitch)
        self._pitch.blockSignals(old_block)

        old_block = self._force.blockSignals(True)
        if self._force.value() != new_force:
            self._force.setValue(new_force)
        self._force.blockSignals(old_block)

    def _move(self, dummy):
        sample_params = self._get_sample_params()
        selected_point = sample_params.get_selected_note_map_point()
        if selected_point:
            new_point = [self._pitch.value(), self._force.value()]
            if new_point not in sample_params.get_note_map_points():
                sample_params.move_note_map_point(selected_point, new_point)
                sample_params.set_selected_note_map_point(new_point)
                self._updater.signal_update(self._get_move_signal_type())


class RandomList(EditorList, ProcessorUpdater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        for signal in self._get_update_signals():
            self.register_action(signal, self._update_all)

        self._update_all()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _get_callback_info(self):
        cb_info = {
            'get_map_points'             : self._get_map_points,
            'get_selected_map_point'     : self._get_selected_map_point,
            'get_model_random_list'      : self._get_model_random_list,
            'get_selection_signal_type'  : self._get_selection_signal_type,
            'get_random_list_signal_type': self._get_random_list_signal_type,
            'get_update_signals'         : self._get_update_signals,
        }
        return cb_info

    def _make_adder_widget(self):
        self._adder = RandomEntryAdder(self._get_callback_info())
        self.add_to_updaters(self._adder)
        return self._adder

    def _get_updated_editor_count(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            return random_list.get_length()
        return 0

    def _make_editor_widget(self, index):
        editor = RandomEntryEditor(self._get_callback_info(), index)
        self.add_to_updaters(editor)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)

    def _update_all(self):
        self.update_list()

        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            self._adder.setVisible(not random_list.is_full())
        else:
            self._adder.setVisible(False)

        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        self._adder.setEnabled(len(sample_params.get_sample_ids()) > 0)

    # Protected callbacks

    def _get_map_points(self):
        raise NotImplementedError

    def _get_selected_map_point(self):
        raise NotImplementedError

    def _get_model_random_list(self):
        raise NotImplementedError

    def _get_selection_signal_type(self):
        raise NotImplementedError

    def _get_random_list_signal_type(self):
        raise NotImplementedError

    def _get_update_signals(self):
        raise NotImplementedError


class RandomEntryAdder(QPushButton, ProcessorUpdater):

    def __init__(self, cb_info):
        super().__init__('Add sample entry')
        self._get_map_points = cb_info['get_map_points']
        self._get_selected_map_point = cb_info['get_selected_map_point']
        self._get_model_random_list = cb_info['get_model_random_list']
        self._get_update_signal_type = cb_info['get_random_list_signal_type']

    def _on_setup(self):
        self.clicked.connect(self._add_entry)

    def _add_entry(self):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.add_entry()
            self._updater.signal_update(self._get_update_signal_type())


class RandomEntryEditor(QWidget, ProcessorUpdater):

    def __init__(self, cb_info, index):
        super().__init__()
        self._get_map_points = cb_info['get_map_points']
        self._get_selected_map_point = cb_info['get_selected_map_point']
        self._get_model_random_list = cb_info['get_model_random_list']
        self._get_random_list_signal_type = cb_info['get_random_list_signal_type']
        self._get_update_signals = cb_info['get_update_signals']

        self._index = index

        self._sample_selector = KqtComboBox()
        self._sample_selector.setSizeAdjustPolicy(QComboBox.AdjustToContents)

        self._pitch_shift = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._pitch_shift.setRange(-6000, 6000)

        self._volume_shift = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._volume_shift.setRange(-64, 64)

        self._remove_button = ProcessorIconButton()

        self.add_to_updaters(self._remove_button)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(self._sample_selector)
        h.addSpacing(2)
        h.addWidget(TightLabel('Pitch shift:'))
        h.addWidget(self._pitch_shift)
        h.addSpacing(2)
        h.addWidget(TightLabel('Volume shift:'))
        h.addWidget(self._volume_shift)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def _on_setup(self):
        for signal in self._get_update_signals():
            self.register_action(signal, self._update_all)

        self._remove_button.set_icon('delete_small')

        style_mgr = self._ui_model.get_style_manager()
        self._remove_button.set_sizes(
                style_mgr.get_style_param('list_button_size'),
                style_mgr.get_style_param('list_button_padding'))

        self._sample_selector.currentIndexChanged.connect(self._change_sample)
        self._pitch_shift.valueChanged.connect(self._change_pitch_shift)
        self._volume_shift.valueChanged.connect(self._change_volume_shift)
        self._remove_button.clicked.connect(self._remove_entry)

        self._update_all()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        sample_params = self._get_sample_params()

        sample_ids = sample_params.get_sample_ids()

        point = self._get_selected_map_point()
        cur_sample_id = None
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            if self._index >= random_list.get_length():
                # We are being removed
                return
            cur_sample_id = random_list.get_sample_id(self._index)

        # Update sample selector contents
        sample_info = sorted([
            (sid, sample_params.get_sample_name(sid)) for sid in sample_ids])

        old_block = self._sample_selector.blockSignals(True)
        self._sample_selector.setEnabled(len(sample_ids) > 0)
        items = []
        cur_index = -1
        for i, info in enumerate(sample_info):
            sample_id, sample_name = info
            vis_name = sample_name or '-'
            items.append((vis_name, sample_id))
            if sample_id == cur_sample_id:
                cur_index = i
        self._sample_selector.set_items(items)
        self._sample_selector.setCurrentIndex(cur_index)
        self._sample_selector.model().sort(0)
        self._sample_selector.blockSignals(old_block)

        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)

            # Update pitch shift
            old_block = self._pitch_shift.blockSignals(True)
            new_pitch_shift = random_list.get_cents_offset(self._index)
            if new_pitch_shift != self._pitch_shift.value():
                self._pitch_shift.setValue(new_pitch_shift)
            self._pitch_shift.blockSignals(old_block)

            # Update volume shift
            old_block = self._volume_shift.blockSignals(True)
            new_volume_shift = random_list.get_volume_adjust(self._index)
            if new_volume_shift != self._volume_shift.value():
                self._volume_shift.setValue(new_volume_shift)
            self._volume_shift.blockSignals(old_block)

    def _change_sample(self, item_index):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            sample_id = self._sample_selector.itemData(item_index)
            random_list = self._get_model_random_list(point)
            random_list.set_sample_id(self._index, sample_id)
            self._updater.signal_update(self._get_random_list_signal_type())

    def _change_pitch_shift(self, value):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.set_cents_offset(self._index, value)
            self._updater.signal_update(self._get_random_list_signal_type())

    def _change_volume_shift(self, value):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.set_volume_adjust(self._index, value)
            self._updater.signal_update(self._get_random_list_signal_type())

    def _remove_entry(self):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.remove_entry(self._index)
            self._updater.signal_update(self._get_random_list_signal_type())


class NoteRandomList(RandomList):

    def __init__(self):
        super().__init__()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_map_points(self):
        return self._get_sample_params().get_note_map_points()

    def _get_selected_map_point(self):
        return self._get_sample_params().get_selected_note_map_point()

    def _get_model_random_list(self, point):
        return self._get_sample_params().get_note_map_random_list(point)

    def _get_selection_signal_type(self):
        return 'signal_sample_note_map_selection_{}'.format(self._proc_id)

    def _get_random_list_signal_type(self):
        return 'signal_sample_note_map_random_list_{}'.format(self._proc_id)

    def _get_update_signals(self):
        return set([
            self._get_selection_signal_type(),
            self._get_random_list_signal_type(),
            'signal_sample_format_{}'.format(self._proc_id)])


class HitMapEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._hit_selector = SampleHitSelector()
        self._hit_map = HitMap()
        self._hit_map_entry = HitMapEntry()

        self.add_to_updaters(self._hit_selector, self._hit_map, self._hit_map_entry)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(2)
        v.addWidget(self._hit_selector)
        v.addWidget(self._hit_map)
        v.addWidget(self._hit_map_entry)
        self.setLayout(v)


class SampleHitSelector(HitSelector, ProcessorUpdater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self.update_contents)
        self.register_action(self._get_hit_update_signal_type(), self.update_contents)

        self.create_layout(self._ui_model.get_typewriter_manager())
        self.update_contents()

        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _get_update_signal_type(self):
        return 'signal_sample_hit_map_hit_selection_{}'.format(self._proc_id)

    def _get_hit_update_signal_type(self):
        return 'signal_hit_{}'.format(self._au_id)

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_selected_hit_info(self):
        sample_params = self._get_sample_params()
        return sample_params.get_selected_hit_info()

    def _set_selected_hit_info(self, hit_info):
        sample_params = self._get_sample_params()
        sample_params.set_selected_hit_info(hit_info)
        self._updater.signal_update(self._get_update_signal_type())

    def _get_hit_name(self, index):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        hit = au.get_hit(index)
        return hit.get_name()

    def _update_style(self):
        self.update_style(self._ui_model.get_style_manager())


class HitMap(RandomListMap):

    def __init__(self):
        super().__init__()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _has_pitch_axis(self):
        return False

    def _get_selection_signal_type(self):
        return 'signal_sample_hit_map_selection_{}'.format(self._proc_id)

    def _get_move_signal_type(self):
        return 'signal_sample_hit_map_move_{}'.format(self._proc_id)

    def _get_update_signals(self):
        return set([
            self._get_selection_signal_type(),
            self._get_move_signal_type(),
            'signal_sample_hit_map_hit_selection_{}'.format(self._proc_id)])

    def _get_all_points(self):
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        return [[0, f] for f in sample_params.get_hit_map_forces(hit_info)]

    def _get_selected_point(self):
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        force = sample_params.get_selected_hit_map_force()
        if force == None:
            return None
        return [0, force]

    def _set_selected_point(self, point):
        sample_params = self._get_sample_params()
        if not point:
            sample_params.set_selected_hit_map_force(None)
            return
        _, force = point
        sample_params.set_selected_hit_map_force(force)

    def _add_point(self, point):
        _, force = point
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        sample_params.add_hit_map_point(hit_info, force)

    def _move_point(self, old_point, new_point):
        _, old_force = old_point
        _, new_force = new_point
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        sample_params.move_hit_map_point(hit_info, old_force, new_force)

    def _remove_point(self, point):
        _, force = point
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        sample_params.remove_hit_map_point(hit_info, force)


class HitMapEntry(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self.setEnabled(False)

        self._force = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._force.setRange(-36, 0)

        self._random_list = HitRandomList()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(TightLabel('Force:'))
        h.addWidget(self._force)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addLayout(h)
        v.addWidget(self._random_list)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._random_list)

        self.register_action(self._get_selection_signal_type(), self._update_all)
        self.register_action(self._get_move_signal_type(), self._update_all)
        self.register_action(self._get_hit_selection_signal_type(), self._update_all)

        self._force.valueChanged.connect(self._move)

        self._update_all()

    def _get_selection_signal_type(self):
        return 'signal_sample_hit_map_selection_{}'.format(self._proc_id)

    def _get_move_signal_type(self):
        return 'signal_sample_hit_map_move_{}'.format(self._proc_id)

    def _get_hit_selection_signal_type(self):
        return 'signal_sample_hit_map_hit_selection_{}'.format(self._proc_id)

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        selected_force = sample_params.get_selected_hit_map_force()
        if selected_force not in sample_params.get_hit_map_forces(hit_info):
            self.setEnabled(False)
            return

        self.setEnabled(True)

        old_block = self._force.blockSignals(True)
        if self._force.value() != selected_force:
            self._force.setValue(selected_force)
        self._force.blockSignals(old_block)

    def _move(self, new_force):
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        selected_force = sample_params.get_selected_hit_map_force()
        if ((selected_force != None) and
                (new_force not in sample_params.get_hit_map_forces(hit_info))):
            sample_params.move_hit_map_point(hit_info, selected_force, new_force)
            sample_params.set_selected_hit_map_force(new_force)
            self._updater.signal_update(self._get_move_signal_type())


class HitRandomList(RandomList):

    def __init__(self):
        super().__init__()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_map_points(self):
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        return sample_params.get_hit_map_forces(hit_info)

    def _get_selected_map_point(self):
        return self._get_sample_params().get_selected_hit_map_force()

    def _get_model_random_list(self, point):
        sample_params = self._get_sample_params()
        hit_info = sample_params.get_selected_hit_info()
        force = sample_params.get_selected_hit_map_force()
        return sample_params.get_hit_map_random_list(hit_info, force)

    def _get_selection_signal_type(self):
        return 'signal_sample_hit_map_selection_{}'.format(self._proc_id)

    def _get_random_list_signal_type(self):
        return 'signal_sample_hit_map_random_list_{}'.format(self._proc_id)

    def _get_update_signals(self):
        return set([
            self._get_selection_signal_type(),
            self._get_random_list_signal_type(),
            'signal_sample_hit_map_hit_selection_{}'.format(self._proc_id),
            'signal_sample_format_{}'.format(self._proc_id)])


class Samples(QSplitter, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self.setOrientation(Qt.Horizontal)

        self._sample_list = SampleList()
        self._sample_editor = SampleEditor()

        self._keyboard_mapper = ProcessorKeyboardMapper()

        self.add_to_updaters(
                self._sample_list, self._sample_editor, self._keyboard_mapper)

        self.addWidget(self._sample_list)
        self.addWidget(self._sample_editor)

        self.setStretchFactor(0, 1)
        self.setStretchFactor(1, 2)

    def keyPressEvent(self, event):
        module = self._ui_model.get_module()
        control_id = module.get_control_id_by_au_id(self._au_id)
        if not control_id:
            return

        control_mgr = self._ui_model.get_control_manager()

        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        use_test_output = (proc.get_existence() and
            control_mgr.is_processor_testing_enabled(self._proc_id))

        if use_test_output:
            sample_params = utils.get_proc_params(
                    self._ui_model, self._au_id, self._proc_id)
            sample_id = sample_params.get_selected_sample_id()
            if sample_id != None:
                sample_num = int(sample_id.split('_')[1], 16)
                sample_num_param = str(sample_num)
                control_mgr.set_test_processor_param(self._proc_id, sample_num_param)
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()
        control_mgr.set_test_processor_param(self._proc_id, None)


class SampleListToolBar(QToolBar, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._import_button = QToolButton()
        self._import_button.setText('Import samples')
        self._import_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove sample')
        self._remove_button.setEnabled(False)

        self.addWidget(self._import_button)
        self.addWidget(self._remove_button)

    def _on_setup(self):
        self.register_action(self._get_list_signal_type(), self._update_enabled)
        self.register_action(self._get_selection_signal_type(), self._update_enabled)

        self._import_button.clicked.connect(self._import_samples)
        self._remove_button.clicked.connect(self._remove_sample)

        self._update_enabled()

    def _get_selection_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _get_list_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_note_map_random_list_signal_type(self):
        return 'signal_sample_note_map_random_list_{}'.format(self._proc_id)

    def _get_hit_map_random_list_signal_type(self):
        return 'signal_sample_hit_map_random_list_{}'.format(self._proc_id)

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_enabled(self):
        sample_params = self._get_sample_params()
        id_count = len(sample_params.get_sample_ids())
        any_selected = (sample_params.get_selected_sample_id() != None)
        self._import_button.setEnabled(id_count < sample_params.get_max_sample_count())
        self._remove_button.setEnabled(any_selected and (id_count > 0))

    def _import_samples(self):
        sample_params = self._get_sample_params()
        sample_ids = sample_params.get_free_sample_ids()
        if not sample_ids:
            return

        filters = [
            'All supported types (*.wav *.aiff *.aif *.aifc *.au *.snd *.wv *.flac)',
            'Waveform Audio File Format (*.wav)',
            'Audio Interchange File Format (*.aiff *.aif *.aifc)',
            'Sun Au (*.au *.snd)',
            'WavPack (*.wv)',
            'Free Lossless Audio Codec (*.flac)',
        ]

        free_count = len(sample_ids)

        sample_paths, _ = QFileDialog.getOpenFileNames(
                None,
                'Import samples ({} free slot{})'.format(
                    free_count, '' if free_count == 1 else 's'),
                config.get_config().get_value('dir_samples') or '',
                ';;'.join(filters))
        if sample_paths:
            # Make sure we've got enough space
            if len(sample_paths) > free_count:
                error_msg_lines = [
                        'Too many samples requested ({})'.format(len(sample_paths)),
                        'This processor has space for {} more sample{}'.format(
                            free_count, '' if free_count == 1 else 's')]
                error_msg = '<p>{}</p>'.format('<br>'.join(error_msg_lines))
                dialog = ImportErrorDialog(self._ui_model, error_msg)
                dialog.exec_()
                return

            imports = zip(sample_ids, sample_paths)

            def on_complete():
                self._updater.signal_update(
                        self._get_list_signal_type(),
                        self._get_note_map_random_list_signal_type(),
                        self._get_hit_map_random_list_signal_type())

            def on_error(e):
                error_msg_lines = str(e).split('\n')
                error_msg = '<p>{}</p>'.format('<br>'.join(error_msg_lines))
                dialog = ImportErrorDialog(self._ui_model, error_msg)
                dialog.exec_()

            task = sample_params.get_task_import_samples(imports, on_complete, on_error)
            task_executor = self._ui_model.get_task_executor()
            task_executor(task)

    def _remove_sample(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        if sample_id:
            sample_params.remove_sample(sample_id)
            self._updater.signal_update(
                self._get_list_signal_type(),
                self._get_note_map_random_list_signal_type(),
                self._get_hit_map_random_list_signal_type())


class ImportErrorDialog(QDialog):

    def __init__(self, ui_model, error_msg):
        super().__init__()
        style_mgr = ui_model.get_style_manager()
        icon_bank = ui_model.get_icon_bank()

        error_img_orig = QPixmap(icon_bank.get_icon_path('error'))
        error_img = error_img_orig.scaledToWidth(
                style_mgr.get_scaled_size_param('dialog_icon_size'),
                Qt.SmoothTransformation)
        error_label = QLabel()
        error_label.setPixmap(error_img)

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        margin = style_mgr.get_scaled_size_param('large_padding')
        h.setContentsMargins(margin, margin, margin, margin)
        h.setSpacing(margin * 2)
        h.addWidget(error_label)
        h.addWidget(self._message)

        self._button_layout = QHBoxLayout()

        v = QVBoxLayout()
        v.addLayout(h)
        v.addLayout(self._button_layout)

        self.setLayout(v)

        # Dialog contents, TODO: make a common ErrorDialog class
        self._message.setText(error_msg)

        self._ok_button = QPushButton('OK')
        self._button_layout.addStretch(1)
        self._button_layout.addWidget(self._ok_button)
        self._button_layout.addStretch(1)

        self._ok_button.clicked.connect(self.close)


class SampleListModel(QAbstractListModel, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._items = []

    def _on_setup(self):
        self._make_items()

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        for sample_id in sample_params.get_sample_ids():
            name = sample_params.get_sample_name(sample_id)
            self._items.append((sample_id, name))
        self._items = sorted(self._items, key=lambda x: x[1] or '')

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                _, vis_name = self._items[row]
                if vis_name == None:
                    vis_name = '-'
                return vis_name

        return None

    def headerData(self, section, orientation, role):
        return None


class SampleListView(QListView, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._keyboard_mapper = ProcessorKeyboardMapper()

        self.add_to_updaters(self._keyboard_mapper)

        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def _get_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _select_sample(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item:
            sample_id, _ = item
            sample_params = utils.get_proc_params(
                    self._ui_model, self._au_id, self._proc_id)
            sample_params.set_selected_sample_id(sample_id)
            self._updater.signal_update(self._get_update_signal_type())

    def setModel(self, model):
        super().setModel(model)
        self.selectionModel().currentChanged.connect(self._select_sample)

    def keyPressEvent(self, event):
        if self._keyboard_mapper.is_handled_key(event):
            event.ignore()
        else:
            super().keyPressEvent(event)

    def keyReleaseEvent(self, event):
        if self._keyboard_mapper.is_handled_key(event):
            event.ignore()
        else:
            super().keyReleaseEvent(event)


class SampleList(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._toolbar = SampleListToolBar()

        self._list_model = None
        self._list_view = SampleListView()

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 2, 4)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._toolbar, self._list_view)
        self.register_action(self._get_update_signal_type(), self._update_model)
        self.register_action(self._get_rename_signal_type(), self._update_model)

        self._update_model()

    def _get_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_rename_signal_type(self):
        return 'signal_sample_rename_{}'.format(self._proc_id)

    def _update_model(self):
        if self._list_model:
            self.remove_from_updaters(self._list_model)
        self._list_model = SampleListModel()
        self.add_to_updaters(self._list_model)
        self._list_view.setModel(self._list_model)


class SampleEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._name = QLineEdit()

        self._freq = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._freq.setRange(1, 2**32)

        self._resample = QPushButton('Resample...')

        freq_l = QHBoxLayout()
        freq_l.setContentsMargins(0, 0, 0, 0)
        freq_l.setSpacing(2)
        freq_l.addWidget(self._freq, 1)
        freq_l.addWidget(self._resample)

        self._loop_mode = KqtComboBox()
        loop_modes = (
                ('Off', 'off'),
                ('Unidirectional', 'uni'),
                ('Bidirectional', 'bi'))
        for vis_mode, mode in loop_modes:
            self._loop_mode.addItem(vis_mode, mode)

        self._loop_xfader = QPushButton('Create crossfade...')

        self._loop_start = QSpinBox()
        self._loop_start.setRange(0, 2**30)
        self._loop_end = QSpinBox()
        self._loop_end.setRange(0, 2**30)
        self._length = QLabel('?')

        self._format = QLabel('?')
        self._format_change = QPushButton('Change...')

        format_l = QHBoxLayout()
        format_l.setContentsMargins(0, 0, 0, 0)
        format_l.setSpacing(2)
        format_l.addWidget(self._format, 1)
        format_l.addWidget(self._format_change)

        loop_mode_l = QHBoxLayout()
        loop_mode_l.setContentsMargins(0, 0, 0, 0)
        loop_mode_l.setSpacing(2)
        loop_mode_l.addWidget(self._loop_mode, 1)
        loop_mode_l.addWidget(self._loop_xfader)

        gl = QGridLayout()
        gl.setContentsMargins(0, 0, 0, 0)
        gl.setSpacing(2)
        gl.addWidget(QLabel('Name:'), 0, 0)
        gl.addWidget(self._name, 0, 1)
        gl.addWidget(QLabel('Middle frequency:'), 1, 0)
        gl.addLayout(freq_l, 1, 1)
        gl.addWidget(QLabel('Loop mode:'), 2, 0)
        gl.addLayout(loop_mode_l, 2, 1)
        gl.addWidget(QLabel('Loop start:'), 3, 0)
        gl.addWidget(self._loop_start, 3, 1)
        gl.addWidget(QLabel('Loop end:'), 4, 0)
        gl.addWidget(self._loop_end, 4, 1)
        gl.addWidget(QLabel('Length:'), 5, 0)
        gl.addWidget(self._length, 5, 1)
        gl.addWidget(QLabel('Format:'), 6, 0)
        gl.addLayout(format_l, 6, 1)

        self._sample_view = SampleView()

        v = QVBoxLayout()
        v.setContentsMargins(2, 4, 4, 4)
        v.setSpacing(0)
        v.addLayout(gl)
        v.addWidget(self._sample_view, 1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action(self._get_list_update_signal_type(), self._update_all)
        self.register_action(self._get_selection_update_signal_type(), self._update_all)
        self.register_action(self._get_rename_signal_type(), self._update_name)
        self.register_action(self._get_freq_signal_type(), self._update_freq)
        self.register_action(
                self._get_resample_signal_type(), self._update_after_resample)
        self.register_action(self._get_format_signal_type(), self._update_after_format)
        self.register_action(self._get_loop_signal_type(), self._update_loop)
        self.register_action(self._get_loop_xfade_signal_type(), self._update_all)
        self.register_action(self._get_cut_signal_type(), self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._name.editingFinished.connect(self._change_name)
        self._freq.valueChanged.connect(self._change_freq)
        self._resample.clicked.connect(self._convert_freq)
        self._loop_mode.currentIndexChanged.connect(self._change_loop_mode)
        self._loop_xfader.clicked.connect(self._create_loop_xfade)
        self._loop_start.valueChanged.connect(self._change_loop_start)
        self._loop_end.valueChanged.connect(self._change_loop_end)
        self._sample_view.set_ui_model(self._ui_model)
        self._sample_view.loopStartChanged.connect(self._change_loop_start)
        self._sample_view.loopStopChanged.connect(self._change_loop_end)
        self._sample_view.postLoopCut.connect(self._post_loop_cut)
        self._format_change.clicked.connect(self._change_format)

        self._update_style()
        self._update_all()

    def _on_teardown(self):
        self._sample_view.unregister_updaters()

    def _get_list_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_selection_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _get_note_random_list_signal_type(self):
        return 'signal_sample_note_map_random_list_{}'.format(self._proc_id)

    def _get_hit_random_list_signal_type(self):
        return 'signal_sample_hit_map_random_list_{}'.format(self._proc_id)

    def _get_rename_signal_type(self):
        return 'signal_sample_rename_{}'.format(self._proc_id)

    def _get_freq_signal_type(self):
        return 'signal_sample_freq_{}'.format(self._proc_id)

    def _get_resample_signal_type(self):
        return 'signal_sample_resample_{}'.format(self._proc_id)

    def _get_format_signal_type(self):
        return 'signal_sample_format_{}'.format(self._proc_id)

    def _get_loop_signal_type(self):
        return 'signal_sample_loop_{}'.format(self._proc_id)

    def _get_loop_xfade_signal_type(self):
        return 'signal_sample_loop_xfade_{}'.format(self._proc_id)

    def _get_cut_signal_type(self):
        return 'signal_sample_cut_{}'.format(self._proc_id)

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        if not style_mgr.is_custom_style_enabled():
            self._sample_view.set_config({})
            return

        def get_colour(name):
            return QColor(style_mgr.get_style_param(name))

        config = {
            'bg_colour'                 : get_colour('waveform_bg_colour'),
            'centre_line_colour'        : get_colour('waveform_centre_line_colour'),
            'zoomed_out_colour'         : get_colour('waveform_zoomed_out_colour'),
            'single_item_colour'        : get_colour('waveform_single_item_colour'),
            'interp_colour'             : get_colour('waveform_interpolated_colour'),
            'max_node_size'             : style_mgr.get_scaled_size(0.55),
            'loop_line_colour'          : get_colour('waveform_loop_marker_colour'),
            'focused_loop_line_colour'  : get_colour('waveform_focus_colour'),
            'loop_line_dash'            : [style_mgr.get_scaled_size(0.4)] * 2,
            'loop_line_thickness'       : style_mgr.get_scaled_size(0.1),
            'loop_handle_colour'        : get_colour('waveform_loop_marker_colour'),
            'focused_loop_handle_colour': get_colour('waveform_focus_colour'),
            'loop_handle_size'          : style_mgr.get_scaled_size(1.3),
            'loop_handle_focus_dist_max': style_mgr.get_scaled_size(1.5),
        }

        self._sample_view.set_config(config)
        self._update_sample_view()
        self._update_loop()

    def _update_all(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        has_sample = sample_id in sample_params.get_sample_ids()
        self.setEnabled(has_sample)

        self._update_name()
        self._update_freq()
        self._update_sample_view()
        self._update_loop()
        self._update_format()

    def _update_after_resample(self):
        self._update_freq()
        self._update_sample_view()
        self._update_loop()

    def _update_after_format(self):
        self._update_format()
        self._update_sample_view()
        self._update_loop()

    def _update_name(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        has_sample = sample_id in sample_params.get_sample_ids()

        name = ''
        if has_sample:
            name = sample_params.get_sample_name(sample_id) or ''

        old_block = self._name.blockSignals(True)
        if self._name.text() != name:
            self._name.setText(name)
        self._name.blockSignals(old_block)

    def _update_freq(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()

        old_block = self._freq.blockSignals(True)
        new_freq = sample_params.get_sample_freq(sample_id)
        if self._freq.value() != new_freq:
            self._freq.setValue(new_freq)
        self._freq.blockSignals(old_block)

    def _update_sample_view(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()

        sample_length = sample_params.get_sample_length(sample_id)
        self._length.setText(str(sample_length))
        get_sample_data = sample_params.get_sample_data_retriever(sample_id)
        self._sample_view.set_sample(sample_length, get_sample_data)

    def _update_loop(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()

        new_loop_mode = sample_params.get_sample_loop_mode(sample_id)
        new_loop_start = sample_params.get_sample_loop_start(sample_id)
        new_loop_end = sample_params.get_sample_loop_end(sample_id)

        sample_length = sample_params.get_sample_length(sample_id)
        is_loop_range_valid = 0 <= new_loop_start < new_loop_end <= sample_length
        is_loop_enabled = new_loop_mode != 'off' and is_loop_range_valid

        old_block = self._loop_mode.blockSignals(True)
        loop_mode_index = self._loop_mode.findData(new_loop_mode)
        if (loop_mode_index != self._loop_mode.itemData(self._loop_mode.currentIndex())):
            self._loop_mode.setCurrentIndex(loop_mode_index)
        self._loop_mode.blockSignals(old_block)

        self._loop_xfader.setEnabled(
                (new_loop_mode == 'uni') and
                (new_loop_start > 0) and
                (new_loop_end - new_loop_start >= 2))

        old_block = self._loop_start.blockSignals(True)
        self._loop_start.setMinimum(0)
        self._loop_start.setMaximum(max(0, sample_length - 1))
        if new_loop_start != self._loop_start.value():
            self._loop_start.setValue(new_loop_start)
        self._loop_start.setEnabled(is_loop_enabled)
        if is_loop_enabled:
            self._loop_start.setMaximum(new_loop_end - 1)
        self._loop_start.blockSignals(old_block)

        old_block = self._loop_end.blockSignals(True)
        self._loop_end.setMinimum(min(1, sample_length))
        self._loop_end.setMaximum(sample_length)
        if new_loop_end != self._loop_end.value():
            self._loop_end.setValue(new_loop_end)
        self._loop_end.setEnabled(is_loop_enabled)
        if is_loop_enabled:
            self._loop_end.setMinimum(new_loop_start + 1)
        self._loop_end.blockSignals(old_block)

        loop_range = [
                sample_params.get_sample_loop_start(sample_id),
                sample_params.get_sample_loop_end(sample_id)]
        if new_loop_mode != 'off':
            self._sample_view.set_loop_range(loop_range)
        else:
            self._sample_view.set_loop_range(None)

    def _update_format(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()

        format_info = sample_params.get_sample_format(sample_id)
        desc = ''
        if format_info:
            bits, is_float = format_info
            desc = '{}-bit{}'.format(bits, ' float' if is_float else '')
        self._format.setText(desc)

    def _change_name(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_name(sample_id, str(self._name.text()))
        self._updater.signal_update(
            self._get_rename_signal_type(),
            self._get_note_random_list_signal_type(),
            self._get_hit_random_list_signal_type())

    def _change_freq(self, value):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_freq(sample_id, value)
        self._updater.signal_update(self._get_freq_signal_type())

    def _convert_freq(self):
        sample_params = self._get_sample_params()
        task_executor = self._ui_model.get_task_executor()
        on_resample = lambda: self._updater.signal_update(
                self._get_resample_signal_type())
        resample_editor = ResampleEditor(sample_params, task_executor, on_resample)
        resample_editor.exec_()

    def _change_format(self):
        sample_params = self._get_sample_params()
        task_executor = self._ui_model.get_task_executor()
        on_convert = lambda: self._updater.signal_update(
                self._get_format_signal_type())
        format_editor = SampleFormatEditor(sample_params, task_executor, on_convert)
        format_editor.exec_()

    def _change_loop_mode(self, item_index):
        loop_mode = self._loop_mode.itemData(item_index)
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_loop_mode(sample_id, loop_mode)
        self._updater.signal_update(self._get_loop_signal_type())

    def _create_loop_xfade(self):
        on_xfade = lambda: self._updater.signal_update(
                self._get_loop_xfade_signal_type())
        xfader = LoopXFader(
                self._get_sample_params(), self._ui_model.get_task_executor(), on_xfade)
        xfader.exec_()

    def _change_loop_start(self, start):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_loop_start(sample_id, start)
        self._updater.signal_update(self._get_loop_signal_type())

    def _change_loop_end(self, end):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_loop_end(sample_id, end)
        self._updater.signal_update(self._get_loop_signal_type())

    def _post_loop_cut(self):
        on_cut = lambda: self._updater.signal_update(self._get_cut_signal_type())
        confirm_dialog = PostLoopCutConfirmDialog(
                self._ui_model,
                self._get_sample_params(),
                self._ui_model.get_task_executor(),
                on_cut)
        confirm_dialog.exec_()


class ResampleEditor(QDialog):

    def __init__(self, sample_params, task_executor, on_resample):
        super().__init__()
        self._sample_params = sample_params
        self._task_executor = task_executor
        self._on_resample = on_resample

        sample_id = self._sample_params.get_selected_sample_id()
        sample_freq = self._sample_params.get_sample_freq(sample_id)

        self._freq = QSpinBox()
        self._freq.setMinimum(int(math.ceil(sample_freq / 256)))
        self._freq.setMaximum(int(math.floor(sample_freq * 256)))
        self._freq.setValue(sample_freq)

        sl = QHBoxLayout()
        sl.setContentsMargins(0, 0, 0, 0)
        sl.setSpacing(4)
        sl.addWidget(QLabel('New frequency:'))
        sl.addWidget(self._freq, 1)

        self._cancel_button = QPushButton('Cancel')
        self._resample_button = QPushButton('Resample')

        bl = QHBoxLayout()
        bl.setContentsMargins(0, 0, 0, 0)
        bl.setSpacing(4)
        bl.addWidget(self._cancel_button)
        bl.addWidget(self._resample_button)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addLayout(sl)
        v.addLayout(bl)
        self.setLayout(v)

        self._freq.valueChanged.connect(self._update_enabled)

        self._cancel_button.clicked.connect(self.close)
        self._resample_button.clicked.connect(self._resample)

        self._update_enabled()

    def _update_enabled(self):
        sample_id = self._sample_params.get_selected_sample_id()
        sample_freq = self._sample_params.get_sample_freq(sample_id)
        self._resample_button.setEnabled(sample_freq != self._freq.value())

    def _resample(self):
        sample_id = self._sample_params.get_selected_sample_id()

        self.setEnabled(False)

        target_freq = self._freq.value()

        task = self._sample_params.get_task_convert_sample_freq(
                sample_id, target_freq, self._on_resample)
        self._task_executor(task)

        self.close()


class SampleFormatEditor(QDialog):

    def __init__(self, sample_params, task_executor, on_convert):
        super().__init__()
        self._sample_params = sample_params
        self._task_executor = task_executor
        self._on_convert = on_convert

        sample_id = self._sample_params.get_selected_sample_id()
        sample_format = self._sample_params.get_sample_format(sample_id)

        self._format = KqtComboBox()
        self._normalise = QCheckBox()

        formats = [(8, False), (16, False), (24, False), (32, False), (32, True)]
        for i, fmt in enumerate(formats):
            bits, is_float = fmt
            desc = '{}-bit{}'.format(bits, ' float' if is_float else '')
            self._format.addItem(desc, fmt)
            if fmt == sample_format:
                self._format.setCurrentIndex(i)

        sl = QGridLayout()
        sl.setContentsMargins(0, 0, 0, 0)
        sl.setSpacing(4)
        sl.addWidget(QLabel('Format:'), 0, 0)
        sl.addWidget(self._format, 0, 1)
        sl.addWidget(QLabel('Normalise:'), 1, 0)
        sl.addWidget(self._normalise, 1, 1)

        self._cancel_button = QPushButton('Cancel')
        self._convert_button = QPushButton('Convert')

        bl = QHBoxLayout()
        bl.setContentsMargins(0, 0, 0, 0)
        bl.setSpacing(4)
        bl.addWidget(self._cancel_button)
        bl.addWidget(self._convert_button)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addLayout(sl)
        v.addLayout(bl)
        self.setLayout(v)

        self._format.currentIndexChanged.connect(self._change_format)

        self._cancel_button.clicked.connect(self.close)
        self._convert_button.clicked.connect(self._convert)

        self._update_enabled()

    def _change_format(self, dummy):
        self._update_enabled()

    def _update_enabled(self):
        sample_id = self._sample_params.get_selected_sample_id()
        sample_format = self._sample_params.get_sample_format(sample_id)
        selected_format = tuple(self._format.itemData(self._format.currentIndex()))
        if not selected_format:
            self._normalise.setEnabled(True)
            self._convert_button.setEnabled(False)
            return

        sample_bits, _ = sample_format
        selected_bits, _ = selected_format
        if selected_bits < sample_bits:
            self._normalise.setEnabled(True)
        else:
            self._normalise.setEnabled(False)
            old_block = self._normalise.blockSignals(True)
            self._normalise.setCheckState(Qt.Unchecked)
            self._normalise.blockSignals(old_block)

        self._convert_button.setEnabled(sample_format != selected_format)

    def _convert(self):
        sample_id = self._sample_params.get_selected_sample_id()

        self.setEnabled(False)

        bits, is_float = self._format.itemData(self._format.currentIndex())
        normalise = (self._normalise.checkState() == Qt.Checked)

        task = self._sample_params.get_task_convert_sample_format(
                sample_id, bits, is_float, normalise, self._on_convert)
        self._task_executor(task)

        self.close()


class LoopXFader(QDialog):

    def __init__(self, sample_params, task_executor, on_xfade):
        super().__init__()
        self._sample_params = sample_params
        self._task_executor = task_executor
        self._on_xfade = on_xfade

        sample_id = sample_params.get_selected_sample_id()
        loop_start = sample_params.get_sample_loop_start(sample_id)
        loop_end = sample_params.get_sample_loop_end(sample_id)
        loop_length = loop_end - loop_start
        max_xfade_length = min(loop_start, loop_length)

        self._xfade_length = QSpinBox()
        self._xfade_length.setRange(1, max_xfade_length)
        self._xfade_length.setValue(max_xfade_length)

        xl = QHBoxLayout()
        xl.setContentsMargins(0, 0, 0, 0)
        xl.setSpacing(4)
        xl.addWidget(QLabel('Crossfade length:'))
        xl.addWidget(self._xfade_length, 1)

        self._cancel_button = QPushButton('Cancel')
        self._xfade_button = QPushButton('Create crossfade')

        bl = QHBoxLayout()
        bl.addWidget(self._cancel_button)
        bl.addWidget(self._xfade_button)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addLayout(xl)
        v.addLayout(bl)
        self.setLayout(v)

        self._cancel_button.clicked.connect(self.close)
        self._xfade_button.clicked.connect(self._xfade)

    def _xfade(self):
        sample_id = self._sample_params.get_selected_sample_id()

        self.setEnabled(False)

        xfade_length = self._xfade_length.value()

        task = self._sample_params.get_task_create_xfade_loop_region(
                sample_id, xfade_length, self._on_xfade)
        self._task_executor(task)

        self.close()


class PostLoopCutConfirmDialog(ConfirmDialog):

    def __init__(self, ui_model, sample_params, task_executor, on_cut):
        super().__init__(ui_model)

        self._set_message('Cut all sample data past loop end?')

        self._sample_params = sample_params
        self._task_executor = task_executor
        self._on_cut = on_cut

        self.setWindowTitle('Confirm sample cut')

        self._cancel_button = QPushButton('No')
        self._cut_button = QPushButton('Yes')

        bl = self._get_button_layout()
        bl.addWidget(self._cancel_button)
        bl.addWidget(self._cut_button)

        self._cancel_button.setFocus(Qt.PopupFocusReason)

        self._cancel_button.clicked.connect(self.close)
        self._cut_button.clicked.connect(self._cut)

    def _cut(self):
        sample_id = self._sample_params.get_selected_sample_id()

        self.setEnabled(False)

        task = self._sample_params.get_task_post_loop_cut(sample_id, self._on_cut)
        self._task_executor(task)

        self.close()


