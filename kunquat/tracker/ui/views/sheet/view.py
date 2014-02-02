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

from __future__ import print_function
from itertools import islice, izip, izip_longest, takewhile
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.model.triggerposition import TriggerPosition
from config import *
import utils
from verticalmovestate import VerticalMoveState


class View(QWidget):

    heightChanged = pyqtSignal(name='heightChanged')
    followCursor = pyqtSignal(int, int, name='followCursor')

    def __init__(self):
        QWidget.__init__(self)

        self._ui_model = None
        self._updater = None

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setFocusPolicy(Qt.StrongFocus)

        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']
        self._px_offset = 0
        self._patterns = []

        self._col_width = DEFAULT_CONFIG['col_width']
        self._first_col = 0
        self._visible_cols = 0

        self._col_rends = [ColumnGroupRenderer(i) for i in xrange(COLUMN_COUNT)]

        self._heights = []
        self._start_heights = []

        self._vertical_move_state = VerticalMoveState()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_module' in signals:
            self._update_all_patterns()
            self.update()
        if 'signal_selection' in signals:
            self._follow_edit_cursor()

    def _update_all_patterns(self):
        all_patterns = utils.get_all_patterns(self._ui_model)
        self.set_patterns(all_patterns)

    def set_config(self, config):
        self._config = config
        for cr in self._col_rends:
            cr.set_config(self._config)

    def set_first_column(self, first_col):
        if self._first_col != first_col:
            self._first_col = first_col
            self.update()

    def set_patterns(self, patterns):
        self._patterns = patterns
        self._set_pattern_heights()
        lengths = [p.get_length() for p in patterns]
        for i, cr in enumerate(self._col_rends):
            cr.set_pattern_lengths(lengths)
            columns = [p.get_column(i) for p in patterns]
            cr.set_columns(columns)

        selection = self._ui_model.get_selection()
        if self._patterns:
            if not selection.get_location():
                location = TriggerPosition(0, 0, 0, tstamp.Tstamp(0), 0)
                selection.set_location(location)
        else:
            if selection.get_location():
                selection.set_location(None)

    def _set_pattern_heights(self):
        lengths = [p.get_length() for p in self._patterns]
        self._heights = utils.get_pat_heights(lengths, self._px_per_beat)
        self._start_heights = utils.get_pat_start_heights(self._heights)
        for cr in self._col_rends:
            cr.set_pattern_heights(self._heights, self._start_heights)

        QObject.emit(self, SIGNAL('heightChanged()'))

    def get_total_height(self):
        return sum(self._heights)

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

    def _get_col_offset(self, col_num):
        max_visible_cols = utils.get_max_visible_cols(self.width(), self._col_width)
        first_col = utils.clamp_start_col(self._first_col, max_visible_cols)
        return (col_num - first_col) * self._col_width

    def _get_row_offset(self, location):
        # Get location components
        track = location.get_track()
        system = location.get_system()
        selected_col = location.get_col_num()
        row_ts = location.get_row_ts()

        # Get pattern that contains our location
        module = self._ui_model.get_module()
        album = module.get_album()
        song = album.get_song_by_track(track)
        pat_instance = song.get_pattern_instance(system)
        cur_pattern = pat_instance.get_pattern()

        for pattern, start_height in izip(self._patterns, self._start_heights):
            if cur_pattern == pattern:
                start_px = start_height - self._px_offset
                location_from_start_px = (
                        (row_ts.beats * tstamp.BEAT + row_ts.rem) *
                        self._px_per_beat) // tstamp.BEAT
                location_px = location_from_start_px + start_px
                return location_px

        return None

    def _follow_edit_cursor(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            self.update()
            return

        is_scrolling_required = False
        new_first_col = self._first_col
        new_y_offset = self._px_offset

        # Check horizontal scrolling
        col_num = location.get_col_num()

        x_offset = self._get_col_offset(col_num)
        if x_offset < 0:
            is_scrolling_required = True
            new_first_col = col_num
        elif x_offset + self._col_width > self.width():
            is_scrolling_required = True
            new_first_col = col_num - (self.width() // self._col_width)

        # Check vertical scrolling
        y_offset = self._get_row_offset(location)
        min_snap_dist = self._config['edit_cursor']['min_snap_dist']
        min_center_dist = min(min_snap_dist, self.height() // 2)
        tr_height = self._config['tr_height']
        min_y_offset = min_center_dist - tr_height // 2
        max_y_offset = self.height() - min_center_dist - tr_height // 2
        if y_offset < min_y_offset:
            is_scrolling_required = True
            new_y_offset = new_y_offset - (min_y_offset - y_offset)
        elif y_offset >= max_y_offset:
            is_scrolling_required = True
            new_y_offset = new_y_offset + (y_offset - max_y_offset)

        if is_scrolling_required:
            QObject.emit(
                    self,
                    SIGNAL('followCursor(int, int)'),
                    new_y_offset,
                    new_first_col)
        else:
            self.update()

    def _draw_edit_cursor(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if location:
            assert self._patterns

            track = location.get_track()
            system = location.get_system()
            selected_col = location.get_col_num()
            row_ts = location.get_row_ts()

            # Get pixel offsets
            x_offset = self._get_col_offset(selected_col)
            if not 0 <= x_offset < self.width():
                return
            y_offset = self._get_row_offset(location)
            if not -self._config['tr_height'] < y_offset < self.height():
                return

            # Set up paint device
            painter = QPainter(self)
            tfm = QTransform().translate(x_offset, y_offset)
            painter.setTransform(tfm)

            # Draw the horizontal line
            painter.setPen(self._config['edit_cursor']['line_colour'])
            painter.drawLine(
                    QPoint(0, 0),
                    QPoint(self._col_width - 2, 0))

            # Get trigger row at cursor
            module = self._ui_model.get_module()
            album = module.get_album()
            song = album.get_song_by_track(track)
            pattern = song.get_pattern_instance(system).get_pattern()
            column = pattern.get_column(selected_col)

            try:
                # Draw the trigger row
                trigger_count = column.get_trigger_count_at_row(row_ts)
                tr = [column.get_trigger(row_ts, i) for i in xrange(trigger_count)]
            except KeyError:
                # No triggers, just draw a hollow rectangle
                metrics = self._config['font_metrics']
                bounding_rect = metrics.tightBoundingRect(u'þ')
                bounding_rect.translate(0, -bounding_rect.top())
                painter.setPen(self._config['trigger']['default_colour'])
                painter.drawRect(bounding_rect)

    def _move_edit_cursor(self):
        px_delta = self._vertical_move_state.get_delta()
        if px_delta == 0:
            return

        module = self._ui_model.get_module()
        album = module.get_album()
        if not album or album.get_track_count() == 0:
            return

        # Get location info
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        track = location.get_track()
        system = location.get_system()
        col_num = location.get_col_num()
        row_ts = location.get_row_ts()
        trigger_index = location.get_trigger_index()

        cur_song = album.get_song_by_track(track)
        cur_pattern = cur_song.get_pattern_instance(system).get_pattern()
        cur_column = cur_pattern.get_column(col_num)

        # Check moving to the previous system
        if px_delta < 0 and row_ts == 0:
            if track > 0 or system > 0:
                new_track = track
                new_system = system - 1
                new_song = album.get_song_by_track(new_track)
                if new_system < 0:
                    new_track -= 1
                    new_song = album.get_song_by_track(new_track)
                    new_system = new_song.get_system_count() - 1
                new_pattern = new_song.get_pattern_instance(new_system).get_pattern()
                pat_height = utils.get_pat_height(
                        new_pattern.get_length(), self._px_per_beat)
                new_ts = utils.get_tstamp_from_px(
                        pat_height + px_delta, self._px_per_beat)
                new_ts = max(tstamp.Tstamp(0), new_ts)
                assert new_ts <= new_pattern.get_length()

                new_location = TriggerPosition(
                        new_track, new_system, col_num, new_ts, trigger_index)
                selection.set_location(new_location)

            return

        # Get default trigger tstamp on the current pixel position
        cur_px_offset = utils.get_px_from_tstamp(row_ts, self._px_per_beat)
        def_ts = utils.get_tstamp_from_px(cur_px_offset, self._px_per_beat)
        assert utils.get_px_from_tstamp(def_ts, self._px_per_beat) == cur_px_offset

        # Convert pixel delta to tstamp delta
        ts_delta = utils.get_tstamp_from_px(px_delta, self._px_per_beat)

        # Get target tstamp
        new_ts = def_ts + ts_delta

        # Get shortest movement between target tstamp and closest trigger row
        move_range_start = min(new_ts, row_ts)
        move_range_stop = max(new_ts, row_ts) + tstamp.Tstamp(0, 1)
        if px_delta < 0:
            move_range_stop -= tstamp.Tstamp(0, 1)
        else:
            move_range_start += tstamp.Tstamp(0, 1)

        trow_tstamps = cur_column.get_trigger_row_positions_in_range(
                move_range_start, move_range_stop)
        if trow_tstamps:
            self._vertical_move_state.try_snap_delay()
            if px_delta < 0:
                new_ts = max(trow_tstamps)
            else:
                new_ts = min(trow_tstamps)

        # Check moving outside pattern boundaries
        if new_ts < 0:
            self._vertical_move_state.try_snap_delay()
            new_ts = tstamp.Tstamp(0)
        elif new_ts > cur_pattern.get_length():
            new_track = track
            new_system = system + 1
            if new_system >= cur_song.get_system_count():
                new_track += 1
                new_system = 0
                if new_track >= album.get_track_count():
                    # End of sheet
                    new_ts = cur_pattern.get_length()
                    new_location = TriggerPosition(
                            track, system, col_num, new_ts, trigger_index)
                    selection.set_location(new_location)
                    return

            # Next pattern
            self._vertical_move_state.try_snap_delay()
            new_location = TriggerPosition(
                    new_track, new_system, col_num, tstamp.Tstamp(0), trigger_index)
            selection.set_location(new_location)
            return

        # Move inside pattern
        new_location = TriggerPosition(track, system, col_num, new_ts, trigger_index)
        selection.set_location(new_location)

    def keyPressEvent(self, ev):
        if ev.key() == Qt.Key_Up:
            self._vertical_move_state.press_up()
            self._move_edit_cursor()
        elif ev.key() == Qt.Key_Down:
            self._vertical_move_state.press_down()
            self._move_edit_cursor()

    def keyReleaseEvent(self, ev):
        if ev.isAutoRepeat():
            return
        if ev.key() == Qt.Key_Up:
            self._vertical_move_state.release_up()
        elif ev.key() == Qt.Key_Down:
            self._vertical_move_state.release_down()

    def resizeEvent(self, ev):
        max_visible_cols = utils.get_max_visible_cols(self.width(), self._col_width)
        first_col = utils.clamp_start_col(self._first_col, max_visible_cols)
        visible_cols = utils.get_visible_cols(first_col, max_visible_cols)

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

        # Draw edit cursor
        if self.hasFocus():
            self._draw_edit_cursor()

        if pixmaps_created == 0:
            pass # TODO: update was easy, predraw a likely next pixmap
        else:
            print('{} column pixmap{} created'.format(
                pixmaps_created, 's' if pixmaps_created != 1 else ''))

        end = time.time()
        elapsed = end - start
        print('View updated in {:.2f} ms'.format(elapsed * 1000))

    def focusInEvent(self, ev):
        self.update()

    def focusOutEvent(self, ev):
        self.update()


class ColumnGroupRenderer():

    """Manages rendering of column n for each pattern.

    """

    def __init__(self, num):
        self._num = num

        self._width = DEFAULT_CONFIG['col_width']
        self._px_offset = 0
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']

        self._heights = []
        self._start_heights = []

    def set_config(self, config):
        self._config = config

    def set_width(self, width):
        if self._width != width:
            self._width = width

    def set_columns(self, columns):
        self._columns = columns
        for i, cache in enumerate(self._caches):
            cache.set_column(self._columns[i])

    def set_pattern_lengths(self, lengths):
        self._lengths = lengths

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
        first_index = utils.get_first_visible_pat_index(
                self._px_offset,
                self._start_heights)

        pixmaps_created = 0

        # FIXME: contains some copypasta from Ruler.paintEvent

        overlap = None
        max_tr_width = self._width - 1

        rel_end_height = 0 # empty song

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

            # Draw overlapping part of previous pattern
            if overlap:
                src_rect, image = overlap

                # Prevent from drawing over the first trigger row
                first_tr = cache.get_first_trigger_row()
                if first_tr:
                    first_ts, _ = first_tr
                    first_rems = first_ts.beats * tstamp.BEAT + first_ts.rem
                    first_start_y = first_rems * self._px_per_beat // tstamp.BEAT
                    first_start_y += rel_start_height
                    src_rect_stop_y = rel_start_height + src_rect.height()
                    if src_rect_stop_y > first_start_y:
                        tr_overlap = src_rect_stop_y - first_start_y
                        src_rect.setHeight(src_rect.height() - tr_overlap)

                width = min(max_tr_width, src_rect.width())
                dest_rect = QRect(
                        0, rel_start_height,
                        width, src_rect.height())
                src_rect.setWidth(width)
                painter.drawImage(dest_rect, image, src_rect)
                overlap = None

            # Find trigger row that overlaps with next pattern
            last_tr = cache.get_last_trigger_row(self._lengths[pi])
            if last_tr:
                last_ts, last_image = last_tr
                last_rems = last_ts.beats * tstamp.BEAT + last_ts.rem
                last_start_y = last_rems * self._px_per_beat // tstamp.BEAT
                last_stop_y = last_start_y + self._config['tr_height']
                if last_stop_y >= self._heights[pi]:
                    # + 1 is the shared pixel row between patterns
                    rect_height = last_stop_y - self._heights[pi] + 1
                    rect_start = self._config['tr_height'] - rect_height
                    rect = QRect(
                            0, rect_start,
                            last_image.rect().width(), rect_height)
                    overlap = rect, last_image
        else:
            # Fill trailing blank
            painter.setBackground(self._config['canvas_bg_colour'])
            painter.eraseRect(
                    QRect(
                        0, rel_end_height,
                        self._width, height - rel_end_height)
                    )

            # Draw trigger row that extends beyond the last pattern
            if overlap:
                src_rect, image = overlap
                # Last pattern and blank do not share pixel rows
                src_rect.setY(src_rect.y() + 1)
                width = min(max_tr_width, src_rect.width())
                dest_rect = QRect(
                        0, rel_end_height,
                        width, src_rect.height())
                src_rect.setWidth(width)
                painter.drawImage(dest_rect, image, src_rect)

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

            rect = utils.get_pixmap_rect(
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

        # Background
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(QRect(0, 0, self._width - 1, ColumnCache.PIXMAP_HEIGHT))

        # Start and stop timestamps
        start_px = index * ColumnCache.PIXMAP_HEIGHT
        stop_px = (index + 1) * ColumnCache.PIXMAP_HEIGHT

        visible_tr_start_px = start_px - self._config['tr_height'] + 1
        start_ts = tstamp.Tstamp(0,
                visible_tr_start_px * tstamp.BEAT // self._px_per_beat)
        stop_ts = tstamp.Tstamp(0,
                stop_px * tstamp.BEAT // self._px_per_beat)

        def ts_to_y_offset(ts):
            rems = ts.beats * tstamp.BEAT + ts.rem
            abs_y = rems * self._px_per_beat // tstamp.BEAT
            y_offset = abs_y - start_px
            return y_offset

        # Trigger rows
        painter.setCompositionMode(QPainter.CompositionMode_SourceOver)
        for ts, image, next_ts in self._tr_cache.iter_images(start_ts, stop_ts):
            y_offset = ts_to_y_offset(ts)

            src_rect = image.rect()
            dest_rect = src_rect.translated(QPoint(0, y_offset))

            if next_ts != None:
                next_y_offset = ts_to_y_offset(next_ts)
                y_dist = next_y_offset - y_offset
                if y_dist < dest_rect.height():
                    rect_height = max(1, y_dist)
                    dest_rect.setHeight(rect_height)
                    src_rect.setHeight(rect_height)

            painter.drawImage(dest_rect, image, src_rect)

        # Border
        painter.setPen(self._config['border_colour'])
        painter.drawLine(
                QPoint(self._width - 1, 0),
                QPoint(self._width - 1, ColumnCache.PIXMAP_HEIGHT))

        # Testing
        """
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, self._width, ColumnCache.PIXMAP_HEIGHT))
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._width - 1, ColumnCache.PIXMAP_HEIGHT - 1)
        pixmap_desc = '{}-{}-{}'.format(self._col_num, self._pat_num, index)
        painter.drawText(QPoint(2, 12), pixmap_desc)
        """

        return pixmap

    def get_first_trigger_row(self):
        return self._tr_cache.get_first_trigger_row()

    def get_last_trigger_row(self, max_ts):
        return self._tr_cache.get_last_trigger_row(max_ts)


class TRCache():

    def __init__(self):
        self._images = {}

    def set_config(self, config):
        self._config = config
        self._images = {}

    def set_triggers(self, column):
        self._rows = self._build_trigger_rows(column)
        self._images = {} # TODO: only remove out-of-date images

    def _build_trigger_rows(self, column):
        trs = {}
        for ts in column.get_trigger_row_positions():
            trow = [column.get_trigger(ts, i)
                    for i in xrange(column.get_trigger_count_at_row(ts))]
            trs[ts] = trow

        trlist = list(trs.items())
        trlist.sort()
        return trlist

    def iter_images(self, start_ts, stop_ts):
        images_created = 0

        next_tstamps = (row[0] for row in islice(self._rows, 1, None))

        for row, next_ts in izip_longest(self._rows, next_tstamps):
            ts, triggers = row
            if ts < start_ts:
                continue
            elif ts >= stop_ts:
                break
            if ts not in self._images:
                self._images[ts] = self._create_image(triggers)
                images_created += 1
            yield (ts, self._images[ts], next_ts)

        if images_created > 0:
            print('{} trigger row image{} created'.format(
                images_created, 's' if images_created != 1 else ''))

    def _create_image(self, triggers):
        pdev = QPixmap(1, 1)
        rends = [TriggerRenderer(self._config, t) for t in triggers]
        widths = [r.get_trigger_width(pdev) for r in rends]

        image = QImage(
                sum(widths),
                self._config['tr_height'],
                QImage.Format_ARGB32)
        image.fill(0)

        painter = QPainter(image)
        for renderer, width in izip(rends, widths):
            renderer.draw_trigger(painter)
            painter.setTransform(QTransform().translate(width, 0), True)

        # Testing
        """
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, image.width(), image.height()))
        painter.setPen(Qt.red)
        painter.drawRect(QRect(0, 0, image.width() - 1, image.height() - 1))
        painter.setTransform(QTransform().rotate(-45))
        for i in xrange(4):
            side = self._config['tr_height']
            painter.fillRect(QRect(i * side * 2, 0, side, (i + 1) * side * 3), Qt.red)
        """

        return image

    def get_first_trigger_row(self):
        if self._rows:
            ts, _ = self._rows[0]
            if ts not in self._images:
                return None
            return ts, self._images[ts]
        return None

    def get_last_trigger_row(self, max_ts):
        for row in reversed(self._rows):
            ts, _ = row
            if ts <= max_ts:
                if ts not in self._images:
                    # We haven't rendered the trigger row yet,
                    # so it must be outside the view
                    return None
                return ts, self._images[ts]
        return None


class TriggerRenderer():

    def __init__(self, config, trigger):
        assert trigger
        self._config = config
        self._trigger = trigger

    def get_trigger_width(self, pdev): # TODO: note names
        evtype = self._trigger.get_type()
        expr = self._trigger.get_argument()

        # Padding
        total_padding = self._config['trigger']['padding'] * 2
        if expr != None:
            # Space between type and expression
            total_padding += self._config['trigger']['padding']

        # Text
        metrics = self._config['font_metrics']
        self._baseline_offset = metrics.tightBoundingRect('A').height()
        evtype_width = metrics.boundingRect(evtype).width()
        expr_width = 0
        if expr != None:
            expr_width = metrics.boundingRect(expr).width()

        # Drawing parameters
        self._evtype_offset = self._config['trigger']['padding']
        self._expr_offset = (self._evtype_offset + evtype_width +
                self._config['trigger']['padding'])
        self._width = total_padding + evtype_width + expr_width

        return self._width

    def draw_trigger(self, painter):
        painter.setPen(Qt.white)
        painter.drawLine(QPoint(0, 0), QPoint(self._width - 2, 0))

        painter.setCompositionMode(QPainter.CompositionMode_Plus)

        evtype = self._trigger.get_type()
        expr = self._trigger.get_argument()
        painter.drawText(QPoint(self._evtype_offset, self._baseline_offset), evtype)
        if expr != None:
            painter.drawText(QPoint(self._expr_offset, self._baseline_offset), expr)


