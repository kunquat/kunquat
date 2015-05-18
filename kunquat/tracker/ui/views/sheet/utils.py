# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2015
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
from kunquat.kunquat.limits import *
import kunquat.tracker.ui.model.tstamp as tstamp


# Model utils

def get_all_pattern_instances_with_locations(ui_model):
    module = ui_model.get_module()

    album = module.get_album()
    if not album:
        return

    track_count = album.get_track_count()
    songs = (album.get_song_by_track(i) for i in xrange(track_count))
    for track, song in enumerate(songs):
        system_count = song.get_system_count()
        pattern_instances = (song.get_pattern_instance(i)
                for i in xrange(system_count))
        for system, pinst in enumerate(pattern_instances):
            yield (track, system), pinst

    return

def get_all_pattern_instances(ui_model):
    return [info[1] for info in get_all_pattern_instances_with_locations(ui_model)]

def get_all_patterns(ui_model):
    return [pinst.get_pattern() for pinst in get_all_pattern_instances(ui_model)]

def get_pattern_index_at_location(ui_model, track, system):
    for index, info in enumerate(get_all_pattern_instances_with_locations(ui_model)):
        loc, _ = info
        cur_track, cur_system = loc
        if track == cur_track and system == cur_system:
            return index
    return None


# Column view utils

def get_max_visible_cols(full_width, col_width):
    return min(full_width // col_width + 1, COLUMNS_MAX + 1)

def clamp_start_col(first_col, max_visible_cols):
    return min(first_col, COLUMNS_MAX - max_visible_cols + 1)

def get_visible_cols(first_col, max_visible_cols):
    if first_col + max_visible_cols > COLUMNS_MAX:
        return max_visible_cols - 1
    return max_visible_cols


# Pattern view utils

def get_pat_height(length, px_per_beat):
    return int(math.ceil(float(length + tstamp.Tstamp(0, 1)) * px_per_beat))

def get_pat_heights(lengths, px_per_beat):
    return [get_pat_height(pl, px_per_beat) for pl in lengths]

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


# Pixel <-> Tstamp conversions

def get_tstamp_from_px(px, px_per_beat):
    beats = px // px_per_beat
    rem_px = px % px_per_beat
    rem = rem_px * tstamp.BEAT // px_per_beat
    if rem * px_per_beat < rem_px * tstamp.BEAT:
        rem += 1
    return tstamp.Tstamp(beats, rem)

def get_px_from_tstamp(ts, px_per_beat):
    beats_add = ts.beats * px_per_beat
    rem_add = ts.rem * px_per_beat
    return beats_add + (rem_add // tstamp.BEAT)


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


