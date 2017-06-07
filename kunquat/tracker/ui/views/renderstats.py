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
        self._output_speed = QLabel()
        self._render_speed = QLabel()
        self._render_load = QLabel()
        self._ui_load = QLabel()

        self._render_load_container = LoadHistoryContainer(self._render_load_history)
        self._ui_load_container = LoadHistoryContainer(self._ui_load_history)

        self._profile_control = ProfileControl()

        self.setFocusPolicy(Qt.StrongFocus)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(2)
        v.addWidget(QLabel('Audio rendering load:'))
        v.addWidget(self._render_load_container)
        v.addSpacing(4)
        v.addWidget(QLabel('UI load:'))
        v.addWidget(self._ui_load_container)
        '''
        v.addWidget(self._output_speed)
        v.addWidget(self._render_speed)
        v.addWidget(self._render_load)
        v.addWidget(self._ui_load)
        '''
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        updater = ui_model.get_updater()
        updater.register_updater(self.perform_updates)
        self._stat_manager = ui_model.get_stat_manager()
        self._render_load_history.set_ui_model(ui_model)
        self._ui_load_history.set_ui_model(ui_model)

        icon_bank = self._ui_model.get_icon_bank()
        self._render_load_container.set_icon_bank(icon_bank)
        self._ui_load_container.set_icon_bank(icon_bank)

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


class LoadHistoryContainer(QWidget):

    def __init__(self, load_history):
        super().__init__()
        self._load_history = load_history

        self._zoom_levels = [2**x for x in range(5)]
        self._zoom_level_index = self._zoom_levels.index(8)

        self._zoom_in_button = QToolButton()
        self._zoom_out_button = QToolButton()

        self._toolbar = QToolBar()
        self._toolbar.setOrientation(Qt.Vertical)
        self._toolbar.addWidget(self._zoom_in_button)
        self._toolbar.addWidget(self._zoom_out_button)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(0)
        h.addWidget(self._toolbar)
        h.addWidget(self._load_history)
        self.setLayout(h)

        QObject.connect(self._zoom_in_button, SIGNAL('clicked()'), self._zoom_in)
        QObject.connect(self._zoom_out_button, SIGNAL('clicked()'), self._zoom_out)

        self._update_step_width()

    def set_icon_bank(self, icon_bank):
        self._zoom_in_button.setIcon(QIcon(icon_bank.get_icon_path('zoom_in')))
        self._zoom_out_button.setIcon(QIcon(icon_bank.get_icon_path('zoom_out')))

    def _update_enabled_buttons(self):
        self._zoom_in_button.setEnabled(
                self._zoom_level_index < len(self._zoom_levels) - 1)
        self._zoom_out_button.setEnabled(self._zoom_level_index > 0)

    def _update_step_width(self):
        self._load_history.set_step_width(self._zoom_levels[self._zoom_level_index])
        self._update_enabled_buttons()

    def _zoom_in(self):
        self._zoom_level_index = min(
                self._zoom_level_index + 1, len(self._zoom_levels) - 1)
        self._update_step_width()

    def _zoom_out(self):
        self._zoom_level_index = max(0, self._zoom_level_index - 1)
        self._update_step_width()


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
    'padding'           : 5,
    'bg_colour'         : QColor(0, 0, 0),
    'max_line_colour'   : QColor(0x44, 0xcc, 0xff),
    'avg_line_colour'   : QColor(0x22, 0x88, 0xaa),
    'line_thickness'    : 1.3,
    'legend_margin'     : 10,
    'legend_padding'    : 5,
    'legend_line_length': 45,
    'legend_font'       : _font,
    'legend_text_colour': QColor(0xcc, 0xcc, 0xcc),
}


class LoadHistory(QWidget, Updater):

    _PIXMAP_TARGET_WIDTH = 128

    def __init__(self):
        super().__init__()

        self._max_vis_history = []
        self._avg_vis_history = []
        self._max_curve_path = None
        self._avg_curve_path = None

        self._step_width = 8

        self._axis_x_renderer = HorizontalAxisRenderer()
        self._axis_y_renderer = VerticalAxisRenderer()

        self._graph_images = {}

        self._legend_image = None

        self._config = {}
        self._set_configs({}, {})

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_step_width(self, step_width):
        self._step_width = step_width
        self._flush_vis()
        self.update()

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self.update)
        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
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
            'legend_text_colour': get_colour('envelope_axis_label_colour'),
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
        self._graph_images = {}
        self._legend_image = None

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
        ls_per_pixmap = self._PIXMAP_TARGET_WIDTH // self._step_width
        load_peaks = self._get_peaks()
        load_avgs = self._get_averages()
        assert len(load_avgs) == len(load_peaks)
        full_pixmap_count = (len(load_peaks) - 1) // ls_per_pixmap
        rem_ls_count = max(len(load_peaks) - 1, 0) % ls_per_pixmap
        if rem_ls_count > 0:
            # End of the curves
            rem_point_count = rem_ls_count + 1

            avg_path = QPainterPath()
            avg_path.moveTo(0, load_avgs[-rem_point_count])
            for i, load in enumerate(load_avgs[-rem_point_count + 1:]):
                avg_path.lineTo(i + 1, load)

            peak_path = QPainterPath()
            peak_path.moveTo(0, load_peaks[-rem_point_count])
            for i, load in enumerate(load_peaks[-rem_point_count + 1:]):
                peak_path.lineTo(i + 1, load)

            painter.save()
            clip_rect = QRect(padding, 0, 1, self.height()).united(area_rect)
            painter.setClipRect(clip_rect)
            painter.setRenderHint(QPainter.Antialiasing)

            tfm = QTransform()
            path_start = -self._step_width * (rem_point_count - 1)
            tfm.translate(
                    padding + area_rect.width() - 1 + 0.5 + path_start,
                    padding + area_rect.height() - 1 + 0.5)
            tfm.scale(self._step_width, -area_rect.height() + 1)
            painter.setTransform(tfm)

            pen = QPen()
            pen.setCosmetic(True)
            pen.setWidthF(self._config['line_thickness'])

            pen.setColor(QColor(self._config['avg_line_colour']))
            painter.setPen(pen)
            painter.drawPath(avg_path)

            pen.setColor(QColor(self._config['max_line_colour']))
            painter.setPen(pen)
            painter.drawPath(peak_path)

            painter.restore()

        for i in reversed(range(full_pixmap_count)):
            # Cached curves
            x_stop = area_rect.right() - (
                    (rem_ls_count + (full_pixmap_count - i - 1) * ls_per_pixmap) *
                    self._step_width)
            if x_stop < area_rect.left():
                break
            x_start = x_stop - ls_per_pixmap * self._step_width

            image_key = i * ls_per_pixmap
            if image_key not in self._graph_images:
                image_width = ls_per_pixmap * self._step_width
                image = QImage(
                        image_width,
                        padding + area_rect.height(),
                        QImage.Format_ARGB32)
                image.fill(0)
                img_painter = QPainter(image)
                img_painter.setRenderHint(QPainter.Antialiasing)

                left_index = i * ls_per_pixmap
                right_index = (i + 1) * ls_per_pixmap
                first_index = max(0, left_index - 1)
                last_index = min(len(load_peaks) - 1, right_index + 1)

                avg_path = QPainterPath()
                avg_path.moveTo(first_index - left_index, load_avgs[first_index])
                for k, load in enumerate(load_avgs[first_index + 1 : last_index + 1]):
                    avg_path.lineTo(k + (first_index - left_index) + 1, load)

                peak_path = QPainterPath()
                peak_path.moveTo(first_index - left_index, load_peaks[first_index])
                for k, load in enumerate(load_peaks[first_index + 1 : last_index + 1]):
                    peak_path.lineTo(k + (first_index - left_index) + 1, load)

                pen = QPen()
                pen.setCosmetic(True)
                pen.setWidthF(self._config['line_thickness'])

                tfm = QTransform()
                tfm.translate(0.5, padding + area_rect.height() - 1 + 0.5)
                tfm.scale(self._step_width, -area_rect.height() + 1)
                img_painter.setTransform(tfm)

                pen.setColor(QColor(self._config['avg_line_colour']))
                img_painter.setPen(pen)
                img_painter.drawPath(avg_path)

                pen.setColor(QColor(self._config['max_line_colour']))
                img_painter.setPen(pen)
                img_painter.drawPath(peak_path)

                #img_painter.setPen(QColor('#fff'))
                #img_painter.drawRect(0, 0, image.width() - 1, image.height() - 1)

                img_painter.end()
                self._graph_images[image_key] = image

            clip_rect = QRect(padding, 0, 1, self.height()).united(area_rect)
            painter.setClipRect(clip_rect)
            painter.drawImage(x_start, 0, self._graph_images[image_key])

        # Legend
        if not self._legend_image:
            margin = self._config['legend_margin']
            padding = self._config['legend_padding']
            fm = QFontMetrics(self._config['legend_font'], self)

            line_length = self._config['legend_line_length']
            peaks_text = 'Peak'
            avgs_text = 'Average'

            baseline_offset = fm.tightBoundingRect('A').height()
            strike_offset = fm.strikeOutPos()

            text_width_reserve = max(
                    fm.tightBoundingRect(peaks_text).width(),
                    fm.tightBoundingRect(avgs_text).width())

            width = margin + line_length + padding + text_width_reserve + margin

            text_height = fm.tightBoundingRect('Ag').height()
            height = padding + text_height + padding + text_height + padding

            self._legend_image = QImage(width, height, QImage.Format_ARGB32)
            self._legend_image.fill(0)
            img_painter = QPainter(self._legend_image)

            # Background
            legend_bg_colour = QColor(self._config['bg_colour'])
            legend_bg_colour.setAlpha(0xcc)
            img_painter.setBackground(legend_bg_colour)
            img_painter.eraseRect(
                    0, 0, self._legend_image.width(), self._legend_image.height())

            # Lines
            img_painter.translate(margin, 0)
            pen = QPen()
            pen.setCosmetic(True)
            pen.setWidthF(self._config['line_thickness'])

            pen.setColor(self._config['max_line_colour'])
            img_painter.setPen(pen)
            offset_y = padding + strike_offset
            img_painter.drawLine(0, offset_y, line_length, offset_y)

            pen.setColor(self._config['avg_line_colour'])
            img_painter.setPen(pen)
            offset_y = padding + text_height + padding + strike_offset
            img_painter.drawLine(0, offset_y, line_length, offset_y)

            # Texts
            img_painter.translate(line_length + padding, 0)
            img_painter.setPen(self._config['legend_text_colour'])
            offset_y = padding + baseline_offset
            img_painter.drawText(QPoint(0, offset_y), peaks_text)
            offset_y = padding + text_height + padding + baseline_offset
            img_painter.drawText(QPoint(0, offset_y), avgs_text)

            img_painter.end()

        painter.setClipRect(area_rect)
        painter.drawImage(area_rect.topLeft(), self._legend_image)

        end = time.time()
        elapsed = end - start
        #print('Load history updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, event):
        self._flush_vis()
        self.update()

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


