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

from PyQt4.QtGui import *


DEFAULT_CONFIG = {
        'ruler': {
            'bg_colour'       : QColor(0x11, 0x22, 0x55),
            'fg_colour'       : QColor(0xaa, 0xcc, 0xff),
            'canvas_bg_colour': QColor(0, 0, 0),
            'font'            : QFont(QFont().defaultFamily(), 9),
            'line_min_dist'   : 3,
            'line_len_short'  : 2,
            'line_len_long'   : 4,
            'num_min_dist'    : 48,
            },
        'header': {
            },
        'col_width'       : 128,
        'px_per_beat'     : 128,
        'canvas_bg_colour': QColor(0, 0, 0),
        'bg_colour'       : QColor(0, 0, 0),
        'font'            : QFont(QFont().defaultFamily(), 11),
        }


COLUMN_COUNT = 64


