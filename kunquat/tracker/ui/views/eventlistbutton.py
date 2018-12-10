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

from kunquat.tracker.ui.qt import *

from . import utils
from .updater import Updater


_LIGHT_CONFIG = {
        'width'     : 12,
        'height'    : 12,
        'delta'     : 0.02,
        'fade_time' : 0.25,
        'colour_on' : QColor(0xff, 0x33, 0),
        'colour_off': QColor(0x44, 0, 0),
    }


class EventLight(QWidget):

    def __init__(self):
        super().__init__()
        self._config = None
        self.set_config({})

        self._light_norm = 0

        self._refresh_timer = QTimer()
        self._refresh_timer.timeout.connect(self._update_vis)
        self._refresh_timer.start(self._config['delta'] * 1000)

    def set_config(self, config):
        self._config = _LIGHT_CONFIG.copy()
        self._config.update(config)

    def do_flash(self):
        self._light_norm = 1

    def _update_vis(self):
        if self._light_norm == 0:
            return
        self._light_norm = max(
                0, self._light_norm -
                    self._config['delta'] / self._config['fade_time'])
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        colour = utils.lerp_colour(
                self._config['colour_off'],
                self._config['colour_on'],
                self._light_norm)
        painter.fillRect(0, 0, self._config['width'], self._config['height'], colour)

    def sizeHint(self):
        return QSize(self._config['width'], self._config['height'])


class EventListButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self._last_event_count = 0

        self._light = EventLight()
        light_layout = QWidgetItem(self._light)
        light_layout.setAlignment(Qt.AlignVCenter)
        self._text = QLabel(self._get_text(0))
        self._text.setMargin(0)

        h = QHBoxLayout()
        h.setContentsMargins(6, 4, 6, 3)
        h.addItem(light_layout)
        h.addWidget(self._text)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_event_log_updated', self._update_event_count)
        self.register_action('signal_style_changed', self._update_style)

        self.clicked.connect(self._clicked)

        self._update_event_count()
        self._update_style()

    def _clicked(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.show_event_log()

    def _get_text(self, count):
        s = '' if count == 1 else 's'
        return '{} event{}'.format(count, s)

    def _update_event_count(self):
        event_history = self._ui_model.get_event_history()
        event_count = event_history.get_received_event_count()

        if event_count != self._last_event_count:
            self._last_event_count = event_count
            self._light.do_flash()
            self._text.setText(self._get_text(event_count))

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        left_margin = (style_mgr.get_scaled_size_param('medium_padding') +
                style_mgr.get_scaled_size_param('border_thin_width'))
        margin = style_mgr.get_scaled_size(
                style_mgr.get_style_param('small_padding') +
                style_mgr.get_style_param('border_thin_width'))
        self.layout().setContentsMargins(left_margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size(
            style_mgr.get_style_param('small_padding')))

        active_colour = QColor(style_mgr.get_style_param('active_indicator_colour'))
        inactive_colour = utils.lerp_colour(QColor(0, 0, 0), active_colour, 0.25)

        config = {
            'width'     : style_mgr.get_scaled_size(1),
            'height'    : style_mgr.get_scaled_size(1),
            'colour_on' : active_colour,
            'colour_off': inactive_colour,
        }

        self._light.set_config(config)

    def sizeHint(self):
        return self.layout().sizeHint()


