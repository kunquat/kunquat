# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import islice, zip_longest

from PySide.QtCore import *
from PySide.QtGui import *

import kunquat.tracker.ui.model.tstamp as tstamp
from .config import *
from . import utils
from .buffercache import BufferCache
from .triggerrenderer import TriggerRenderer


class ColumnGroupRenderer():

    """Manages rendering of column n for each pattern.

    """

    def __init__(self, num):
        self._num = num

        self._ui_model = None

        self._caches = None

        self._width = DEFAULT_CONFIG['col_width']
        self._px_offset = 0
        self._px_per_beat = None

        self._heights = []
        self._start_heights = []

    def set_config(self, config):
        self._config = config
        if self._caches:
            for cache in self._caches:
                cache.set_config(self._config)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        if self._caches:
            for cache in self._caches:
                cache.set_ui_model(ui_model)

    def set_width(self, width):
        if self._width != width:
            self._width = width
            self._sync_caches()

    def set_columns(self, columns):
        self._columns = columns
        for i, cache in enumerate(self._caches):
            cache.set_column(self._columns[i])

    def update_column(self, pattern_index, col_data):
        self._columns[pattern_index] = col_data
        if self._caches:
            cache = self._caches[pattern_index]
            cache.flush()
            cache.set_column(self._columns[pattern_index])

    def flush_caches(self):
        if self._caches:
            for cache in self._caches:
                cache.flush()

    def flush_final_pixmaps(self):
        if self._caches:
            for cache in self._caches:
                cache.flush_final_pixmaps()

    def set_pattern_lengths(self, lengths):
        self._lengths = lengths

    def set_pattern_heights(self, heights, start_heights):
        self._heights = heights
        self._start_heights = start_heights

        # FIXME: revisit cache creation
        if self._caches and (len(self._caches) == len(self._heights)):
            self._sync_caches()
        else:
            self._create_caches()

    def rearrange_patterns(self, new_pinsts):
        cur_caches = {}
        for cache in (self._caches or []):
            cur_caches[cache.get_pinst_ref()] = cache

        new_caches = []
        new_columns = []
        for pinst in new_pinsts:
            column = pinst.get_column(self._num)
            new_columns.append(column)

            pinst_ref = (pinst.get_pattern_num(), pinst.get_instance_num())
            if pinst_ref in cur_caches:
                cache = cur_caches[pinst_ref]
            else:
                cache = ColumnCachePair(self._num, pinst_ref)
                cache.set_config(self._config)
                cache.set_ui_model(self._ui_model)
                cache.set_column(column)
            new_caches.append(cache)
        self._caches = new_caches
        self._columns = new_columns

        self._sync_caches()

    def _create_caches(self):
        pinsts = utils.get_all_pattern_instances(self._ui_model)
        self._caches = [
                ColumnCachePair(self._num, (p.get_pattern_num(), p.get_instance_num()))
                for p in pinsts]
        for cache in self._caches:
            cache.set_config(self._config)
            cache.set_ui_model(self._ui_model)

        self._sync_caches()

    def _sync_caches(self):
        if self._caches:
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

    def get_memory_usage(self):
        if not self._caches:
            return 0
        return sum(cache.get_memory_usage() for cache in self._caches)

    def _get_active_pattern_index(self):
        playback_manager = self._ui_model.get_playback_manager()
        active_pattern_index = None
        if (playback_manager.follow_playback_cursor() and
                not playback_manager.is_recording()):
            active_pattern_index = utils.get_current_playback_pattern_index(
                    self._ui_model)
        else:
            selection = self._ui_model.get_selection()
            location = selection.get_location()
            if location:
                active_pattern_index = utils.get_pattern_index_at_location(
                        self._ui_model, location.get_track(), location.get_system())

        return active_pattern_index

    def draw(self, painter, height, grid):
        # Render columns of visible patterns
        first_index = utils.get_first_visible_pat_index(
                self._px_offset, self._start_heights)

        pixmaps_created = 0

        # FIXME: contains some copypasta from Ruler.paintEvent

        overlap = None
        max_tr_width = self._width - 1

        rel_end_height = 0 # empty song

        active_pattern_index = self._get_active_pattern_index()

        for pi in range(first_index, len(self._heights)):
            if self._start_heights[pi] > self._px_offset + height:
                break

            # Current pattern offset and height
            rel_start_height = self._start_heights[pi] - self._px_offset
            rel_end_height = rel_start_height + self._heights[pi]
            cur_offset = max(0, -rel_start_height)

            # Choose cache based on whether this pattern contains the edit cursor
            if pi == active_pattern_index:
                cache = self._caches[pi].get_active_cache()
            else:
                cache = self._caches[pi].get_inactive_cache()

            # Draw pixmaps
            canvas_y = max(0, rel_start_height)
            for (src_rect, pixmap) in cache.iter_pixmaps(
                    cur_offset, min(rel_end_height, height) - canvas_y, grid):
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
            if rel_end_height < height:
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

    def predraw(self, height, grid):
        # Find a missing pixmap that isn't drawn yet but is likely needed soon
        first_index = utils.get_first_visible_pat_index(
                self._px_offset, self._start_heights)

        active_pattern_index = self._get_active_pattern_index()

        for pi in range(first_index, len(self._heights)):
            if self._start_heights[pi] > self._px_offset + height:
                break

            # Current pattern offset and height
            rel_start_height = self._start_heights[pi] - self._px_offset
            rel_end_height = rel_start_height + self._heights[pi]
            cur_offset = max(0, -rel_start_height)

            # Determine which cache is used for visible pixmaps and which isn't
            vis_cache = self._caches[pi].get_active_cache()
            invis_cache = self._caches[pi].get_inactive_cache()
            if pi != active_pattern_index:
                vis_cache, invis_cache = invis_cache, vis_cache

            # Try predrawing a pixmap immediately below the last one
            below_check_dist = ColumnCache.PIXMAP_HEIGHT // 2
            if height <= rel_end_height < (height + below_check_dist):
                next_pi = pi + 1
                if next_pi < len(self._heights):
                    if next_pi == active_pattern_index:
                        below_vis_cache = self._caches[next_pi].get_active_cache()
                    else:
                        below_vis_cache = self._caches[next_pi].get_inactive_cache()
                    if below_vis_cache.predraw_pixmap(0, 1, grid):
                        return True

            if vis_cache.predraw_pixmap(
                    cur_offset + height, below_check_dist, grid):
                return True

            # Try predrawing a pixmap that is needed after active pattern change
            canvas_y = max(0, rel_start_height)
            if invis_cache.predraw_pixmap(
                    cur_offset, min(rel_end_height, height) - canvas_y, grid):
                return True

        return False

    def update_hit_names(self, height):
        # Flush parts of the cache that contain hits
        first_index = utils.get_first_visible_pat_index(
                self._px_offset, self._start_heights)

        active_pattern_index = self._get_active_pattern_index()

        for pi in range(0, first_index):
            self._caches[pi].flush()

        stop_index = first_index

        for pi in range(first_index, len(self._heights)):
            if self._start_heights[pi] > self._px_offset + height:
                stop_index = pi + 1
                break

            vis_cache = self._caches[pi].get_active_cache()
            invis_cache = self._caches[pi].get_inactive_cache()
            if pi != active_pattern_index:
                vis_cache, invis_cache = invis_cache, vis_cache

            vis_cache.update_hit_names()
            invis_cache.flush()

        for pi in range(stop_index, len(self._heights)):
            self._caches[pi].flush()


class ColumnCachePair():

    def __init__(self, col_num, pinst_ref):
        self._pinst_ref = pinst_ref
        self._active = ColumnCache(col_num, pinst_ref, inactive=False)
        self._inactive = ColumnCache(col_num, pinst_ref, inactive=True)

    def get_pinst_ref(self):
        return self._pinst_ref

    def set_config(self, config):
        self._active.set_config(config)
        self._inactive.set_config(config)

    def set_ui_model(self, ui_model):
        self._active.set_ui_model(ui_model)
        self._inactive.set_ui_model(ui_model)

    def set_column(self, column):
        self._active.set_column(column)
        self._inactive.set_column(column)

    def set_width(self, width):
        self._active.set_width(width)
        self._inactive.set_width(width)

    def set_px_per_beat(self, px_per_beat):
        self._active.set_px_per_beat(px_per_beat)
        self._inactive.set_px_per_beat(px_per_beat)

    def flush(self):
        self._active.flush()
        self._inactive.flush()

    def flush_final_pixmaps(self):
        self._active.flush_final_pixmaps()
        self._inactive.flush_final_pixmaps()

    def get_memory_usage(self):
        return self._active.get_memory_usage() + self._inactive.get_memory_usage()

    def get_active_cache(self):
        return self._active

    def get_inactive_cache(self):
        return self._inactive


class ColumnCache():

    PIXMAP_HEIGHT = 256

    def __init__(self, col_num, pinst_ref, *, inactive):
        self._col_num = col_num
        self._pinst_ref = pinst_ref
        self._inactive = inactive
        self._ui_model = None

        self._pixmaps = BufferCache()
        self._pixmaps_created = 0

        self._tr_cache = TRCache()
        self._gl_cache = GridLineCache()

        if self._inactive:
            self._tr_cache.set_inactive()
            self._gl_cache.set_inactive()

        self._width = DEFAULT_CONFIG['col_width']
        self._px_per_beat = None

    def set_config(self, config):
        self._config = config
        self._pixmaps.flush()
        self._tr_cache.set_config(config)
        self._gl_cache.set_config(config)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._tr_cache.set_ui_model(ui_model)

    def set_column(self, column):
        self._column = column
        self._tr_cache.set_triggers(column)

    def flush(self):
        self._pixmaps.flush()
        self._tr_cache.flush()
        self._gl_cache.flush()

    def flush_final_pixmaps(self):
        self._pixmaps.flush()

    def set_width(self, width):
        self._gl_cache.set_width(width)

        if self._width != width:
            self._width = width
            self._pixmaps.flush()

            # Keep roughly 4 screens of pixmaps cached
            memory_limit = 4 * (4 * width * 1600)
            self._pixmaps.set_memory_limit(memory_limit)

    def set_px_per_beat(self, px_per_beat):
        assert px_per_beat > 0
        if self._px_per_beat != px_per_beat:
            self._px_per_beat = px_per_beat
            self._pixmaps.flush()

    def get_memory_usage(self):
        tr_memory_usage = self._tr_cache.get_memory_usage()
        return self._pixmaps.get_memory_usage() + tr_memory_usage

    def update_hit_names(self):
        if self._tr_cache.contains_hits():
            self._pixmaps.flush()
            self._tr_cache.flush()

    def iter_pixmaps(self, start_px, height_px, grid):
        assert start_px >= 0
        assert height_px >= 0

        stop_px = start_px + height_px

        # Get pixmap indices
        start_index = start_px // ColumnCache.PIXMAP_HEIGHT
        stop_index = 1 + (start_px + height_px - 1) // ColumnCache.PIXMAP_HEIGHT

        self._pixmaps_created = 0

        for i in range(start_index, stop_index):
            if i not in self._pixmaps:
                pixmap = self._create_pixmap(i, grid)
                self._pixmaps[i] = pixmap
                self._pixmaps_created += 1
            else:
                pixmap = self._pixmaps[i]

            rect = utils.get_pixmap_rect(
                    i,
                    start_px, stop_px,
                    self._width,
                    ColumnCache.PIXMAP_HEIGHT)

            yield (rect, pixmap)

    def predraw_pixmap(self, start_px, height_px, grid):
        assert start_px >= 0
        assert height_px >= 0

        # Get pixmap indices
        start_index = start_px // ColumnCache.PIXMAP_HEIGHT
        stop_index = 1 + (start_px + height_px - 1) // ColumnCache.PIXMAP_HEIGHT

        for i in range(start_index, stop_index):
            if i not in self._pixmaps:
                self._pixmaps[i] = self._create_pixmap(i, grid)
                return True

        return False

    def get_pixmaps_created(self):
        return self._pixmaps_created

    def _get_final_colour(self, colour):
        if self._inactive:
            return utils.scale_colour(colour, self._config['inactive_dim'])
        return colour

    def _create_pixmap(self, index, grid):
        pixmap = QPixmap(self._width, ColumnCache.PIXMAP_HEIGHT)

        painter = QPainter(pixmap)

        album = self._ui_model.get_module().get_album()
        track, system = album.get_pattern_instance_location_by_nums(*self._pinst_ref)
        pat_index = utils.get_pattern_index_at_location(self._ui_model, track, system)
        assert pat_index != None

        # Background
        painter.setBackground(self._get_final_colour(self._config['bg_colour']))
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

        # Grid
        sheet_manager = self._ui_model.get_sheet_manager()
        if sheet_manager.is_grid_enabled():
            pinsts = utils.get_all_pattern_instances(self._ui_model)
            pinst = pinsts[pat_index]

            grid_start_ts = tstamp.Tstamp(0, start_px * tstamp.BEAT // self._px_per_beat)
            tr_height_ts = utils.get_tstamp_from_px(
                    self._config['tr_height'], self._px_per_beat)
            lines = grid.get_grid_lines(
                    pinst, self._col_num, grid_start_ts, stop_ts, tr_height_ts)

            for line_info in lines:
                line_ts, line_style = line_info
                line_y_offset = ts_to_y_offset(line_ts)

                line_pixmap = self._gl_cache.get_line_pixmap(line_style)
                painter.drawPixmap(QPoint(0, line_y_offset), line_pixmap)

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
        painter.setPen(self._get_final_colour(self._config['border_colour']))
        painter.drawLine(
                QPoint(self._width - 1, 0),
                QPoint(self._width - 1, ColumnCache.PIXMAP_HEIGHT))

        # Testing
        """
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, self._width, ColumnCache.PIXMAP_HEIGHT))
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._width - 1, ColumnCache.PIXMAP_HEIGHT - 1)
        pixmap_desc = '{}-{}-{}'.format(self._col_num, pat_index, index)
        painter.drawText(QPoint(2, 12), pixmap_desc)
        """

        return pixmap

    def get_first_trigger_row(self):
        return self._tr_cache.get_first_trigger_row()

    def get_last_trigger_row(self, max_ts):
        return self._tr_cache.get_last_trigger_row(max_ts)


class TRCache():

    def __init__(self):
        self._images = BufferCache()
        self._ui_model = None
        self._notation_manager = None
        self._inactive = False
        self._rows = []

    def set_inactive(self):
        self._inactive = True

    def set_config(self, config):
        self._config = config
        self._images.flush()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._notation_manager = ui_model.get_notation_manager()

    def set_triggers(self, column):
        self._rows = self._build_trigger_rows(column)
        self._images.flush() # TODO: only remove out-of-date images

    def contains_hits(self):
        for row in self._rows:
            ts, triggers = row
            for trigger in triggers:
                if trigger.get_type() == 'h':
                    return True
        return False

    def flush(self):
        self._images.flush()

    def _build_trigger_rows(self, column):
        trs = {}
        for ts in column.get_trigger_row_positions():
            trow = [column.get_trigger(ts, i)
                    for i in range(column.get_trigger_count_at_row(ts))]
            trs[ts] = trow

        trlist = list(trs.items())
        trlist.sort()
        return trlist

    def iter_images(self, start_ts, stop_ts):
        images_created = 0

        next_tstamps = (row[0] for row in islice(self._rows, 1, None))

        for row, next_ts in zip_longest(self._rows, next_tstamps):
            ts, triggers = row
            if ts < start_ts:
                continue
            elif ts >= stop_ts:
                break

            if ts not in self._images:
                image = self._create_image(triggers)
                self._images[ts] = image
                images_created += 1
            else:
                image = self._images[ts]

            yield (ts, image, next_ts)

        #if images_created > 0:
        #    print('{} trigger row image{} created'.format(
        #        images_created, 's' if images_created != 1 else ''))

    def get_memory_usage(self):
        return self._images.get_memory_usage()

    def _create_image(self, triggers):
        module = self._ui_model.get_module()
        force_shift = module.get_force_shift()

        notation = self._notation_manager.get_selected_notation()
        rends = [TriggerRenderer(self._config, t, notation, force_shift)
                for t in triggers]
        widths = [r.get_total_width() for r in rends]

        if self._inactive:
            for r in rends:
                r.set_inactive()

        image = QImage(
                sum(widths),
                self._config['tr_height'],
                QImage.Format_ARGB32)
        image.fill(0)

        painter = QPainter(image)
        painter.setCompositionMode(QPainter.CompositionMode_Plus)
        for renderer, width in zip(rends, widths):
            renderer.draw_trigger(painter)
            painter.setTransform(QTransform().translate(width, 0), True)

        # Testing
        """
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, image.width(), image.height()))
        painter.setPen(Qt.red)
        painter.drawRect(QRect(0, 0, image.width() - 1, image.height() - 1))
        painter.setTransform(QTransform().rotate(-45))
        for i in range(4):
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


class GridLineCache():

    def __init__(self):
        self._config = None
        self._inactive = False

        self._pixmaps = BufferCache()

        self._width = DEFAULT_CONFIG['col_width']

    def set_inactive(self):
        self._inactive = True
        self._pixmaps.flush()

    def set_config(self, config):
        self._config = config
        self._pixmaps.flush()

    def flush(self):
        self._pixmaps.flush()

    def set_width(self, width):
        if self._width != width:
            self._width = width
            self._pixmaps.flush()

    def get_line_pixmap(self, style):
        if style not in self._pixmaps:
            pixmap = self._create_line_pixmap(style)
            self._pixmaps[style] = pixmap
        return self._pixmaps[style]

    def _get_final_colour(self, colour):
        if self._inactive:
            return utils.scale_colour(colour, self._config['inactive_dim'])
        return colour

    def _create_line_pixmap(self, style):
        pixmap = QPixmap(self._width, 1)

        painter = QPainter(pixmap)

        # Background
        painter.setBackground(self._get_final_colour(self._config['bg_colour']))
        painter.eraseRect(QRect(0, 0, self._width - 1, 1))

        # Line
        pen = QPen(self._config['grid']['styles'][style])
        pen.setColor(self._get_final_colour(pen.color()))
        painter.setPen(pen)
        painter.drawLine(QPoint(0, 0), QPoint(self._width - 1, 0))

        return pixmap


