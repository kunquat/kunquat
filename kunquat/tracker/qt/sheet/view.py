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


class View(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']
        self._px_offset = 0
        self._lengths = []

        self._col_width = DEFAULT_CONFIG['col_width']
        self._first_col = 0
        self._visible_cols = 0

        self._col_rends = [ColumnGroupRenderer(i) for i in xrange(COLUMN_COUNT)]

    def set_config(self, config):
        self._config = config
        for cr in self._col_rends:
            cr.set_config(self._config)

    def set_first_column(self, first_col):
        if self._first_col != first_col:
            self._first_col = first_col
            self.update()

    def set_patterns(self, patterns):
        self._lengths = [p['length'] for p in patterns]
        self._set_pattern_heights()
        for i, cr in enumerate(self._col_rends):
            columns = [p['columns'][i] for p in patterns]
            cr.set_columns(columns)

    def _set_pattern_heights(self):
        self._heights = get_pat_heights(self._lengths, self._px_per_beat)
        self._start_heights = get_pat_start_heights(self._heights)
        for cr in self._col_rends:
            cr.set_pattern_heights(self._heights, self._start_heights)

    def set_px_offset(self, offset):
        if self._px_offset != offset:
            self._px_offset = offset
            for cr in self._col_rends:
                cr.set_px_offset(self._px_offset)
            self.update()

    def set_px_per_beat(self, px_per_beat):
        if self._px_per_beat != px_per_beat:
            self._px_per_beat = px_per_beat
            for cr in self._col_rends:
                cr.set_px_per_beat(self._px_per_beat)
            self._set_pattern_heights()
            self.update()

    def resizeEvent(self, ev):
        max_visible_cols = get_max_visible_cols(self.width(), self._col_width)
        first_col = clamp_start_col(self._first_col, max_visible_cols)
        visible_cols = get_visible_cols(first_col, max_visible_cols)

        update_rect = None

        if first_col != self._first_col:
            update_rect = QRect(0, 0, self.width(), self.height())
        elif visible_cols > self._visible_cols:
            x_offset = self._visible_cols * self._col_width
            width = self.width() - x_offset
            update_rect = QRect(x_offset, 0, width, self.height())

        self._first_col = first_col
        self._visible_cols = visible_cols

        if ev.size().height() > ev.oldSize().height():
            update_rect = QRect(0, 0, self.width(), self.height())

        if update_rect:
            self.update(update_rect)

    def paintEvent(self, ev):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['canvas_bg_colour'])

        rect = ev.rect()

        # See which columns should be drawn
        draw_col_start = rect.x() // self._col_width
        draw_col_stop = 1 + (rect.x() + rect.width() - 1) // self._col_width
        draw_col_stop = min(draw_col_stop, COLUMN_COUNT - self._first_col)

        # Draw columns
        pixmaps_created = 0
        for rel_col_index in xrange(draw_col_start, draw_col_stop):
            x_offset = rel_col_index * self._col_width
            tfm = QTransform().translate(x_offset, 0)
            painter.setTransform(tfm)
            pixmaps_created += self._col_rends[
                    self._first_col + rel_col_index].draw(painter, self.height())

        painter.setTransform(QTransform())

        # Fill horizontal trailing blank
        hor_trail_start = draw_col_stop * self._col_width
        if hor_trail_start < self.width():
            width = self.width() - hor_trail_start
            painter.eraseRect(QRect(hor_trail_start, 0, width, self.height()))

        if pixmaps_created == 0:
            pass # TODO: update was easy, predraw a likely next pixmap
        else:
            print('{} column pixmap{} created'.format(
                pixmaps_created, 's' if pixmaps_created != 1 else ''))

        end = time.time()
        elapsed = end - start
        print('View updated in {:.2f} ms'.format(elapsed * 1000))


class ColumnGroupRenderer():

    """Manages rendering of column n for each pattern.

    """

    def __init__(self, num):
        self._num = num

        self._width = DEFAULT_CONFIG['col_width']
        self._px_offset = 0
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']

    def set_config(self, config):
        self._config = config

    def set_width(self, width):
        if self._width != width:
            self._width = width

    def set_columns(self, columns):
        self._columns = columns
        for i, cache in enumerate(self._caches):
            cache.set_column(self._columns[i])

    def set_pattern_heights(self, heights, start_heights):
        self._heights = heights
        self._start_heights = start_heights
        self._create_caches()

    def _create_caches(self):
        self._caches = [ColumnCache(self._num, i)
                for i in xrange(len(self._heights))]
        for cache in self._caches:
            cache.set_config(self._config)
        self._sync_caches()

    def _sync_caches(self):
        for cache in self._caches:
            cache.set_width(self._width)
            cache.set_px_per_beat(self._px_per_beat)

    def set_px_per_beat(self, px_per_beat):
        if self._px_per_beat != px_per_beat:
            self._px_per_beat = px_per_beat
            self._sync_caches()

    def set_px_offset(self, px_offset):
        if self._px_offset != px_offset:
            self._px_offset = px_offset
            self._sync_caches()

    def draw(self, painter, height):
        # Render columns of visible patterns
        first_index = get_first_visible_pat_index(
                self._px_offset,
                self._start_heights)

        pixmaps_created = 0

        # FIXME: copypasta from Ruler.paintEvent

        for pi in xrange(first_index, len(self._heights)):
            if self._start_heights[pi] > self._px_offset + height:
                break

            # Current pattern offset and height
            rel_start_height = self._start_heights[pi] - self._px_offset
            rel_end_height = rel_start_height + self._heights[pi]
            cur_offset = max(0, -rel_start_height)

            # Draw pixmaps
            canvas_y = max(0, rel_start_height)
            cache = self._caches[pi]
            for (src_rect, pixmap) in cache.iter_pixmaps(
                    cur_offset, min(rel_end_height, height) - canvas_y):
                dest_rect = QRect(0, canvas_y, self._width, src_rect.height())
                painter.drawPixmap(dest_rect, pixmap, src_rect)
                canvas_y += src_rect.height()

            pixmaps_created += cache.get_pixmaps_created()
        else:
            # Fill trailing blank
            painter.setBackground(self._config['canvas_bg_colour'])
            painter.eraseRect(
                    QRect(
                        0, rel_end_height,
                        self._width, height - rel_end_height)
                    )

        # Testing
        """
        painter.eraseRect(0, 0, self._col_width, height)
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._col_width - 1, height - 1)
        painter.drawText(QPoint(2, 12), str(self._num))
        """

        return pixmaps_created


class ColumnCache():

    PIXMAP_HEIGHT = 256

    def __init__(self, col_num, pat_num):
        self._col_num = col_num
        self._pat_num = pat_num

        self._pixmaps = {}
        self._pixmaps_created = 0

        self._tr_cache = TRCache()

        self._width = DEFAULT_CONFIG['col_width']
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']

    def set_config(self, config):
        self._config = config
        self._pixmaps = {}
        self._tr_cache.set_config(config)

    def set_column(self, column):
        self._column = column
        self._tr_cache.set_triggers(column)

    def set_width(self, width):
        if self._width != width:
            self._width = width
            self._pixmaps = {}

    def set_px_per_beat(self, px_per_beat):
        assert px_per_beat > 0
        if self._px_per_beat != px_per_beat:
            self._px_per_beat = px_per_beat
            self._pixmaps = {}

    def iter_pixmaps(self, start_px, height_px):
        assert start_px >= 0
        assert height_px >= 0

        stop_px = start_px + height_px

        # Get pixmap indices
        start_index = start_px // ColumnCache.PIXMAP_HEIGHT
        stop_index = 1 + (start_px + height_px - 1) // ColumnCache.PIXMAP_HEIGHT

        self._pixmaps_created = 0

        for i in xrange(start_index, stop_index):
            if i not in self._pixmaps:
                self._pixmaps[i] = self._create_pixmap(i)
                self._pixmaps_created += 1

            rect = get_pixmap_rect(
                    i,
                    start_px, stop_px,
                    self._width,
                    ColumnCache.PIXMAP_HEIGHT)

            yield (rect, self._pixmaps[i])

    def get_pixmaps_created(self):
        return self._pixmaps_created

    def _create_pixmap(self, index):
        pixmap = QPixmap(self._width, ColumnCache.PIXMAP_HEIGHT)

        painter = QPainter(pixmap)

        # Testing
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, self._width, ColumnCache.PIXMAP_HEIGHT))
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._width - 1, ColumnCache.PIXMAP_HEIGHT - 1)
        pixmap_desc = '{}-{}-{}'.format(self._col_num, self._pat_num, index)
        painter.drawText(QPoint(2, 12), pixmap_desc)

        # Start and stop timestamps
        start_px = index * ColumnCache.PIXMAP_HEIGHT
        stop_px = (index + 1) * ColumnCache.PIXMAP_HEIGHT

        visible_tr_start_px = start_px - self._config['tr_height'] + 1
        start_ts = tstamp.Tstamp(0,
                visible_tr_start_px * tstamp.BEAT // self._px_per_beat)
        stop_ts = tstamp.Tstamp(0,
                stop_px * tstamp.BEAT // self._px_per_beat)

        # Trigger rows
        for ts, tpixmap in self._tr_cache.iter_pixmaps(start_ts, stop_ts):
            rems = ts.beats * tstamp.BEAT + ts.rem
            abs_y = rems * self._px_per_beat // tstamp.BEAT
            y_offset = abs_y - start_px

            painter.drawPixmap(QPoint(0, y_offset), tpixmap)

        return pixmap


class TRCache():

    def __init__(self):
        self._pixmaps = {}

    def set_config(self, config):
        self._config = config
        self._pixmaps = {}

    def set_triggers(self, triggers):
        self._rows = self._build_trigger_rows(triggers)
        self._pixmaps = {} # TODO: only remove out-of-date pixmaps

    def _build_trigger_rows(self, triggers):
        trs = {}
        for ts, evspec in triggers:
            ts = tstamp.Tstamp(ts)
            if ts not in trs:
                trs[ts] = []
            trs[ts].append(evspec)

        trlist = list(trs.items())
        trlist.sort()
        return trlist

    def iter_pixmaps(self, start_ts, stop_ts):
        pixmaps_created = 0

        for ts, evspec in self._rows:
            if ts < start_ts:
                continue
            elif ts >= stop_ts:
                break
            if ts not in self._pixmaps:
                self._pixmaps[ts] = self._create_pixmap(evspec)
                pixmaps_created += 1
            yield (ts, self._pixmaps[ts])

        if pixmaps_created > 0:
            print('{} trigger row pixmap{} created'.format(
                pixmaps_created, 's' if pixmaps_created != 1 else ''))

    def _create_pixmap(self, evspec):
        pixmap = QPixmap(64, self._config['tr_height'])

        painter = QPainter(pixmap)

        # Testing
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, pixmap.width(), pixmap.height()))
        painter.setPen(Qt.white)
        painter.drawRect(QRect(0, 0, pixmap.width() - 1, pixmap.height() - 1))
        painter.setTransform(QTransform().rotate(-45))
        for i in xrange(4):
            side = self._config['tr_height']
            painter.fillRect(QRect(i * side * 2, 0, side, (i + 1) * side * 3), Qt.white)

        return pixmap


