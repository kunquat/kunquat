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

from PyQt4.QtCore import *
from PyQt4.QtGui import *


_LIGHT_CONFIG = {
        'width'     : 12,
        'height'    : 12,
        'delta'     : 0.02,
        'fade_time' : 0.25,
        'colour_on' : QColor(0xff, 0x33, 0),
        'colour_off': QColor(0x44, 0, 0),
        }

def lerp_val(v1, v2, t):
    assert 0 <= t <= 1
    return v1 + (v2 - v1) * t

def lerp_colour(c1, c2, t):
    assert 0 <= t <= 1
    return QColor(
            lerp_val(c1.red(), c2.red(), t),
            lerp_val(c1.green(), c2.green(), t),
            lerp_val(c1.blue(), c2.blue(), t))


class EventLight(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._config = _LIGHT_CONFIG.copy()

        self._light_norm = 0

        self._refresh_timer = QTimer()
        QObject.connect(self._refresh_timer, SIGNAL('timeout()'), self._update_vis)
        self._refresh_timer.start(self._config['delta'] * 1000)

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
        colour = lerp_colour(
                self._config['colour_off'],
                self._config['colour_on'],
                self._light_norm)
        painter.fillRect(0, 0, self._config['width'], self._config['height'], colour)

    def sizeHint(self):
        return QSize(self._config['width'], self._config['height'])


class EventListButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None
        self._updater = None
        self._last_event = None

        self._light = EventLight()
        light_layout = QWidgetItem(self._light)
        light_layout.setAlignment(Qt.AlignVCenter)
        self._text = QLabel('Event Log')

        h = QHBoxLayout()
        h.setContentsMargins(5, 5, 5, 5)
        h.addItem(light_layout)
        h.addWidget(self._text)
        self.setLayout(h)

        self.setAutoRaise(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_event_log()

    def _perform_updates(self, signals):
        event_history = self._ui_model.get_event_history()
        log = event_history.get_log()

        if log:
            if log[0] != self._last_event:
                self._last_event = log[0]
                self._light.do_flash()
        else:
            if self._last_event:
                self._last_event = None
                self._light.do_flash()

    def sizeHint(self):
        return self.layout().sizeHint()


