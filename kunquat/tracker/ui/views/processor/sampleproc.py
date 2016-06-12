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

import math
import time

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.extras.wavpack import WavPackRMem
from kunquat.tracker.ui.model.procparams.sampleparams import SampleImportError
from kunquat.tracker.ui.views.audio_unit.hitselector import HitSelector
from kunquat.tracker.ui.views.axisrenderer import HorizontalAxisRenderer, VerticalAxisRenderer
from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper
from kunquat.tracker.ui.views.utils import lerp_val
from .sampleview import SampleView
from . import utils


class SampleProc(QTabWidget):

    @staticmethod
    def get_name():
        return 'Sample synthesis'

    def __init__(self):
        super().__init__()

        self._note_map_editor = NoteMapEditor()
        self._hit_map_editor = HitMapEditor()
        self._samples = Samples()

        self.addTab(self._note_map_editor, 'Note map')
        self.addTab(self._hit_map_editor, 'Hit map')
        self.addTab(self._samples, 'Samples')

    def set_au_id(self, au_id):
        self._note_map_editor.set_au_id(au_id)
        self._hit_map_editor.set_au_id(au_id)
        self._samples.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._note_map_editor.set_proc_id(proc_id)
        self._hit_map_editor.set_proc_id(proc_id)
        self._samples.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._note_map_editor.set_ui_model(ui_model)
        self._hit_map_editor.set_ui_model(ui_model)
        self._samples.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._samples.unregister_updaters()
        self._hit_map_editor.unregister_updaters()
        self._note_map_editor.unregister_updaters()


class NoteMapEditor(QWidget):

    def __init__(self):
        super().__init__()

        self._note_map = NoteMap()
        self._note_map_entry = NoteMapEntry()

        h = QHBoxLayout()
        h.setContentsMargins(2, 2, 2, 2)
        h.setSpacing(4)
        h.addWidget(self._note_map, 1)
        h.addWidget(self._note_map_entry, 2)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._note_map.set_au_id(au_id)
        self._note_map_entry.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._note_map.set_proc_id(proc_id)
        self._note_map_entry.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._note_map.set_ui_model(ui_model)
        self._note_map_entry.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._note_map_entry.unregister_updaters()
        self._note_map.unregister_updaters()


class RandomListMap(QWidget):

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

    _FONT = QFont(QFont().defaultFamily(), 9)
    _FONT.setWeight(QFont.Bold)

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
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._axis_x_renderer = HorizontalAxisRenderer()
        self._axis_x_renderer.set_config(self._AXIS_CONFIG, self)
        self._axis_x_renderer.set_val_range([-36, 0])

        self._axis_y_renderer = VerticalAxisRenderer()
        self._axis_y_renderer.set_config(self._AXIS_CONFIG, self)
        y_range = [-6000, 6000] if self._has_pitch_axis() else [0, 0]
        self._axis_y_renderer.set_val_range(y_range)

        self._focused_point = None

        self._state = self._STATE_IDLE

        self._is_start_snapping_active = False
        self._moving_pointer_offset = (0, 0)

        self._config = None
        self._set_config({})

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setMouseTracking(True)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _set_config(self, config):
        self._config = self._DEFAULT_CONFIG.copy()
        self._config.update(config)

        if not self._has_pitch_axis():
            padding = self._config['padding']
            x_axis_height = self._AXIS_CONFIG['axis_x']['height']
            self.setFixedHeight(padding * 2 + x_axis_height)

    def _perform_updates(self, signals):
        update_signals = self._get_update_signals()
        if not signals.isdisjoint(update_signals):
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
                    self._updater.signal_update(set([self._get_selection_signal_type()]))
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
                        self._updater.signal_update(set([self._get_move_signal_type()]))

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

                self._updater.signal_update(set([self._get_selection_signal_type()]))
            else:
                new_point = self._get_point_coords((x, y))
                pitch_range = self._axis_y_renderer.get_val_range()
                force_range = self._axis_x_renderer.get_val_range()
                if ((pitch_range[0] <= new_point[0] <= pitch_range[1]) and
                        force_range[0] <= new_point[1] <= force_range[1]):
                    self._add_point(new_point)
                    self._set_selected_point(new_point)

                    self._state = self._STATE_MOVING
                    self._is_start_snapping_active = True
                    self._moving_pointer_offset = (0, 0)

                    self._updater.signal_update(set([self._get_selection_signal_type()]))

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
        axis_x_height = self._AXIS_CONFIG['axis_x']['height']
        axis_x_top = self.height() - padding - axis_x_height
        axis_y_width = self._AXIS_CONFIG['axis_y']['width']
        axis_x_length = self.width() - axis_y_width - (padding * 2) + 1
        axis_y_length = self.height() - axis_x_height - (padding * 2) + 1

        painter.setTransform(
                QTransform().translate(padding, axis_x_top))
        self._axis_x_renderer.set_width(self.width())
        self._axis_x_renderer.set_axis_length(axis_x_length)
        self._axis_x_renderer.set_x_offset(axis_y_width - 1)
        self._axis_x_renderer.set_y_offset_x(self.width() - axis_y_width - padding * 2)
        self._axis_x_renderer.render(painter)

        if self._has_pitch_axis():
            painter.setTransform(QTransform().translate(padding, padding))
            self._axis_y_renderer.set_height(self.height())
            self._axis_y_renderer.set_axis_length(axis_y_length)
            self._axis_y_renderer.set_x_offset_y(
                    (self.height() - axis_x_height - padding * 2) // 2)
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


class NoteMapEntry(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setEnabled(False)

        self._pitch = QDoubleSpinBox()
        self._pitch.setDecimals(0)
        self._pitch.setRange(-6000, 6000)
        self._force = QDoubleSpinBox()
        self._force.setDecimals(1)
        self._force.setRange(-36, 0)

        self._random_list = NoteRandomList()

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

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._random_list.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._random_list.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._random_list.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._pitch, SIGNAL('valueChanged(double)'), self._move)
        QObject.connect(self._force, SIGNAL('valueChanged(double)'), self._move)

        self._update_all()

    def unregister_updaters(self):
        self._random_list.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _get_selection_signal_type(self):
        return 'signal_sample_note_map_selection_{}'.format(self._proc_id)

    def _get_move_signal_type(self):
        return 'signal_sample_note_map_move_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_selection_signal_type(), self._get_move_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

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
                self._updater.signal_update(set([self._get_move_signal_type()]))


class RandomList(EditorList):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

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
        self._adder.set_au_id(self._au_id)
        self._adder.set_proc_id(self._proc_id)
        self._adder.set_ui_model(self._ui_model)
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
        editor.set_au_id(self._au_id)
        editor.set_proc_id(self._proc_id)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _perform_updates(self, signals):
        update_signals = self._get_update_signals()
        if not signals.isdisjoint(update_signals):
            self._update_all()

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


class RandomEntryAdder(QPushButton):

    def __init__(self, cb_info):
        super().__init__('Add sample entry')
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._get_map_points = cb_info['get_map_points']
        self._get_selected_map_point = cb_info['get_selected_map_point']
        self._get_model_random_list = cb_info['get_model_random_list']
        self._get_update_signal_type = cb_info['get_random_list_signal_type']

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._add_entry)

    def unregister_updaters(self):
        pass

    def _add_entry(self):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.add_entry()
            self._updater.signal_update(set([self._get_update_signal_type()]))


class RandomEntryEditor(QWidget):

    def __init__(self, cb_info, index):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._get_map_points = cb_info['get_map_points']
        self._get_selected_map_point = cb_info['get_selected_map_point']
        self._get_model_random_list = cb_info['get_model_random_list']
        self._get_random_list_signal_type = cb_info['get_random_list_signal_type']
        self._get_update_signals = cb_info['get_update_signals']

        self._index = index

        self._sample_selector = QComboBox()
        self._sample_selector.setSizeAdjustPolicy(QComboBox.AdjustToContents)

        self._pitch_shift = QDoubleSpinBox()
        self._pitch_shift.setDecimals(0)
        self._pitch_shift.setRange(-6000, 6000)

        self._volume_shift = QDoubleSpinBox()
        self._volume_shift.setDecimals(1)
        self._volume_shift.setRange(-64, 64)

        self._remove_button = QPushButton()
        self._remove_button.setStyleSheet('padding: 0 -2px;')

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

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))
        self._remove_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        QObject.connect(
                self._sample_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._change_sample)

        QObject.connect(
                self._pitch_shift,
                SIGNAL('valueChanged(double)'),
                self._change_pitch_shift)

        QObject.connect(
                self._volume_shift,
                SIGNAL('valueChanged(double)'),
                self._change_volume_shift)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_entry)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = self._get_update_signals()
        if not signals.isdisjoint(update_signals):
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
        self._sample_selector.clear()
        self._sample_selector.setEnabled(len(sample_ids) > 0)
        for i, info in enumerate(sample_info):
            sample_id, sample_name = info
            vis_name = sample_name or '-'
            self._sample_selector.addItem(vis_name, sample_id)
            if sample_id == cur_sample_id:
                self._sample_selector.setCurrentIndex(i)
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
            self._updater.signal_update(set([self._get_random_list_signal_type()]))

    def _change_pitch_shift(self, value):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.set_cents_offset(self._index, value)
            self._updater.signal_update(set([self._get_random_list_signal_type()]))

    def _change_volume_shift(self, value):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.set_volume_adjust(self._index, value)
            self._updater.signal_update(set([self._get_random_list_signal_type()]))

    def _remove_entry(self):
        point = self._get_selected_map_point()
        if (point != None) and (point in self._get_map_points()):
            random_list = self._get_model_random_list(point)
            random_list.remove_entry(self._index)
            self._updater.signal_update(set([self._get_random_list_signal_type()]))


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
            self._get_selection_signal_type(), self._get_random_list_signal_type()])


class HitMapEditor(QWidget):

    def __init__(self):
        super().__init__()

        self._hit_selector = SampleHitSelector()
        self._hit_map = HitMap()
        self._hit_map_entry = HitMapEntry()

        v = QVBoxLayout()
        v.setContentsMargins(2, 2, 2, 2)
        v.setSpacing(2)
        v.addWidget(self._hit_selector)
        v.addWidget(self._hit_map)
        v.addWidget(self._hit_map_entry)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._hit_selector.set_au_id(au_id)
        self._hit_map.set_au_id(au_id)
        self._hit_map_entry.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._hit_selector.set_proc_id(proc_id)
        self._hit_map.set_proc_id(proc_id)
        self._hit_map_entry.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._hit_selector.set_ui_model(ui_model)
        self._hit_map.set_ui_model(ui_model)
        self._hit_map_entry.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._hit_map_entry.unregister_updaters()
        self._hit_map.unregister_updaters()
        self._hit_selector.unregister_updaters()


class SampleHitSelector(HitSelector):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self.create_layout(self._ui_model.get_typewriter_manager())
        self.update_contents()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_sample_hit_map_hit_selection_{}'.format(self._proc_id)

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self.update_contents()

    def _get_selected_hit_info(self):
        sample_params = self._get_sample_params()
        return sample_params.get_selected_hit_info()

    def _set_selected_hit_info(self, hit_info):
        sample_params = self._get_sample_params()
        sample_params.set_selected_hit_info(hit_info)
        self._updater.signal_update(set([self._get_update_signal_type()]))


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


class HitMapEntry(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setEnabled(False)

        self._force = QDoubleSpinBox()
        self._force.setDecimals(1)
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

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._random_list.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._random_list.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._random_list.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._force, SIGNAL('valueChanged(double)'), self._move)

        self._update_all()

    def unregister_updaters(self):
        self._random_list.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _get_selection_signal_type(self):
        return 'signal_sample_hit_map_selection_{}'.format(self._proc_id)

    def _get_move_signal_type(self):
        return 'signal_sample_hit_map_move_{}'.format(self._proc_id)

    def _get_hit_selection_signal_type(self):
        return 'signal_sample_hit_map_hit_selection_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_selection_signal_type(),
            self._get_move_signal_type(),
            self._get_hit_selection_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

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
            self._updater.signal_update(set([self._get_move_signal_type()]))


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
            'signal_sample_hit_map_hit_selection_{}'.format(self._proc_id)])


class Samples(QWidget):

    def __init__(self):
        super().__init__()

        self._sample_list = SampleList()
        self._sample_editor = SampleEditor()

        h = QHBoxLayout()
        h.setContentsMargins(2, 2, 2, 2)
        h.setSpacing(4)
        h.addWidget(self._sample_list)
        h.addWidget(self._sample_editor)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._sample_list.set_au_id(au_id)
        self._sample_editor.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._sample_list.set_proc_id(proc_id)
        self._sample_editor.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._sample_list.set_ui_model(ui_model)
        self._sample_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._sample_editor.unregister_updaters()
        self._sample_list.unregister_updaters()


class SampleListToolBar(QToolBar):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._import_button = QToolButton()
        self._import_button.setText('Import sample')
        self._import_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove sample')
        self._remove_button.setEnabled(False)

        self.addWidget(self._import_button)
        self.addWidget(self._remove_button)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._import_button, SIGNAL('clicked()'), self._import_sample)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_sample)

        self._update_enabled()

    def _get_selection_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _get_list_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_note_map_random_list_signal_type(self):
        return 'signal_sample_note_map_random_list_{}'.format(self._proc_id)

    def _get_hit_map_random_list_signal_type(self):
        return 'signal_sample_hit_map_random_list_{}'.format(self._proc_id)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_list_signal_type(), self._get_selection_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_enabled(self):
        sample_params = self._get_sample_params()
        id_count = len(sample_params.get_sample_ids())
        any_selected = (sample_params.get_selected_sample_id() != None)
        self._import_button.setEnabled(id_count < sample_params.get_max_sample_count())
        self._remove_button.setEnabled(any_selected and (id_count > 0))

    def _import_sample(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_free_sample_id()
        if not sample_id:
            return

        filters = [
            'All supported types (*.wav *.aiff *.aif *.aifc *.au *.snd *.wv *.flac)',
            'Waveform Audio File Format (*.wav)',
            'Audio Interchange File Format (*.aiff *.aif *.aifc)',
            'Sun Au (*.au *.snd)',
            'WavPack (*.wv)',
            'Free Lossless Audio Codec (*.flac)',
        ]

        sample_path, _ = QFileDialog.getOpenFileName(
                caption='Import sample', filter=';;'.join(filters))
        if sample_path:
            try:
                sample_params.import_sample(sample_id, sample_path)
                self._updater.signal_update(set([
                    self._get_list_signal_type(),
                    self._get_note_map_random_list_signal_type(),
                    self._get_hit_map_random_list_signal_type()]))
            except SampleImportError as e:
                icon_bank = self._ui_model.get_icon_bank()
                error_msg_lines = str(e).split('\n')
                error_msg = '<p>{}</p>'.format('<br>'.join(error_msg_lines))
                dialog = ImportErrorDialog(icon_bank, error_msg)
                dialog.exec_()

    def _remove_sample(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        if sample_id:
            sample_params.remove_sample(sample_id)
            self._updater.signal_update(set([
                self._get_list_signal_type(),
                self._get_note_map_random_list_signal_type(),
                self._get_hit_map_random_list_signal_type()]))


class ImportErrorDialog(QDialog):

    def __init__(self, icon_bank, error_msg):
        super().__init__()

        error_img_path = icon_bank.get_icon_path('error')
        error_label = QLabel()
        error_label.setPixmap(QPixmap(error_img_path))

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setContentsMargins(8, 8, 8, 8)
        h.setSpacing(16)
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

        QObject.connect(self._ok_button, SIGNAL('clicked()'), self.close)


class SampleListModel(QAbstractListModel):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None

        self._items = []

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._make_items()

    def unregister_updaters(self):
        pass

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


class SampleListView(QListView):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._keyboard_mapper = KeyboardMapper()

        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        self._keyboard_mapper.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._keyboard_mapper.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _select_sample(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item:
            sample_id, _ = item
            sample_params = utils.get_proc_params(
                    self._ui_model, self._au_id, self._proc_id)
            sample_params.set_selected_sample_id(sample_id)
            self._updater.signal_update(set([self._get_update_signal_type()]))

    def setModel(self, model):
        super().setModel(model)

        selection_model = self.selectionModel()
        QObject.connect(
            selection_model,
            SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
            self._select_sample)

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


class SampleList(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._toolbar = SampleListToolBar()

        self._list_model = None
        self._list_view = SampleListView()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._toolbar.set_au_id(au_id)
        self._list_view.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._toolbar.set_proc_id(proc_id)
        self._list_view.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._toolbar.set_ui_model(ui_model)
        self._list_view.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_model()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_model()

    def _update_model(self):
        self._list_model = SampleListModel()
        self._list_model.set_au_id(self._au_id)
        self._list_model.set_proc_id(self._proc_id)
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)


class SampleEditor(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._name = QLineEdit()

        self._freq = QDoubleSpinBox()
        self._freq.setDecimals(0)
        self._freq.setRange(1, 2**32)

        self._loop_mode = QComboBox()
        loop_modes = (
                ('Off', 'off'),
                ('Unidirectional', 'uni'),
                ('Bidirectional', 'bi'))
        for vis_mode, mode in loop_modes:
            self._loop_mode.addItem(vis_mode, mode)

        self._loop_start = QSpinBox()
        self._loop_start.setRange(0, 2**30)
        self._loop_end = QSpinBox()
        self._loop_end.setRange(0, 2**30)
        self._length = QLabel('?')

        gl = QGridLayout()
        gl.setContentsMargins(0, 0, 0, 0)
        gl.setSpacing(2)
        gl.addWidget(QLabel('Name:'), 0, 0)
        gl.addWidget(self._name, 0, 1)
        gl.addWidget(QLabel('Middle frequency:'), 1, 0)
        gl.addWidget(self._freq, 1, 1)
        gl.addWidget(QLabel('Loop mode:'), 2, 0)
        gl.addWidget(self._loop_mode, 2, 1)
        gl.addWidget(QLabel('Loop start:'), 3, 0)
        gl.addWidget(self._loop_start, 3, 1)
        gl.addWidget(QLabel('Loop end:'), 4, 0)
        gl.addWidget(self._loop_end, 4, 1)
        gl.addWidget(QLabel('Length:'), 5, 0)
        gl.addWidget(self._length, 5, 1)

        self._sample_view = SampleView()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addLayout(gl)
        v.addWidget(self._sample_view)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._name, SIGNAL('editingFinished()'), self._change_name)
        QObject.connect(self._freq, SIGNAL('valueChanged(double)'), self._change_freq)

        QObject.connect(
                self._loop_mode,
                SIGNAL('currentIndexChanged(int)'),
                self._change_loop_mode)

        QObject.connect(
                self._loop_start, SIGNAL('valueChanged(int)'), self._change_loop_start)
        QObject.connect(
                self._loop_end, SIGNAL('valueChanged(int)'), self._change_loop_end)

        self._sample_view.set_icon_bank(self._ui_model.get_icon_bank())

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_list_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_selection_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _get_random_list_signal_type(self):
        return 'signal_sample_note_map_random_list_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_list_update_signal_type(),
            self._get_selection_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        has_sample = sample_id in sample_params.get_sample_ids()
        self.setEnabled(has_sample)

        name = ''
        if has_sample:
            name = sample_params.get_sample_name(sample_id) or ''

        old_block = self._name.blockSignals(True)
        if self._name.text() != name:
            self._name.setText(name)
        self._name.blockSignals(old_block)

        old_block = self._freq.blockSignals(True)
        new_freq = sample_params.get_sample_freq(sample_id)
        if self._freq.value() != new_freq:
            self._freq.setValue(new_freq)
        self._freq.blockSignals(old_block)

        old_block = self._loop_mode.blockSignals(True)
        new_loop_mode = sample_params.get_sample_loop_mode(sample_id)
        loop_mode_index = self._loop_mode.findData(new_loop_mode)
        if (loop_mode_index != self._loop_mode.itemData(self._loop_mode.currentIndex())):
            self._loop_mode.setCurrentIndex(loop_mode_index)
        self._loop_mode.blockSignals(old_block)

        sample_length = sample_params.get_sample_length(sample_id)

        old_block = self._loop_start.blockSignals(True)
        self._loop_start.setMaximum(sample_length)
        new_loop_start = sample_params.get_sample_loop_start(sample_id)
        if new_loop_start != self._loop_start.value():
            self._loop_start.setValue(new_loop_start)
        self._loop_start.blockSignals(old_block)

        old_block = self._loop_end.blockSignals(True)
        self._loop_end.setMaximum(sample_length)
        new_loop_end = sample_params.get_sample_loop_end(sample_id)
        if new_loop_end != self._loop_end.value():
            self._loop_end.setValue(new_loop_end)
        self._loop_end.blockSignals(old_block)

        self._length.setText(str(sample_length))

        get_sample_data = sample_params.get_sample_data_retriever(sample_id)
        self._sample_view.set_sample(sample_length, get_sample_data)

    def _change_name(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_name(sample_id, str(self._name.text()))
        self._updater.signal_update(set([
            self._get_list_update_signal_type(), self._get_random_list_signal_type()]))

    def _change_freq(self, value):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_freq(sample_id, value)
        self._updater.signal_update(set([self._get_list_update_signal_type()]))

    def _change_loop_mode(self, item_index):
        loop_mode = self._loop_mode.itemData(item_index)
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_loop_mode(sample_id, loop_mode)
        self._updater.signal_update(set([self._get_list_update_signal_type()]))

    def _change_loop_start(self, start):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_loop_start(sample_id, start)
        self._updater.signal_update(set([self._get_list_update_signal_type()]))

    def _change_loop_end(self, end):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_loop_end(sample_id, end)
        self._updater.signal_update(set([self._get_list_update_signal_type()]))


