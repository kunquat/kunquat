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

import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *


DEFAULT_CONFIG = {
    'bg_colour'         : QColor(0, 0, 0),
    'center_line_colour': QColor(0x66, 0x66, 0x66),
    'waveform_colour'   : QColor(0x55, 0xff, 0x55),
}


class Waveform(QWidget):

    def __init__(self, config={}):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None

        self._config = None
        self._set_config(config)

        self._waveform = None
        self._path = None
        self._pixmap = None

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)
        self.setFocusPolicy(Qt.NoFocus)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._pixmap = None
            self._waveform = None
            self.update()

    def _update_pixmap(self):
        if not self._waveform:
            self._waveform = self._get_waveform_data()

        sample_count = 4096
        assert len(self._waveform) == sample_count

        if not self._path:
            self._path = QPainterPath()
            for i, sample in enumerate(self._waveform):
                self._path.lineTo(i, sample)
            self._path.lineTo(len(self._waveform), self._waveform[0])

        self._pixmap = QPixmap(self.width(), self.height())

        # Set background colour
        painter = QPainter(self._pixmap)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        # Draw the center line
        center_y = self.height() / 2
        painter.setPen(self._config['center_line_colour'])
        painter.drawLine(0, center_y, self.width() - 1, center_y)

        # Draw the waveform
        painter.setTransform(QTransform().translate(0.5, 0.5).scale(
            self.width() / float(sample_count), -(self.height() - 1) / 2.0).translate(
                0, -1))
        painter.setPen(self._config['waveform_colour'])
        painter.setRenderHint(QPainter.Antialiasing)

        assert self._path != None
        painter.drawPath(self._path)

    def paintEvent(self, event):
        if min(self.width(), self.height()) < 4:
            return

        start = time.time()

        if not self._pixmap:
            self._update_pixmap()

        painter = QPainter(self)
        painter.drawPixmap(0, 0, self._pixmap)

        end = time.time()
        elapsed = end - start
        print('Waveform view updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, event):
        self._pixmap = None
        self.update()

    def sizeHint(self):
        return QSize(100, 50)

    # Protected callbacks

    def _get_update_signal_type(self):
        raise NotImplementedError

    def _get_waveform_data(self):
        raise NotImplementedError


