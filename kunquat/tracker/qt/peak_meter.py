# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *


DEFAULT_CONFIG = {
        'colours'   : {
            'bg'  : QColor(0, 0, 0),
            'low' : QColor(0x11, 0xbb, 0x11),
            'mid' : QColor(0xdd, 0xcc, 0x33),
            'high': QColor(0xee, 0x22, 0x11),
            'clip': QColor(0xff, 0x33, 0x22),
            },
        'dim_factor': 0.45,
        'thickness' : 4,
        'padding'   : 2,
        'clip_width': 20,
        'lowest'    : -60,
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
        QWidget.__init__(self)

        self._config = None
        self._colours = None
        self._dim_colours = None
        self._bkg_grad = None
        self._grad = None
        self._grad_width = 0
        self._set_config(config)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Fixed)

        self._levels_dB = [float('-inf')] * 2
        self._holds = [[float('-inf'), 0], [float('-inf'), 0]]
        self._max_levels_dB = [float('-inf')] * 2

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
        for k, v in self._colours.iteritems():
            r, g, b = v.red(), v.green(), v.blue()
            r = lerp(bg_r, r, dim_factor)
            g = lerp(bg_g, g, dim_factor)
            b = lerp(bg_b, b, dim_factor)
            self._dim_colours[k] = QColor(r, g, b)

        self._update_gradients()

    def _update_gradients(self):
        # Common gradient properties
        grad_width = self.width() - self._config['clip_width']
        mid_point = ((-6 - self._config['lowest']) /
                (self._config['highest'] - self._config['lowest']))

        # Create background gradient
        self._bkg_grad = QLinearGradient(0, 0, grad_width, 0)
        self._bkg_grad.setColorAt(0, self._dim_colours['low'])
        self._bkg_grad.setColorAt(mid_point, self._dim_colours['mid'])
        self._bkg_grad.setColorAt(1, self._dim_colours['high'])

        # Create foreground gradient
        self._grad = QLinearGradient(0, 0, grad_width, 0)
        self._grad.setColorAt(0, self._colours['low'])
        self._grad.setColorAt(mid_point, self._colours['mid'])
        self._grad.setColorAt(1, self._colours['high'])

    def set_ui_model(self, ui_model):
        self._stat_manager = ui_model.get_stat_manager()
        self._stat_manager.register_updater(self.update_levels)

    def update_levels(self):
        levels = self._stat_manager.get_audio_levels()
        cur_time = time.time()

        for ch, level in enumerate(levels):
            if level > 0:
                self._levels_dB[ch] = math.log(level, 2)
            else:
                self._levels_dB[ch] = float('-inf')

            # Update hold
            hold = self._holds[ch]
            lifetime = cur_time - hold[1]
            if hold[0] < self._levels_dB[ch] or \
                    lifetime > self._config['hold_time']:
                hold[0] = self._levels_dB[ch]
                hold[1] = cur_time

            # Update max level
            self._max_levels_dB[ch] = max(
                    self._levels_dB[ch], self._max_levels_dB[ch])

        self.update()

    def paintEvent(self, ev):
        painter = QPainter()
        painter.begin(self)

        cfg = self._config

        painter.setBackground(self._colours['bg'])
        painter.eraseRect(ev.rect())

        bar_width = self.width() - cfg['clip_width']

        # Render background bars
        left_y = 0
        right_y = left_y + cfg['thickness'] + cfg['padding']
        for y in (left_y, right_y):
            painter.fillRect(
                    0, y,
                    bar_width, cfg['thickness'],
                    self._bkg_grad)

        # Render lit regions
        for ch in (0, 1):
            y_offset = ch * (cfg['thickness'] + cfg['padding'])
            dB_range = cfg['highest'] - cfg['lowest']

            # Render current peaks
            level_dB = self._levels_dB[ch]
            filled = (level_dB - cfg['lowest']) / dB_range
            filled = min(max(0, filled), 1)
            painter.fillRect(
                    0, y_offset,
                    bar_width * filled, cfg['thickness'],
                    self._grad)

            # Render holds
            hold = self._holds[ch]
            hold_end = bar_width * (hold[0] - cfg['lowest']) / dB_range
            hold_start = max(0, hold_end - cfg['hold_width'])
            hold_end = min(hold_end, bar_width)
            hold_width = hold_end - hold_start
            if hold_width > 0:
                painter.fillRect(
                        hold_start, y_offset,
                        hold_width, cfg['thickness'],
                        self._grad)

            # Render clips
            if self._max_levels_dB[ch] > 0:
                clip_colour = self._colours['clip']
            else:
                clip_colour = self._dim_colours['clip']
            painter.fillRect(
                    bar_width, y_offset,
                    self.width() - bar_width, cfg['thickness'],
                    clip_colour)

        painter.end()

    def resizeEvent(self, ev):
        self._update_gradients()
        self.update()

    def sizeHint(self):
        return QSize(
                self._config['clip_width'] * 4,
                self._config['thickness'] * 2 + self._config['padding'])


