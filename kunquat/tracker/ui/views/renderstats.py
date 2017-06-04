# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2017
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from PySide.QtCore import *
from PySide.QtGui import *

from .axisrenderer import HorizontalAxisRenderer, VerticalAxisRenderer
from .profilecontrol import ProfileControl
from .updater import Updater


class RenderStats(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._stat_manager = None

        self._render_load_history = RenderLoadHistory()
        self._ui_load_history = UILoadHistory()
        self._output_speed = QLabel(self)
        self._render_speed = QLabel(self)
        self._render_load = QLabel(self)
        self._ui_load = QLabel(self)

        self._profile_control = ProfileControl()

        self.setFocusPolicy(Qt.StrongFocus)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(2)
        v.addWidget(QLabel('Audio rendering load:'))
        v.addWidget(self._render_load_history)
        v.addSpacing(4)
        v.addWidget(QLabel('UI load:'))
        v.addWidget(self._ui_load_history)
        v.addWidget(self._output_speed)
        v.addWidget(self._render_speed)
        v.addWidget(self._render_load)
        v.addWidget(self._ui_load)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        updater = ui_model.get_updater()
        updater.register_updater(self.perform_updates)
        self._stat_manager = ui_model.get_stat_manager()
        self._render_load_history.set_ui_model(ui_model)
        self._ui_load_history.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ui_load_history.unregister_updaters()
        self._render_load_history.unregister_updaters()
        updater = self._ui_model.get_updater()
        updater.unregister_updater(self.perform_updates)

    def update_output_speed(self):
        output_speed = self._stat_manager.get_output_speed()
        text = 'output speed: {} fps'.format(int(output_speed))
        self._output_speed.setText(text)

    def update_render_speed(self):
        output_speed = self._stat_manager.get_render_speed()
        text = 'render speed: {} fps'.format(int(output_speed))
        self._render_speed.setText(text)

    def update_render_load(self):
        render_load = self._stat_manager.get_render_load()
        text = 'render load: {} %'.format(int(render_load * 100))
        self._render_load.setText(text)

    def update_ui_load(self):
        ui_load = self._stat_manager.get_ui_load()
        text = 'ui load: {} %'.format(int(ui_load * 100))
        self._ui_load.setText(text)

    def perform_updates(self, signals):
        self.update_output_speed()
        self.update_render_speed()
        self.update_render_load()
        self.update_ui_load()

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            self._profile_control.show()


_font = QFont(QFont().defaultFamily(), 9)
_font.setWeight(QFont.Bold)


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


UI_LOAD_HISTORY_CONFIG = {
    'padding'        : 5,
    'bg_colour'      : QColor(0, 0, 0),
    'max_line_colour': QColor(0x44, 0xcc, 0xff),
    'avg_line_colour': QColor(0x22, 0x88, 0xaa),
    'line_thickness' : 1.3,
}


class LoadHistory(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._max_vis_history = []
        self._avg_vis_history = []
        self._max_curve_path = None
        self._avg_curve_path = None

        self._step_width = 8

        self._axis_x_renderer = HorizontalAxisRenderer()
        self._axis_y_renderer = VerticalAxisRenderer()

        self._config = {}
        self._set_configs({}, {})

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_history)
        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_history()

    def _update_history(self):
        load_peaks = self._get_peaks()
        load_avgs = self._get_averages()

        # TODO: see if we need to reuse old parts
        self._max_vis_history = load_peaks
        self._avg_vis_history = load_avgs

        if len(self._max_vis_history) >= 2:
            self._max_curve_path = QPainterPath()

            '''
            self._max_curve_path.moveTo(0, 0)
            for i in range(1, 21):
                self._max_curve_path.lineTo(i, (i // 2) % 2)
            '''

            self._max_curve_path.moveTo(0, self._max_vis_history[-1])
            for i, load in enumerate(reversed(self._max_vis_history[:-1])):
                self._max_curve_path.lineTo(i + 1, load)
        else:
            self._max_curve_path = None

        if len(self._avg_vis_history) >= 2:
            self._avg_curve_path = QPainterPath()
            self._avg_curve_path.moveTo(0, self._avg_vis_history[-1])
            for i, load in enumerate(reversed(self._avg_vis_history[:-1])):
                self._avg_curve_path.lineTo(i + 1, load)
        else:
            self._avg_curve_path = None

        self.update()

    def _update_style(self):
        style_manager = self._ui_model.get_style_manager()
        if not style_manager.is_custom_style_enabled():
            self._set_configs({}, {})
            self.update()
            return

        def get_colour(name):
            return QColor(style_manager.get_style_param(name))

        # Note: using waveform colours to avoid additional clutter in the style config
        config = {
            'bg_colour': get_colour('waveform_bg_colour'),
            'max_line_colour': get_colour('waveform_single_item_colour'),
            'avg_line_colour': get_colour('waveform_interpolated_colour'),
        }

        axis_config = {
            'label_colour': get_colour('envelope_axis_label_colour'),
            'line_colour': get_colour('envelope_axis_line_colour'),
        }

        self._set_configs(config, axis_config)
        self.update()

    def _set_configs(self, config, axis_config):
        self._config = UI_LOAD_HISTORY_CONFIG.copy()
        self._config.update(config)

        self._axis_config = AXIS_CONFIG.copy()
        self._axis_config.update(axis_config)
        self._axis_x_renderer.set_config(self._axis_config, self)
        self._axis_y_renderer.set_config(self._axis_config, self)

        self._flush_vis()

    def _flush_vis(self):
        self._max_vis_history = []
        self._avg_vis_history = []
        self._axis_x_renderer.flush_cache()
        self._axis_y_renderer.flush_cache()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        padding = self._config['padding']

        axis_y_width = self._axis_config['axis_y']['width']
        axis_x_height = self._axis_config['axis_x']['height']
        area_rect = QRect(
                padding,
                padding,
                self.width() - padding * 2,
                self.height() - padding - axis_x_height)
        if not area_rect.isValid():
            return

        x_width = area_rect.width() / self._step_width

        # Axes
        painter.save()
        painter.translate(QPoint(0, area_rect.y() + area_rect.height() - 1))
        self._axis_x_renderer.set_width(self.width())
        self._axis_x_renderer.set_x_offset(area_rect.x())
        self._axis_x_renderer.set_axis_length(area_rect.width())
        self._axis_x_renderer.set_val_range((-x_width, 0))
        self._axis_x_renderer.set_draw_zero_marker_enabled(True)
        self._axis_x_renderer.render(painter)
        painter.restore()

        painter.save()
        painter.translate(QPoint(padding + area_rect.width() - axis_y_width, 0))
        self._axis_y_renderer.set_height(self.height())
        self._axis_y_renderer.set_padding(padding)
        self._axis_y_renderer.set_x_offset_y(area_rect.y() + area_rect.height() - 1)
        self._axis_y_renderer.set_axis_length(area_rect.height())
        self._axis_y_renderer.set_val_range((0, 1))
        self._axis_y_renderer.set_draw_zero_marker_enabled(False)
        self._axis_y_renderer.render(painter)
        painter.restore()

        # Curves
        if self._max_curve_path or self._avg_curve_path:
            painter.save()

            tfm = QTransform()
            tfm.translate(
                    padding + area_rect.width() - 1 + 0.5,
                    padding + area_rect.height() - 1 + 0.5)
            tfm.scale(-self._step_width, -area_rect.height() + 1)
            painter.setRenderHint(QPainter.Antialiasing)

            clip_rect = QRect(padding, 0, 1, self.height()).united(area_rect)
            painter.setClipRect(clip_rect)
            painter.setTransform(tfm)
            pen = QPen()
            pen.setCosmetic(True)
            pen.setWidthF(self._config['line_thickness'])

            if self._avg_curve_path:
                pen.setColor(QColor(self._config['avg_line_colour']))
                painter.setPen(pen)
                painter.drawPath(self._avg_curve_path)

            if self._max_curve_path:
                pen.setColor(QColor(self._config['max_line_colour']))
                painter.setPen(pen)
                painter.drawPath(self._max_curve_path)

            painter.restore()

        end = time.time()
        elapsed = end - start
        #print('Load history updated in {:.2f} ms'.format(elapsed * 1000))

    # Protected callbacks

    def _get_update_signal_type(self):
        raise NotImplementedError

    def _get_peaks(self):
        raise NotImplementedError

    def _get_averages(self):
        raise NotImplementedError


class RenderLoadHistory(LoadHistory):

    def __init__(self):
        super().__init__()

    def _get_update_signal_type(self):
        return 'signal_load_history'

    def _get_peaks(self):
        stat_manager = self._ui_model.get_stat_manager()
        return stat_manager.get_render_load_peaks()

    def _get_averages(self):
        stat_manager = self._ui_model.get_stat_manager()
        return stat_manager.get_render_load_averages()


class UILoadHistory(LoadHistory):

    def __init__(self):
        super().__init__()

    def _get_update_signal_type(self):
        return 'signal_load_history'

    def _get_peaks(self):
        stat_manager = self._ui_model.get_stat_manager()
        return stat_manager.get_ui_load_peaks()

    def _get_averages(self):
        stat_manager = self._ui_model.get_stat_manager()
        return stat_manager.get_ui_load_averages()


