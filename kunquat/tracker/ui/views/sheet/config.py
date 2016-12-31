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

from PySide.QtCore import *
from PySide.QtGui import *


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
            'inactive_dim'    : 0.6,
            },
        'header': {
            'bg_colour'    : QColor(0x22, 0x44, 0x22),
            'fg_colour'    : QColor(0xcc, 0xee, 0xaa),
            'border_colour': QColor(0x55, 0x77, 0x55),
            'font'         : QFont(QFont().defaultFamily(), 11, QFont.Bold),
            },
        'col_width'       : 12, # unit is em
        'trs_per_beat'    : 4,
        'zoom_factor'     : 1.25,
        'inactive_dim'    : 0.6,
        'canvas_bg_colour': QColor(0x11, 0x11, 0x11),
        'bg_colour'       : QColor(0, 0, 0),
        'border_colour'   : QColor(0x55, 0x55, 0x55),
        'font'            : QFont(QFont().defaultFamily(), 12),
        'disabled_colour' : QColor(0x88, 0x88, 0x88, 0x7f),
        'trigger': {
            'default_colour'   : QColor(0xcc, 0xdd, 0xee),
            'note_on_colour'   : QColor(0xff, 0xdd, 0xbb),
            'hit_colour'       : QColor(0xbb, 0xee, 0x88),
            'note_off_colour'  : QColor(0xcc, 0x99, 0x66),
            'warning_bg_colour': QColor(0xee, 0x33, 0x11),
            'warning_fg_colour': QColor(0xff, 0xff, 0xcc),
            'padding'          : 3,
            },
        'edit_cursor': {
            'view_line_colour': QColor(0xdd, 0xee, 0xff),
            'edit_line_colour': QColor(0xff, 0x88, 0x44),
            'min_snap_dist'   : 64,
            'guide_colour'    : QColor(0xff, 0x88, 0x44, 0x7f),
            },
        'area_selection': {
            'border_colour': QColor(0x88, 0xaa, 0xcc),
            'fill_colour'  : QColor(0x88, 0xaa, 0xcc, 0x7f),
            },
        'grid': {
            'styles': {
                0: QPen(QBrush(QColor(0xa0, 0xa0, 0xa0)), 1, Qt.SolidLine),
                1: QPen(QBrush(QColor(0xa0, 0xa0, 0xa0)), 1, Qt.DashLine),
                2: QPen(QBrush(QColor(0xa0, 0xa0, 0xa0)), 1, Qt.DotLine),
                3: QPen(QBrush(QColor(0x60, 0x60, 0x60)), 1, Qt.SolidLine),
                4: QPen(QBrush(QColor(0x60, 0x60, 0x60)), 1, Qt.DashLine),
                5: QPen(QBrush(QColor(0x60, 0x60, 0x60)), 1, Qt.DotLine),
                6: QPen(QBrush(QColor(0x40, 0x40, 0x40)), 1, Qt.SolidLine),
                7: QPen(QBrush(QColor(0x40, 0x40, 0x40)), 1, Qt.DashLine),
                8: QPen(QBrush(QColor(0x40, 0x40, 0x40)), 1, Qt.DotLine),
                },
            'edit_cursor': {
                'height': 11,
                'width' : 13,
                'colour': QColor(0xee, 0x77, 0x33),
                },
            },
        }


