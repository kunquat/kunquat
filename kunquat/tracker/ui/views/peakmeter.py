# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2016
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


DEFAULT_CONFIG = {
        'colours'   : {
            'bg'  : QColor(0, 0, 0),
            'low' : QColor(0x11, 0x99, 0x11),
            'mid' : QColor(0xdd, 0xcc, 0x33),
            'high': QColor(0xee, 0x22, 0x11),
            'clip': QColor(0xff, 0x33, 0x22),
            },
        'dim_factor': 0.45,
        'thickness' : 4,
        'padding'   : 2,
        'clip_width': 20,
        'lowest'    : -96,
        'highest'   : 0,
        'hold_time' : 1,
        'hold_width': 6,
        }


def lerp(from_val, to_val, lerp_val):
    assert lerp_val >= 0
    assert lerp_val <= 1
    diff = to_val - from_val
    return from_val + (lerp_val * diff)


class PeakMeter(QWidget):

    def __init__(self, config={}):
        super().__init__()
        self._updater = None

        self._config = None
        self._colours = None
        self._dim_colours = None
        self._bg_bar = None
        self._fg_bar = None
        self._set_config(config)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Fixed)

        self._levels_dB = [float('-inf')] * 2
        self._holds = [[float('-inf'), 0], [float('-inf'), 0]]
        self._max_levels_dB = [float('-inf')] * 2

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        self._colours = DEFAULT_CONFIG['colours'].copy()
        if 'colours' in config:
            self._colours.update(config['colours'])

        # Update dim colours
        self._dim_colours = {}
        bg_colour = self._colours['bg']
        bg_r, bg_g, bg_b = bg_colour.red(), bg_colour.green(), bg_colour.blue()
        dim_factor = self._config['dim_factor']
        for k, v in self._colours.items():
            r, g, b = v.red(), v.green(), v.blue()
            r = lerp(bg_r, r, dim_factor)
            g = lerp(bg_g, g, dim_factor)
            b = lerp(bg_b, b, dim_factor)
            self._dim_colours[k] = QColor(r, g, b)

        self._update_buffers()

    def _update_buffers(self):
        # Common gradient properties
        grad_width = self.width() - self._config['clip_width']
        mid_point = ((-6 - self._config['lowest']) /
                (self._config['highest'] - self._config['lowest']))

        # Create background gradient
        bg_grad = QLinearGradient(0, 0, grad_width, 0)
        bg_grad.setColorAt(0, self._dim_colours['low'])
        bg_grad.setColorAt(mid_point, self._dim_colours['mid'])
        bg_grad.setColorAt(1, self._dim_colours['high'])

        # Create foreground gradient
        fg_grad = QLinearGradient(0, 0, grad_width, 0)
        fg_grad.setColorAt(0, self._colours['low'])
        fg_grad.setColorAt(mid_point, self._colours['mid'])
        fg_grad.setColorAt(1, self._colours['high'])

        # Create buffer for background bar
        self._bg_bar = QPixmap(grad_width, self._config['thickness'])
        painter = QPainter(self._bg_bar)
        painter.fillRect(
                0, 0,
                grad_width, self._config['thickness'],
                bg_grad)

        # Create buffer for foreground bar
        self._fg_bar = QPixmap(grad_width, self._config['thickness'])
        painter = QPainter(self._fg_bar)
        painter.fillRect(
                0, 0,
                grad_width, self._config['thickness'],
                fg_grad)

    def set_ui_model(self, ui_model):
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._stat_manager = ui_model.get_stat_manager()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        self._update_levels()

    def _update_levels(self):
        levels = self._stat_manager.get_audio_levels()
        cur_time = time.time()

        for ch, level in enumerate(levels):
            if level > 0:
                self._levels_dB[ch] = math.log(level, 2) * 6
            else:
                self._levels_dB[ch] = float('-inf')

            # Update hold
            hold = self._holds[ch]
            lifetime = cur_time - hold[1]
            if hold[0] < self._levels_dB[ch] or \
                    lifetime > self._config['hold_time']:
                hold[0] = self._levels_dB[ch]
                hold[1] = cur_time

        max_levels = self._stat_manager.get_max_audio_levels()
        self._max_levels_dB = [
                (math.log(s, 2) * 6 if s > 0 else float('-inf')) for s in max_levels]

        self.update()

    def paintEvent(self, ev):
        painter = QPainter(self)

        cfg = self._config

        bar_width = self.width() - cfg['clip_width']
        thickness = cfg['thickness']

        painter.setBackground(self._colours['bg'])
        painter.eraseRect(QRect(0, thickness, self.width(), cfg['padding']))

        # Render bars
        for ch in (0, 1):
            y_offset = ch * (thickness + cfg['padding'])
            dB_range = cfg['highest'] - cfg['lowest']

            # Get peak width
            level_dB = self._levels_dB[ch]
            filled_norm = (level_dB - cfg['lowest']) / dB_range
            filled_norm = min(max(0, filled_norm), 1)
            filled = int(bar_width * filled_norm)

            # Fill current peak
            painter.drawPixmap(
                    QRect(0, y_offset, filled, thickness),
                    self._fg_bar,
                    QRect(0, 0, filled, thickness))

            # Fill background
            painter.drawPixmap(
                    QRect(filled, y_offset, bar_width - filled, thickness),
                    self._bg_bar,
                    QRect(filled, 0, bar_width - filled, thickness))

            # Render holds
            hold = self._holds[ch]
            hold_end_norm = (hold[0] - cfg['lowest']) / dB_range
            hold_end_norm = max(-1, hold_end_norm)
            hold_end = int(bar_width * hold_end_norm)
            hold_start = max(0, hold_end - cfg['hold_width'])
            hold_end = min(hold_end, bar_width)
            hold_width = hold_end - hold_start
            if hold_width > 0:
                painter.drawPixmap(
                        QRect(hold_start, y_offset, hold_width, thickness),
                        self._fg_bar,
                        QRect(hold_start, 0, hold_width, thickness))

            # Render clips
            if self._max_levels_dB[ch] > 0:
                clip_colour = self._colours['clip']
            else:
                clip_colour = self._dim_colours['clip']
            painter.fillRect(
                    bar_width, y_offset,
                    self.width() - bar_width, cfg['thickness'],
                    clip_colour)

    def resizeEvent(self, ev):
        self._update_buffers()
        self.update()

    def sizeHint(self):
        return QSize(
                self._config['clip_width'] * 4,
                self._config['thickness'] * 2 + self._config['padding'])


