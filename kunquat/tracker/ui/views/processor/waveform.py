# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2020
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from kunquat.tracker.ui.qt import *
from .pathemboldener import embolden_path


DEFAULT_CONFIG = {
    'bg_colour'         : QColor(0, 0, 0),
    'centre_line_colour': QColor(0x66, 0x66, 0x66),
    'waveform_colour'   : QColor(0x55, 0xff, 0x55),
    'disabled_colour'   : QColor(0x88, 0x88, 0x88, 0x7f),
    'line_thickness'    : 1,
}


class Waveform(QWidget):

    def __init__(self):
        super().__init__()

        self._config = None
        self.set_config({})

        self._waveform = None
        self._path = None
        self._path_image = None
        self._pixmap = None

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)
        self.setFocusPolicy(Qt.NoFocus)

    def set_waveform(self, waveform):
        assert waveform != None
        self._waveform = waveform
        self._path = None
        self._pixmap = None
        self.update()

    def set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)
        self._path_image = None
        self._pixmap = None
        self.update()

    def _update_pixmap(self):
        sample_count = 4096
        assert self._waveform != None
        assert len(self._waveform) == sample_count

        width, height = self.width(), self.height()

        line_thickness = self._config['line_thickness']

        if not self._path:
            self._path = QPainterPath()
            self._path.moveTo(-16, self._waveform[-16])
            for i in range(-15, 0):
                self._path.lineTo(i, self._waveform[i])
            for i, sample in enumerate(self._waveform):
                self._path.lineTo(i, sample)
            self._path.lineTo(len(self._waveform), self._waveform[0])

            self._path_image = None

        if not self._path_image:
            thin_image = QImage(width, height, QImage.Format_ARGB32_Premultiplied)
            thin_image.fill(0)

            shift = 0.5 * ((line_thickness / 2) + (line_thickness % 2))
            squish = 0.5 + line_thickness

            thin_painter = QPainter(thin_image)
            thin_painter.setTransform(QTransform().translate(shift, shift + 0.5).scale(
                width / float(sample_count), -(height - squish) / 2).translate(0, -1))
            pen = QPen(QColor(self._config['waveform_colour']))
            pen.setCosmetic(True)
            thin_painter.setPen(pen)
            thin_painter.setRenderHint(QPainter.Antialiasing)

            thin_painter.drawPath(self._path)

            thin_painter.end()

            #thin_image.setPixelColor(QPoint(50, 50), QColor(0xff, 0xff, 0xff))

            self._path_image = embolden_path(thin_image, line_thickness)

        self._pixmap = QPixmap(width, height)

        # Set background colour
        painter = QPainter(self._pixmap)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, width, height)

        # Draw the centre line
        centre_y = self.height() / 2
        pen = QPen(self._config['centre_line_colour'])
        pen.setCosmetic(True)
        pen.setWidth(line_thickness)
        painter.setPen(pen)
        painter.drawLine(0, int(centre_y), int(width - 1), int(centre_y))

        # Draw the waveform image
        assert self._path_image != None
        painter.drawImage(QPoint(0, 0), self._path_image)

    def paintEvent(self, event):
        if min(self.width(), self.height()) < 4:
            return

        start = time.time()

        if not self._pixmap:
            self._update_pixmap()

        painter = QPainter(self)
        painter.drawPixmap(0, 0, self._pixmap)

        # Grey out disabled widget
        if not self.isEnabled():
            painter.fillRect(
                    0, 0, self.width(), self.height(), self._config['disabled_colour'])

        end = time.time()
        elapsed = end - start
        #print('Waveform view updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, event):
        self._pixmap = None
        self._path_image = None
        self.update()

    def sizeHint(self):
        return QSize(100, 50)


