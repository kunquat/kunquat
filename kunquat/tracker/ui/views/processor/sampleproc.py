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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.tracker.ui.views.audio_unit.hitselector import HitSelector
from kunquat.tracker.ui.views.axisrenderer import HorizontalAxisRenderer, VerticalAxisRenderer
from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper
from kunquat.tracker.ui.views.utils import lerp_val
import utils


class SampleProc(QTabWidget):

    @staticmethod
    def get_name():
        return u'Sample synthesis'

    def __init__(self):
        QTabWidget.__init__(self)

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
        QWidget.__init__(self)

        self._note_map = NoteMap()
        self._note_map_entry = NoteMapEntry()

        h = QHBoxLayout()
        h.setMargin(2)
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


class NoteMap(QWidget):

    _DEFAULT_CONFIG = {
        'padding'                  : 5,
        'bg_colour'                : QColor(0, 0, 0),
        'point_colour'             : QColor(0xee, 0xcc, 0xaa),
        'focused_point_colour'     : QColor(0xff, 0x77, 0x22),
        'focused_point_axis_colour': QColor(0xff, 0x77, 0x22, 0x7f),
        'point_size'               : 7,
        'point_focus_dist_max'     : 5,
        'selected_highlight_colour': QColor(0xff, 0xff, 0xdd),
        'selected_highlight_size'  : 11,
        'selected_highlight_width' : 2,
        'move_snap_dist'           : 10,
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
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._axis_x_renderer = HorizontalAxisRenderer()
        self._axis_x_renderer.set_config(self._AXIS_CONFIG, self)
        self._axis_x_renderer.set_val_range([-36, 0])

        self._axis_y_renderer = VerticalAxisRenderer()
        self._axis_y_renderer.set_config(self._AXIS_CONFIG, self)
        self._axis_y_renderer.set_val_range([-6000, 6000])

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

    def _get_selection_signal_type(self):
        return 'signal_sample_note_map_selection_{}'.format(self._proc_id)

    def _get_move_signal_type(self):
        return 'signal_sample_note_map_move_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_selection_signal_type() in signals:
            self.update()
        if self._get_move_signal_type() in signals:
            self.update()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_area_offset(self):
        padding = self._config['padding']
        offset_x = padding + self._axis_y_renderer.get_width() - 1
        offset_y = padding
        return offset_x, offset_y

    def _map_range(self, val, src_range, target_range):
        start_diff = val - src_range[0]
        pos_norm = (val - src_range[0]) / float(src_range[1] - src_range[0])
        return lerp_val(target_range[0], target_range[1], min(max(0, pos_norm), 1))

    def _get_vis_coords(self, point):
        cents, dB = point

        cents_range = self._axis_y_renderer.get_val_range()
        y_range = self._axis_y_renderer.get_axis_length() - 1, 0
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

        y_range = self._axis_y_renderer.get_axis_length() - 1, 0
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
        sample_params = self._get_sample_params()

        nearest_dist = float('inf')
        nearest_point = None
        for point in sample_params.get_note_map_points():
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
            sample_params = self._get_sample_params()
            point = sample_params.get_selected_note_map_point()
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
                    sample_params.remove_note_map_point(point)
                    sample_params.set_selected_note_map_point(None)
                    self._focused_point = None
                    self._state = self._STATE_IDLE
                    self._updater.signal_update(set([self._get_selection_signal_type()]))
                    return

                if self._is_start_snapping_active:
                    if dist >= self._config['move_snap_dist']:
                        self._is_start_snapping_active = False

                if not self._is_start_snapping_active:
                    new_point = self._get_point_coords((adjusted_x, adjusted_y))
                    if new_point not in sample_params.get_note_map_points():
                        sample_params.move_note_map_point(point, new_point)
                        sample_params.set_selected_note_map_point(new_point)
                        self._focused_point = new_point
                        self._updater.signal_update(set([self._get_move_signal_type()]))

            else:
                self._state = self._STATE_IDLE

    def mousePressEvent(self, event):
        if self._state == self._STATE_IDLE:
            x, y = event.x(), event.y()
            point, dist = self._get_nearest_point_with_dist(x - 1, y - 1)
            if dist <= self._config['point_focus_dist_max']:
                sample_params = self._get_sample_params()
                sample_params.set_selected_note_map_point(point)

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
                    sample_params = self._get_sample_params()
                    sample_params.add_note_map_point(new_point)
                    sample_params.set_selected_note_map_point(new_point)

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
        sample_params = self._get_sample_params()
        point_size = self._config['point_size']
        point_offset = -(point_size // 2)
        selected_point = sample_params.get_selected_note_map_point()
        selected_found = False
        for point in sample_params.get_note_map_points():
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


class TightLabel(QLabel):

    def __init__(self, text):
        QLabel.__init__(self, text)
        self.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)


class NoteMapEntry(QWidget):

    def __init__(self):
        QWidget.__init__(self)
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

        self._random_list = RandomList()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(TightLabel('Pitch:'))
        h.addWidget(self._pitch)
        h.addSpacing(2)
        h.addWidget(TightLabel('Force:'))
        h.addWidget(self._force)

        v = QVBoxLayout()
        v.setMargin(0)
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
            new_point = self._pitch.value(), self._force.value()
            if new_point not in sample_params.get_note_map_points():
                sample_params.move_note_map_point(selected_point, new_point)
                sample_params.set_selected_note_map_point(new_point)
                self._updater.signal_update(set([self._get_move_signal_type()]))


class RandomList(EditorList):

    def __init__(self):
        EditorList.__init__(self)
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

    def _make_adder_widget(self):
        self._adder = RandomEntryAdder()
        self._adder.set_au_id(self._au_id)
        self._adder.set_proc_id(self._proc_id)
        self._adder.set_ui_model(self._ui_model)
        return self._adder

    def _get_updated_editor_count(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        point = sample_params.get_selected_note_map_point()
        if point and (point in sample_params.get_note_map_points()):
            return sample_params.get_note_map_random_list_length(point)
        return 0

    def _make_editor_widget(self, index):
        editor = RandomEntryEditor(index)
        editor.set_au_id(self._au_id)
        editor.set_proc_id(self._proc_id)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _get_selection_signal_type(self):
        return 'signal_sample_note_map_selection_{}'.format(self._proc_id)

    def _get_random_list_signal_type(self):
        return 'signal_sample_note_map_random_list_'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_selection_signal_type(),
            self._get_random_list_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        self.update_list()

        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        point = sample_params.get_selected_note_map_point()
        if point and (point in sample_params.get_note_map_points()):
            is_full = sample_params.is_note_map_random_list_full(point)
            self._adder.setVisible(not is_full)
        else:
            self._adder.setVisible(False)

        self._adder.setEnabled(len(sample_params.get_sample_ids()) > 0)


class RandomEntryAdder(QPushButton):

    def __init__(self):
        QPushButton.__init__(self, 'Add sample entry')
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

        QObject.connect(self, SIGNAL('clicked()'), self._add_entry)

    def unregister_updaters(self):
        pass

    def _get_update_signal_type(self):
        return 'signal_sample_note_map_random_list_'.format(self._proc_id)

    def _add_entry(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        point = sample_params.get_selected_note_map_point()
        if point and (point in sample_params.get_note_map_points()):
            sample_params.add_note_map_random_list_entry(point)
            self._updater.signal_update(set([self._get_update_signal_type()]))


class RandomEntryEditor(QWidget):

    def __init__(self, index):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

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
        h.setMargin(0)
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

    def _get_selection_signal_type(self):
        return 'signal_sample_note_map_selection_{}'.format(self._proc_id)

    def _get_list_signal_type(self):
        return 'signal_sample_note_map_random_list_'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_selection_signal_type(), self._get_list_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_sample_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        sample_params = self._get_sample_params()

        sample_ids = sample_params.get_sample_ids()

        point = sample_params.get_selected_note_map_point()
        cur_sample_id = None
        if point and (point in sample_params.get_note_map_points()):
            if self._index >= sample_params.get_note_map_random_list_length(point):
                # We are being removed
                return

            cur_sample_id = sample_params.get_note_map_random_list_sample_id(
                    point, self._index)

        # Update sample selector contents
        sample_info = sorted([
            (sid, sample_params.get_sample_name(sid)) for sid in sample_ids])

        old_block = self._sample_selector.blockSignals(True)
        self._sample_selector.clear()
        self._sample_selector.setEnabled(len(sample_ids) > 0)
        for i, info in enumerate(sample_info):
            sample_id, sample_name = info
            vis_name = sample_name or u'-'
            self._sample_selector.addItem(vis_name, QVariant(sample_id))
            if sample_id == cur_sample_id:
                self._sample_selector.setCurrentIndex(i)
        self._sample_selector.blockSignals(old_block)

        if point and (point in sample_params.get_note_map_points()):
            # Update pitch shift
            old_block = self._pitch_shift.blockSignals(True)
            new_pitch_shift = sample_params.get_note_map_random_list_cents_offset(
                    point, self._index)
            if new_pitch_shift != self._pitch_shift.value():
                self._pitch_shift.setValue(new_pitch_shift)
            self._pitch_shift.blockSignals(old_block)

            # Update volume shift
            old_block = self._volume_shift.blockSignals(True)
            new_volume_shift = sample_params.get_note_map_random_list_volume_adjust(
                    point, self._index)
            if new_volume_shift != self._volume_shift.value():
                self._volume_shift.setValue(new_volume_shift)
            self._volume_shift.blockSignals(old_block)

    def _change_sample(self, item_index):
        sample_params = self._get_sample_params()
        point = sample_params.get_selected_note_map_point()
        if point and (point in sample_params.get_note_map_points()):
            sample_id = unicode(self._sample_selector.itemData(item_index).toString())
            sample_params.set_note_map_random_list_sample_id(
                    point, self._index, sample_id)
            self._updater.signal_update(set([self._get_list_signal_type()]))

    def _change_pitch_shift(self, value):
        sample_params = self._get_sample_params()
        point = sample_params.get_selected_note_map_point()
        if point and (point in sample_params.get_note_map_points()):
            sample_params.set_note_map_random_list_cents_offset(
                    point, self._index, value)
            self._updater.signal_update(set([self._get_list_signal_type()]))

    def _change_volume_shift(self, value):
        sample_params = self._get_sample_params()
        point = sample_params.get_selected_note_map_point()
        if point and (point in sample_params.get_note_map_points()):
            sample_params.set_note_map_random_list_volume_adjust(
                    point, self._index, value)
            self._updater.signal_update(set([self._get_list_signal_type()]))

    def _remove_entry(self):
        sample_params = self._get_sample_params()
        point = sample_params.get_selected_note_map_point()
        if point and (point in sample_params.get_note_map_points()):
            sample_params.remove_note_map_random_list_entry(point, self._index)
            self._updater.signal_update(set([self._get_list_signal_type()]))


class HitMapEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._hit_selector = SampleHitSelector()

        v = QVBoxLayout()
        v.setMargin(2)
        v.setSpacing(2)
        v.addWidget(self._hit_selector)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._hit_selector.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._hit_selector.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._hit_selector.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._hit_selector.unregister_updaters()


class SampleHitSelector(HitSelector):

    def __init__(self):
        HitSelector.__init__(self)
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


class Samples(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._sample_list = SampleList()
        self._sample_editor = SampleEditor()

        h = QHBoxLayout()
        h.setMargin(2)
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
        QToolBar.__init__(self)
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

        sample_path_qstring = QFileDialog.getOpenFileName(
                caption='Import sample', filter='WavPack audio (*.wv)')
        if sample_path_qstring:
            sample_path = str(sample_path_qstring.toUtf8())
            success = sample_params.import_sample(sample_id, sample_path)
            if success:
                self._updater.signal_update(set([self._get_list_signal_type()]))
            else:
                icon_bank = self._ui_model.get_icon_bank()
                # TODO: Add a more descriptive error message
                error_msg = u'<p>Could not import \'{}\'.</p>'.format(sample_path)
                dialog = ImportErrorDialog(icon_bank, error_msg)
                dialog.exec_()

    def _remove_sample(self):
        sample_params = self._get_sample_params()
        sample_id = sample_params.get_selected_sample_id()
        if sample_id:
            sample_params.remove_sample(sample_id)
            self._updater.signal_update(set([
                self._get_list_signal_type(),
                'signal_sample_note_map_random_list_'.format(self._proc_id)]))


class ImportErrorDialog(QDialog):

    def __init__(self, icon_bank, error_msg):
        QDialog.__init__(self)

        error_img_path = icon_bank.get_icon_path('warning') # TODO: error icon
        error_label = QLabel()
        error_label.setPixmap(QPixmap(error_img_path))

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setMargin(8)
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
        QAbstractListModel.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

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
        self._items = sorted(self._items, key=lambda x: x[1])

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
                return QVariant(vis_name)

        return QVariant()

    def headerData(self, section, orientation, role):
        return QVariant()


class SampleListView(QListView):

    def __init__(self):
        QListView.__init__(self)
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

        for signal_type in ('clicked', 'activated'):
            signal = '{}(const QModelIndex&)'.format(signal_type)
            QObject.connect(self, SIGNAL(signal), self._select_sample)

    def unregister_updaters(self):
        self._keyboard_mapper.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _select_sample(self, index):
        item = self.model().get_item(index)
        if item:
            sample_id, _ = item
            sample_params = utils.get_proc_params(
                    self._ui_model, self._au_id, self._proc_id)
            sample_params.set_selected_sample_id(sample_id)
            self._updater.signal_update(set([self._get_update_signal_type()]))

    def keyPressEvent(self, event):
        if self._keyboard_mapper.is_handled_key(event):
            event.ignore()
        else:
            QListView.keyPressEvent(self, event)

    def keyReleaseEvent(self, event):
        if self._keyboard_mapper.is_handled_key(event):
            event.ignore()
        else:
            QListView.keyReleaseEvent(self, event)


class SampleList(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._toolbar = SampleListToolBar()

        self._list_model = None
        self._list_view = SampleListView()

        v = QVBoxLayout()
        v.setMargin(0)
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
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._name_editor = QLineEdit()

        gl = QGridLayout()
        gl.setMargin(0)
        gl.setSpacing(2)
        gl.addWidget(QLabel('Name:'), 0, 0)
        gl.addWidget(self._name_editor, 0, 1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addLayout(gl)
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

        QObject.connect(
                self._name_editor, SIGNAL('editingFinished()'), self._change_name)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_list_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_selection_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _get_random_list_signal_type(self):
        return 'signal_sample_note_map_random_list_'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_list_update_signal_type(),
            self._get_selection_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        sample_id = sample_params.get_selected_sample_id()
        has_sample = sample_id in sample_params.get_sample_ids()
        self.setEnabled(has_sample)

        name = u''
        if has_sample:
            name = sample_params.get_sample_name(sample_id) or u''

        old_block = self._name_editor.blockSignals(True)
        if self._name_editor.text() != name:
            self._name_editor.setText(name)
        self._name_editor.blockSignals(old_block)

    def _change_name(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_name(sample_id, unicode(self._name_editor.text()))
        self._updater.signal_update(set([
            self._get_list_update_signal_type(), self._get_random_list_signal_type()]))


