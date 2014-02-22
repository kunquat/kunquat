# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtGui import *


DEFAULT_CONFIG = {
        'ruler': {
            'bg_colour'       : QColor(0x11, 0x22, 0x55),
            'fg_colour'       : QColor(0xaa, 0xcc, 0xff),
            'canvas_bg_colour': QColor(0x11, 0x11, 0x11),
            'font'            : QFont(QFont().defaultFamily(), 9),
            'line_min_dist'   : 3,
            'line_len_short'  : 2,
            'line_len_long'   : 4,
            'num_min_dist'    : 48,
            },
        'header': {
            },
        'col_width'       : 128,
        'trs_per_beat'    : 4,
        'zoom_factor'     : 1.25,
        'canvas_bg_colour': QColor(0x11, 0x11, 0x11),
        'bg_colour'       : QColor(0, 0, 0),
        'border_colour'   : QColor(0x55, 0x55, 0x55),
        'font'            : QFont(QFont().defaultFamily(), 12),
        'trigger': {
            'default_colour' : QColor(0xcc, 0xdd, 0xee),
            'note_on_colour' : QColor(0xff, 0xdd, 0xbb),
            'hit_colour'     : QColor(0xdd, 0xee, 0xbb),
            'note_off_colour': QColor(0xcc, 0x99, 0x66),
            'padding'        : 3,
            },
        'edit_cursor': {
            'line_colour'  : QColor(0xff, 0x88, 0x44),
            'min_snap_dist': 64,
            },
        }


COLUMN_COUNT = 64


