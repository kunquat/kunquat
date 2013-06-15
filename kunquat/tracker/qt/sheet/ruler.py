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
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *
from utils import *
import tstamp


class Ruler(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._lengths = []
        self._px_offset = 0
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']
        self._cache = RulerCache()

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._config = config

        num_space = QFontMetrics(self._config['font']).boundingRect('00.000')
        self._width = (num_space.width() +
                self._config['line_len_long'] +
                8)

        self._cache.set_config(config)
        self._cache.set_width(self._width)
        self._cache.set_num_height(num_space.height())

        self.update()

    def set_px_per_beat(self, px_per_beat):
        changed = self._px_per_beat != px_per_beat
        self._px_per_beat = px_per_beat
        self._cache.set_px_per_beat(px_per_beat)
        if changed:
            self._set_pattern_heights()
            self.update()

    def set_pattern_lengths(self, lengths):
        self._lengths = lengths
        self._set_pattern_heights()

    def _set_pattern_heights(self):
        self._heights = get_pat_heights(self._lengths, self._px_per_beat)
        self._start_heights = get_pat_start_heights(self._heights)

    def set_px_offset(self, offset):
        changed = offset != self._px_offset
        self._px_offset = offset
        if changed:
            self._set_pattern_heights()
            self.update()

    def paintEvent(self, ev):
        start = time.time()

        painter = QPainter(self)

        # Render rulers of visible patterns
        first_index = get_first_visible_pat_index(
                self._px_offset,
                self._start_heights)

        for pi in xrange(first_index, len(self._heights)):
            if self._start_heights[pi] > self._px_offset + self.height():
                break

            # Current pattern offset and height
            rel_start_height = self._start_heights[pi] - self._px_offset
            rel_end_height = rel_start_height + self._heights[pi]
            cur_offset = max(0, -rel_start_height)

            # Draw pixmaps
            canvas_y = max(0, rel_start_height)
            for (src_rect, pixmap) in self._cache.iter_pixmaps(
                    cur_offset, min(rel_end_height, self.height()) - canvas_y):
                dest_rect = QRect(0, canvas_y, self.width(), src_rect.height())
                painter.drawPixmap(dest_rect, pixmap, src_rect)
                canvas_y += src_rect.height()

            # Draw bottom border
            if rel_end_height <= self.height():
                painter.setPen(self._config['fg_colour'])
                painter.drawLine(
                        QPoint(0, rel_end_height - 1),
                        QPoint(self._width - 1, rel_end_height - 1))
        else:
            # Fill trailing blank
            painter.setBackground(self._config['canvas_bg_colour'])
            painter.eraseRect(
                    QRect(
                        0, rel_end_height,
                        self._width, self.height() - rel_end_height)
                    )

        end = time.time()
        elapsed = end - start
        #print('Ruler updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, ev):
        self._cache.set_width(ev.size().width())

    def sizeHint(self):
        return QSize(self._width, 128)


class RulerCache():

    PIXMAP_HEIGHT = 128

    def __init__(self):
        self._width = 0
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']
        self._pixmaps = {}

    def set_config(self, config):
        self._config = config

    def set_width(self, width):
        if width != self._width:
            self._pixmaps = {}
        self._width = width

    def set_num_height(self, height):
        self._num_height = height

    def set_px_per_beat(self, px_per_beat):
        if px_per_beat != self._px_per_beat:
            self._pixmaps = {}
        self._px_per_beat = px_per_beat

    def iter_pixmaps(self, start_px, height_px):
        assert start_px >= 0
        assert height_px >= 0

        stop_px = start_px + height_px

        create_count = 0

        for i in get_pixmap_indices(start_px, stop_px, RulerCache.PIXMAP_HEIGHT):
            if i not in self._pixmaps:
                self._pixmaps[i] = self._create_pixmap(i)
                create_count += 1

            rect = get_pixmap_rect(
                    i,
                    start_px, stop_px,
                    self._width,
                    RulerCache.PIXMAP_HEIGHT)

            yield (rect, self._pixmaps[i])

        if create_count > 0:
            """
            print('{} ruler pixmap{} created'.format(
                create_count, 's' if create_count != 1 else ''))
            """

    def _create_pixmap(self, index):
        pixmap = QPixmap(self._width, RulerCache.PIXMAP_HEIGHT)

        cfg = self._config

        painter = QPainter(pixmap)

        # Background
        painter.setBackground(cfg['bg_colour'])
        painter.eraseRect(QRect(0, 0, self._width - 1, RulerCache.PIXMAP_HEIGHT))
        painter.setPen(cfg['fg_colour'])
        painter.drawLine(
                QPoint(self._width - 1, 0),
                QPoint(self._width - 1, RulerCache.PIXMAP_HEIGHT - 1))

        # Start border
        if index == 0:
            painter.drawLine(QPoint(0, 0), QPoint(self._width - 2, 0))

        # Ruler lines
        start_ts = tstamp.Tstamp(0, tstamp.BEAT *
                index * RulerCache.PIXMAP_HEIGHT // self._px_per_beat)
        stop_ts = tstamp.Tstamp(0, tstamp.BEAT *
                (index + 1) * RulerCache.PIXMAP_HEIGHT // self._px_per_beat)

        def draw_ruler_line(painter, y, line_pos, lines_per_beat):
            line_length = (cfg['line_len_long']
                    if line_pos[1] == 0
                    else cfg['line_len_short'])
            painter.drawLine(
                    QPoint(self._width - 1 - line_length, y),
                    QPoint(self._width - 1, y))

        self._draw_markers(
                painter,
                start_ts,
                stop_ts,
                cfg['line_min_dist'],
                draw_ruler_line)

        # Beat numbers
        num_extent = self._num_height // 2
        start_ts = tstamp.Tstamp(0, tstamp.BEAT *
                (index * RulerCache.PIXMAP_HEIGHT - num_extent) //
                self._px_per_beat)
        stop_ts = tstamp.Tstamp(0, tstamp.BEAT *
                ((index + 1) * RulerCache.PIXMAP_HEIGHT + num_extent) //
                self._px_per_beat)

        text_option = QTextOption(Qt.AlignRight | Qt.AlignVCenter)
        def draw_number(painter, y, num_pos, nums_per_beat):
            if num_pos == [0, 0]:
                return

            # Text
            num = num_pos[0] + num_pos[1] / float(nums_per_beat)
            numi = int(num)
            text = str(numi) if num == numi else str(round(num, 3))

            # Draw
            rect = QRectF(0, y - self._num_height,
                    self._width - cfg['line_len_long'] - 2, self._num_height)
            painter.drawText(rect, text, text_option)

        painter.setFont(self._config['font'])
        self._draw_markers(
                painter,
                start_ts,
                stop_ts,
                cfg['num_min_dist'],
                draw_number)

        # Draw pixmap index for debugging
        #painter.drawText(QPoint(2, 12), str(index))

        return pixmap

    def _draw_markers(self, painter, start_ts, stop_ts, min_dist, draw_fn):
        cfg = self._config

        beat_div_base = 2

        if min_dist <= self._px_per_beat:
            markers_per_beat = self._px_per_beat // min_dist
            markers_per_beat = int(beat_div_base**math.floor(
                    math.log(markers_per_beat, beat_div_base)))

            # First visible marker in the first beat
            start_beat_frac = start_ts.rem / float(tstamp.BEAT)
            start_marker_in_beat = int(
                    math.ceil(start_beat_frac * markers_per_beat))

            # First non-visible marker in the last beat
            stop_beat_frac = stop_ts.rem / float(tstamp.BEAT)
            stop_marker_in_beat = int(
                    math.ceil(stop_beat_frac * markers_per_beat))

            def normalise_marker_pos(pos):
                excess = pos[1] // markers_per_beat
                pos[0] += excess
                pos[1] -= excess * markers_per_beat
                assert pos[1] >= 0
                assert pos[1] < markers_per_beat

            # Loop boundaries
            marker_pos = [start_ts.beats, start_marker_in_beat]
            normalise_marker_pos(marker_pos)

            stop_pos = [stop_ts.beats, stop_marker_in_beat]
            normalise_marker_pos(stop_pos)

            # Draw markers
            while marker_pos < stop_pos:
                ts = tstamp.Tstamp(marker_pos[0] +
                        marker_pos[1] / float(markers_per_beat))
                y = float(ts - start_ts) * self._px_per_beat

                draw_fn(painter, y, marker_pos, markers_per_beat)

                # Next marker
                marker_pos[1] += 1
                normalise_marker_pos(marker_pos)

        else:
            # Zoomed far out -- skipping whole beats
            beats_per_marker = (min_dist +
                    self._px_per_beat - 1) // self._px_per_beat
            beats_per_marker = int(beat_div_base**math.ceil(
                    math.log(beats_per_marker, beat_div_base)))

            # First beat with a visible marker
            start_beat = (start_ts - tstamp.Tstamp(0, 1)).beats + 1
            start_marker_in_beat = beats_per_marker * int(
                    math.ceil(start_beat / float(beats_per_marker)))

            # First non-visible beat
            stop_beat = (stop_ts - tstamp.Tstamp(0, 1)).beats + 1

            # Draw markers
            for marker_pos in xrange(
                    start_marker_in_beat, stop_beat, beats_per_marker):
                y = float(marker_pos - start_ts) * self._px_per_beat
                draw_fn(painter, y, [marker_pos, 0], 1)


