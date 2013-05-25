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
        self._set_config(config)

        #self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Fixed)

        self._levels_dB = [float('-inf')] * 2
        self._holds = [[float('-inf'), 0], [float('-inf'), 0]]

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

    def set_ui_model(self, ui_model):
        self._stat_manager = ui_model.get_stat_manager()
        self._stat_manager.register_updater(self.update_levels)

    def update_levels(self):
        levels = self._stat_manager.get_audio_levels()
        for i, level in enumerate(levels):
            self._levels_dB[i] = math.log(level, 2) if level > 0 else float('-inf')
        self.update()

    def paintEvent(self, ev):
        paint = QPainter()
        paint.begin(self)

        colours = self._config['colours']

        paint.setBackground(colours['bg'])
        paint.eraseRect(ev.rect())

        paint.end()


