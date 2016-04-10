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

from __future__ import print_function
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.tracker.ui.model.tstamp as tstamp
from .config import *
from . import utils


class Ruler(QWidget):

    heightChanged = pyqtSignal(name='heightChanged')

    def __init__(self, is_grid_ruler=False):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._is_grid_ruler = is_grid_ruler

        self._lengths = []
        self._px_offset = 0
        self._px_per_beat = None
        self._cache = RulerCache()
        self._inactive_cache = RulerCache()
        self._inactive_cache.set_inactive()

        self._heights = []
        self._start_heights = []

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._config = config

        fm = QFontMetrics(self._config['font'], self)
        self._config['font_metrics'] = fm
        num_space = fm.tightBoundingRect('00.000')
        self._width = (num_space.width() +
                self._config['line_len_long'] +
                8)

        self._cache.set_config(config)
        self._cache.set_width(self._width)
        self._cache.set_num_height(num_space.height())

        self._inactive_cache.set_config(config)
        self._inactive_cache.set_width(self._width)
        self._inactive_cache.set_num_height(num_space.height())

        self.update()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def get_total_height(self):
        return sum(self._heights)

    def _perform_updates(self, signals):
        if not self._is_grid_ruler:
            update_signals = set([
                'signal_module',
                'signal_order_list',
                'signal_pattern_length',
                'signal_undo',
                'signal_redo'])
            if not signals.isdisjoint(update_signals):
                self._update_all_patterns()
                self.update()

        if 'signal_selection' in signals:
            self.update()

    def update_grid_pattern(self):
        assert self._is_grid_ruler

        self._lengths = []

        grid_manager = self._ui_model.get_grid_manager()
        gp_id = grid_manager.get_selected_grid_pattern_id()
        if gp_id != None:
            gp = grid_manager.get_grid_pattern(gp_id)
            gp_length = gp.get_length()
            self._lengths = [gp_length]

        self._set_pattern_heights()
        self.update()

    def _update_all_patterns(self):
        all_patterns = utils.get_all_patterns(self._ui_model)
        self.set_patterns(all_patterns)

    def set_px_per_beat(self, px_per_beat):
        changed = self._px_per_beat != px_per_beat
        self._px_per_beat = px_per_beat
        self._cache.set_px_per_beat(px_per_beat)
        self._inactive_cache.set_px_per_beat(px_per_beat)
        if changed:
            self._set_pattern_heights()
            self.update()

    def set_patterns(self, patterns):
        self._lengths = [p.get_length() for p in patterns]
        self._set_pattern_heights()

    def _set_pattern_heights(self):
        self._heights = utils.get_pat_heights(self._lengths, self._px_per_beat)
        self._start_heights = utils.get_pat_start_heights(self._heights)
        QObject.emit(self, SIGNAL('heightChanged()'))

    def set_px_offset(self, offset):
        changed = offset != self._px_offset
        self._px_offset = offset
        if changed:
            self._set_pattern_heights()
            self.update()

    def _get_final_colour(self, colour, inactive):
        if inactive:
            dim_factor = self._config['inactive_dim']
            new_colour = QColor(colour)
            new_colour.setRed(colour.red() * dim_factor)
            new_colour.setGreen(colour.green() * dim_factor)
            new_colour.setBlue(colour.blue() * dim_factor)
            return new_colour
        return colour

    def paintEvent(self, ev):
        start = time.time()

        painter = QPainter(self)

        # Render rulers of visible patterns
        first_index = utils.get_first_visible_pat_index(
                self._px_offset,
                self._start_heights)

        rel_end_height = 0 # empty song

        # Get pattern index that contains the edit cursor
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if location:
            active_pattern_index = utils.get_pattern_index_at_location(
                    self._ui_model, location.get_track(), location.get_system())
        else:
            active_pattern_index = None

        for pi in xrange(first_index, len(self._heights)):
            if self._start_heights[pi] > self._px_offset + self.height():
                break

            # Current pattern offset and height
            rel_start_height = self._start_heights[pi] - self._px_offset
            rel_end_height = rel_start_height + self._heights[pi]
            cur_offset = max(0, -rel_start_height)

            # Choose cache based on whether this pattern contains the edit cursor
            cache = self._cache if (pi == active_pattern_index) else self._inactive_cache

            # Draw pixmaps
            canvas_y = max(0, rel_start_height)
            for (src_rect, pixmap) in cache.iter_pixmaps(
                    cur_offset, min(rel_end_height, self.height()) - canvas_y):
                dest_rect = QRect(0, canvas_y, self.width(), src_rect.height())
                if pi - 1 == active_pattern_index:
                    # Make sure the active bottom border remains visible
                    src_rect.setTop(src_rect.top() + 1)
                    dest_rect.setTop(dest_rect.top() + 1)
                painter.drawPixmap(dest_rect, pixmap, src_rect)
                canvas_y += src_rect.height()

            # Draw bottom border
            if rel_end_height <= self.height():
                painter.setPen(self._get_final_colour(
                    self._config['fg_colour'], pi != active_pattern_index))
                painter.drawLine(
                        QPoint(0, rel_end_height - 1),
                        QPoint(self._width - 1, rel_end_height - 1))
        else:
            # Fill trailing blank
            painter.setBackground(self._config['canvas_bg_colour'])
            if rel_end_height < self.height():
                painter.eraseRect(
                        QRect(
                            0, rel_end_height,
                            self._width, self.height() - rel_end_height)
                        )

        end = time.time()
        elapsed = end - start
        #print('Ruler updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, event):
        self._cache.set_width(event.size().width())
        self._inactive_cache.set_width(event.size().width())

    def sizeHint(self):
        return QSize(self._width, 128)


class RulerCache():

    PIXMAP_HEIGHT = 128

    def __init__(self):
        self._width = 0
        self._px_per_beat = None
        self._pixmaps = {}
        self._inactive = False

    def set_inactive(self):
        self._inactive = True

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

        for i in utils.get_pixmap_indices(start_px, stop_px, RulerCache.PIXMAP_HEIGHT):
            if i not in self._pixmaps:
                self._pixmaps[i] = self._create_pixmap(i)
                create_count += 1

            rect = utils.get_pixmap_rect(
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

    def _get_final_colour(self, colour):
        if self._inactive:
            dim_factor = self._config['inactive_dim']
            new_colour = QColor(colour)
            new_colour.setRed(colour.red() * dim_factor)
            new_colour.setGreen(colour.green() * dim_factor)
            new_colour.setBlue(colour.blue() * dim_factor)
            return new_colour
        return colour

    def _create_pixmap(self, index):
        pixmap = QPixmap(self._width, RulerCache.PIXMAP_HEIGHT)

        cfg = self._config

        painter = QPainter(pixmap)

        # Background
        painter.setBackground(self._get_final_colour(cfg['bg_colour']))
        painter.eraseRect(QRect(0, 0, self._width - 1, RulerCache.PIXMAP_HEIGHT))
        painter.setPen(self._get_final_colour(cfg['fg_colour']))
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
                    self._width - cfg['line_len_long'] - 2, self._num_height + 3)
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


