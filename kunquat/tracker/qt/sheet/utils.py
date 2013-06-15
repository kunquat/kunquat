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

from __future__ import print_function
import math

from PyQt4.QtCore import *

from config import *
import tstamp


# Column utils

def get_max_visible_cols(full_width, col_width):
    return min(full_width // col_width + 1, COLUMN_COUNT + 1)

def clamp_start_col(first_col, max_visible_cols):
    return min(first_col, COLUMN_COUNT - max_visible_cols + 1)

def get_visible_cols(first_col, max_visible_cols):
    if first_col + max_visible_cols > COLUMN_COUNT:
        return max_visible_cols - 1
    return max_visible_cols


# Pattern utils

def pat_height(length, px_per_beat):
    return int(math.ceil(float(length + tstamp.Tstamp(0, 1)) * px_per_beat))

def get_pat_heights(lengths, px_per_beat):
    return [pat_height(pl, px_per_beat) for pl in lengths]

def get_pat_start_heights(heights):
    start_heights = [0]
    for h in heights:
        start_heights.append(start_heights[-1] + h - 1)
    return start_heights

def get_first_visible_pat_index(px_offset, start_heights):
    index = 0
    for h in start_heights:
        if h == px_offset:
            return index
        elif h > px_offset:
            return index - 1
        index += 1
    return index


# Pixmap buffer utils

def get_pixmap_indices(start_px, stop_px, pixmap_height):
    start_index = start_px // pixmap_height
    stop_index = 1 + (stop_px - 1) // pixmap_height
    return xrange(start_index, stop_index)

def get_pixmap_rect(index, start_px, stop_px, width, pixmap_height):
    pixmap_start_px = index * pixmap_height
    rect_start_abs = max(start_px, pixmap_start_px)
    rect_start = rect_start_abs - pixmap_start_px

    pixmap_stop_px = (index + 1) * pixmap_height
    rect_stop_abs = min(stop_px, pixmap_stop_px)
    rect_stop = rect_stop_abs - pixmap_start_px
    rect_height = rect_stop - rect_start

    return QRect(0, rect_start, width, min(rect_height, stop_px - start_px))


