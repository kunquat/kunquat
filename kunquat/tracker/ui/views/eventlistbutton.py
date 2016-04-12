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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from . import utils


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
        colour = utils.lerp_colour(
                self._config['colour_off'],
                self._config['colour_on'],
                self._light_norm)
        painter.fillRect(0, 0, self._config['width'], self._config['height'], colour)

    def sizeHint(self):
        return QSize(self._config['width'], self._config['height'])


class EventListButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._last_event_count = 0

        self._light = EventLight()
        light_layout = QWidgetItem(self._light)
        light_layout.setAlignment(Qt.AlignVCenter)
        self._text = QLabel(self._get_text(0))

        h = QHBoxLayout()
        h.setMargin(6)
        h.addItem(light_layout)
        h.addWidget(self._text)
        self.setLayout(h)

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

    def _get_text(self, count):
        s = '' if count == 1 else 's'
        return '{} event{}'.format(count, s)

    def _perform_updates(self, signals):
        event_history = self._ui_model.get_event_history()
        event_count = event_history.get_received_event_count()

        if event_count != self._last_event_count:
            self._last_event_count = event_count
            self._light.do_flash()
            self._text.setText(self._get_text(event_count))

    def sizeHint(self):
        return self.layout().sizeHint()


