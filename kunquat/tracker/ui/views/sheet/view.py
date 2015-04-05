# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2015
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
from itertools import islice, izip
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.kunquat.events as events
import kunquat.tracker.cmdline as cmdline
import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.model.trigger import Trigger
from kunquat.tracker.ui.model.triggerposition import TriggerPosition
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper
from config import *
import utils
from columngrouprenderer import ColumnGroupRenderer
from trigger_renderer import TriggerRenderer
from movestate import HorizontalMoveState, VerticalMoveState


class TriggerTypeValidator(QValidator):

    def __init__(self):
        QValidator.__init__(self)

    def validate(self, contents, pos):
        in_str = str(contents)
        if in_str in events.trigger_events_by_name:
            return (QValidator.Acceptable, pos)
        else:
            return (QValidator.Intermediate, pos)


class TriggerArgumentValidator(QValidator):

    def __init__(self):
        QValidator.__init__(self)

    def validate(self, contents, pos):
        return (QValidator.Acceptable, pos)


class FieldEdit(QLineEdit):

    def __init__(self, parent):
        QLineEdit.__init__(self, parent)
        self.hide()

        self._finished_callback = None

        QObject.connect(self, SIGNAL('editingFinished()'), self._finished)

    def _finished(self):
        assert self._finished_callback
        text = unicode(self.text())
        cb = self._finished_callback
        self._finished_callback = None
        cb(text)

    def start_editing(
            self,
            x_offset,
            y_offset,
            validator,
            finished_callback,
            start_input=''):
        self.move(x_offset - 2, y_offset - 2)
        self.setText(start_input)
        self.setSelection(0, len(start_input))
        assert not self._finished_callback
        self._finished_callback = finished_callback
        self.setValidator(validator)
        self.show()
        self.setFocus()

    def keyPressEvent(self, event):
        if QLineEdit.keyPressEvent(self, event):
            return
        if event.key() == Qt.Key_Escape:
            self.parent().setFocus()
        event.accept()

    def focusOutEvent(self, event):
        self.hide()
        self._finished_callback = None


class View(QWidget):

    heightChanged = pyqtSignal(name='heightChanged')
    followCursor = pyqtSignal(str, int, name='followCursor')

    def __init__(self):
        QWidget.__init__(self)

        self._ui_model = None
        self._updater = None
        self._sheet_manager = None
        self._notation_manager = None
        self._visibility_manager = None
        self._playback_manager = None
        self._keyboard_mapper = KeyboardMapper()

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setFocusPolicy(Qt.StrongFocus)

        self._px_per_beat = None
        self._px_offset = 0
        self._pinsts = []

        self._col_width = None
        self._first_col = 0
        self._visible_cols = 0

        self._col_rends = [ColumnGroupRenderer(i) for i in xrange(COLUMN_COUNT)]

        self._heights = []
        self._start_heights = []

        self._horizontal_move_state = HorizontalMoveState()
        self._vertical_move_state = VerticalMoveState()

        self._trow_px_offset = 0

        self._field_edit = FieldEdit(self)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._sheet_manager = ui_model.get_sheet_manager()
        self._notation_manager = ui_model.get_notation_manager()
        self._visibility_manager = ui_model.get_visibility_manager()
        self._playback_manager = ui_model.get_playback_manager()

        self._keyboard_mapper.set_ui_model(ui_model)
        for cr in self._col_rends:
            cr.set_ui_model(ui_model)

        self.setFocus()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_notation' in signals:
            self._update_all_patterns()
            self.update()
        if 'signal_module' in signals:
            self._update_all_patterns()
            self.update()
        if 'signal_order_list' in signals:
            self._update_all_patterns()
            self.update()
        if 'signal_pattern_length' in signals:
            self._update_all_patterns()
            self.update()
        if 'signal_column' in signals:
            self._update_all_patterns()
            self.update()
        if 'signal_selection' in signals:
            self._follow_edit_cursor()
        if 'signal_edit_mode' in signals:
            self.update()
        if 'signal_replace_mode' in signals:
            self.update()

    def _update_all_patterns(self):
        for cr in self._col_rends:
            cr.flush_caches()
        all_pinsts = utils.get_all_pattern_instances(self._ui_model)
        self.set_pattern_instances(all_pinsts)

    def set_config(self, config):
        self._config = config
        for cr in self._col_rends:
            cr.set_config(self._config)

    def set_first_column(self, first_col):
        if self._first_col != first_col:
            self._first_col = first_col
            self.update()

    def set_pattern_instances(self, pinsts):
        self._pinsts = pinsts
        self._set_pattern_heights()
        lengths = [pinst.get_pattern().get_length() for pinst in pinsts]
        for i, cr in enumerate(self._col_rends):
            cr.set_pattern_lengths(lengths)
            columns = [pinst.get_pattern().get_column(i) for pinst in pinsts]
            cr.set_columns(columns)

    def _set_pattern_heights(self):
        lengths = [pinst.get_pattern().get_length() for pinst in self._pinsts]
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
            # Get old edit cursor offset
            location = TriggerPosition(0, 0, 0, tstamp.Tstamp(0), 0)
            if self._ui_model:
                selection = self._ui_model.get_selection()
                location = selection.get_location() or location
            orig_px_offset = self._px_offset
            orig_relative_offset = self._get_row_offset(location) or 0

            # Update internal state
            self._px_per_beat = px_per_beat
            for cr in self._col_rends:
                cr.set_px_per_beat(self._px_per_beat)
            self._set_pattern_heights()

            # Adjust vertical position so that edit cursor maintains its height
            new_cursor_offset = self._get_row_offset(location, absolute=True) or 0
            new_px_offset = new_cursor_offset - orig_relative_offset
            QObject.emit(
                    self,
                    SIGNAL('followCursor(QString, int)'),
                    str(new_px_offset),
                    self._first_col)

    def set_column_width(self, col_width):
        if self._col_width != col_width:
            self._col_width = col_width
            max_visible_cols = utils.get_max_visible_cols(self.width(), self._col_width)
            first_col = utils.clamp_start_col(self._first_col, max_visible_cols)
            visible_cols = utils.get_visible_cols(first_col, max_visible_cols)

            self._first_col = first_col
            self._visible_cols = visible_cols

            for cr in self._col_rends:
                cr.set_width(self._col_width)
            self.update()

            # Adjust horizontal position so that edit cursor is visible
            location = TriggerPosition(0, 0, 0, tstamp.Tstamp(0), 0)
            if self._ui_model:
                selection = self._ui_model.get_selection()
                location = selection.get_location() or location
            edit_col_num = location.get_col_num()

            new_first_col = self._first_col
            x_offset = self._get_col_offset(edit_col_num)
            if x_offset < 0:
                new_first_col = edit_col_num
            elif x_offset + self._col_width > self.width():
                new_first_col = edit_col_num - (self.width() // self._col_width) + 1

            max_visible_cols = utils.get_max_visible_cols(self.width(), self._col_width)
            new_first_col = utils.clamp_start_col(new_first_col, max_visible_cols)

            QObject.emit(
                    self,
                    SIGNAL('followCursor(QString, int)'),
                    str(self._px_offset),
                    new_first_col)

    def _get_col_offset(self, col_num):
        max_visible_cols = utils.get_max_visible_cols(self.width(), self._col_width)
        first_col = utils.clamp_start_col(self._first_col, max_visible_cols)
        return (col_num - first_col) * self._col_width

    def _get_row_offset(self, location, absolute=False):
        ref_offset = 0 if absolute else self._px_offset

        # Get location components
        track = location.get_track()
        system = location.get_system()
        selected_col = location.get_col_num()
        row_ts = location.get_row_ts()

        # Get pattern that contains our location
        try:
            module = self._ui_model.get_module()
            album = module.get_album()
            song = album.get_song_by_track(track)
            cur_pinst = song.get_pattern_instance(system)
        except (IndexError, AttributeError):
            return None

        for pinst, start_height in izip(self._pinsts, self._start_heights):
            if cur_pinst == pinst:
                start_px = start_height - ref_offset
                location_from_start_px = (
                        (row_ts.beats * tstamp.BEAT + row_ts.rem) *
                        self._px_per_beat) // tstamp.BEAT
                location_px = location_from_start_px + start_px
                return location_px

        return None

    def _get_init_trigger_row_width(self, rends, trigger_index):
        total_width = sum(islice(
                (r.get_total_width() for r in rends), 0, trigger_index))
        return total_width

    def _follow_trigger_row(self, location):
        module = self._ui_model.get_module()
        album = module.get_album()

        if album and album.get_track_count() > 0:
            cur_column = self._sheet_manager.get_column_at_location(location)

            row_ts = location.get_row_ts()
            if row_ts in cur_column.get_trigger_row_positions():
                notation = self._notation_manager.get_selected_notation()

                # Get trigger row width information
                trigger_index = location.get_trigger_index()
                trigger_count = cur_column.get_trigger_count_at_row(row_ts)
                triggers = [cur_column.get_trigger(row_ts, i)
                        for i in xrange(trigger_count)]
                rends = [TriggerRenderer(self._config, t, notation) for t in triggers]
                row_width = sum(r.get_total_width() for r in rends)

                init_trigger_row_width = self._get_init_trigger_row_width(
                        rends, trigger_index)

                trigger_padding = self._config['trigger']['padding']

                # Upper bound for row offset
                hollow_rect = self._get_hollow_replace_cursor_rect()
                trail_width = hollow_rect.width() + trigger_padding
                tail_offset = max(0, row_width + trail_width - self._col_width)

                max_offset = min(tail_offset, init_trigger_row_width)

                # Lower bound for row offset
                if trigger_index < len(triggers):
                    renderer = TriggerRenderer(
                            self._config, triggers[trigger_index], notation)
                    # TODO: revisit field bounds handling, this is messy
                    trigger_width = renderer.get_total_width()
                else:
                    trigger_width = trail_width
                min_offset = max(0,
                        init_trigger_row_width - self._col_width + trigger_width)

                # Final offset
                self._trow_px_offset = min(max(
                    min_offset, self._trow_px_offset), max_offset)

    def _follow_edit_cursor(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            self.update()
            return

        is_view_scrolling_required = False
        new_first_col = self._first_col
        new_y_offset = self._px_offset

        # Check column scrolling
        col_num = location.get_col_num()

        x_offset = self._get_col_offset(col_num)
        if x_offset < 0:
            is_view_scrolling_required = True
            new_first_col = col_num
        elif x_offset + self._col_width > self.width():
            is_view_scrolling_required = True
            new_first_col = col_num - (self.width() // self._col_width) + 1

        # Check scrolling inside a trigger row
        self._follow_trigger_row(location)

        # Check vertical scrolling
        y_offset = self._get_row_offset(location)
        if y_offset == None:
            self.update()
            return
        min_snap_dist = self._config['edit_cursor']['min_snap_dist']
        min_center_dist = min(min_snap_dist, self.height() // 2)
        tr_height = self._config['tr_height']
        min_y_offset = min_center_dist - tr_height // 2
        max_y_offset = self.height() - min_center_dist - tr_height // 2
        if y_offset < min_y_offset:
            is_view_scrolling_required = True
            new_y_offset = new_y_offset - (min_y_offset - y_offset)
        elif y_offset >= max_y_offset:
            is_view_scrolling_required = True
            new_y_offset = new_y_offset + (y_offset - max_y_offset)

        if is_view_scrolling_required:
            QObject.emit(
                    self,
                    SIGNAL('followCursor(QString, int)'),
                    str(new_y_offset),
                    new_first_col)
        else:
            self.update()

    def _draw_edit_cursor(self):
        if not self._pinsts:
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()

        selected_col = location.get_col_num()
        row_ts = location.get_row_ts()
        trigger_index = location.get_trigger_index()

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
        column = self._sheet_manager.get_column_at_location(location)

        try:
            # Draw the trigger row
            trigger_count = column.get_trigger_count_at_row(row_ts)
            triggers = [column.get_trigger(row_ts, i)
                    for i in xrange(trigger_count)]
            self._draw_trigger_row_with_edit_cursor(
                    painter, triggers, trigger_index)

        except KeyError:
            # No triggers, just draw a cursor
            if self._sheet_manager.get_replace_mode():
                self._draw_hollow_replace_cursor(
                        painter, self._config['trigger']['padding'], 0)
            else:
                self._draw_hollow_insert_cursor(
                        painter, self._config['trigger']['padding'], 0)

    def _get_hollow_replace_cursor_rect(self):
        metrics = self._config['font_metrics']
        rect = metrics.tightBoundingRect('a') # Seems to produce an OK width
        rect.setTop(0)
        rect.setBottom(self._config['tr_height'] - 3)
        return rect

    def _draw_hollow_replace_cursor(self, painter, x_offset, y_offset):
        rect = self._get_hollow_replace_cursor_rect()
        rect.translate(x_offset, y_offset)
        painter.setPen(self._config['trigger']['default_colour'])
        painter.drawRect(rect)

    def _draw_insert_cursor(self, painter, x_offset, y_offset):
        rect = QRect(QPoint(0, 0), QPoint(2, self._config['tr_height'] - 2))
        rect.translate(x_offset, y_offset)
        painter.fillRect(rect, self._config['trigger']['default_colour'])

    def _draw_hollow_insert_cursor(self, painter, x_offset, y_offset):
        rect = QRect(QPoint(0, 0), QPoint(1, self._config['tr_height'] - 3))
        rect.translate(x_offset, y_offset)
        painter.setPen(self._config['trigger']['default_colour'])
        painter.drawRect(rect)

    def _draw_trigger_row_with_edit_cursor(self, painter, triggers, trigger_index):
        painter.save()

        painter.setClipRect(QRect(
            QPoint(0, 0), QPoint(self._col_width - 2, self._config['tr_height'])))

        # Hide underlying column contents
        painter.fillRect(
                QRect(QPoint(0, 1),
                    QPoint(self._col_width, self._config['tr_height'] - 1)),
                self._config['bg_colour'])

        notation = self._notation_manager.get_selected_notation()
        rends = [TriggerRenderer(self._config, trigger, notation)
                for trigger in triggers]
        widths = [r.get_total_width() for r in rends]
        total_width = sum(widths)

        trigger_tfm = painter.transform().translate(-self._trow_px_offset, 0)
        painter.setTransform(trigger_tfm)

        for i, trigger, renderer in izip(xrange(len(triggers)), triggers, rends):
            # Identify selected field
            if self._sheet_manager.get_replace_mode():
                select_replace = (i == trigger_index)
            else:
                if i == trigger_index:
                    self._draw_insert_cursor(painter, 0, 0)
                select_replace = False

            # Render
            renderer.draw_trigger(painter, False, select_replace)

            # Update transform
            trigger_tfm = trigger_tfm.translate(renderer.get_total_width(), 0)
            painter.setTransform(trigger_tfm)

        if trigger_index >= len(triggers):
            # Draw cursor at the end of the row
            if self._sheet_manager.get_replace_mode():
                self._draw_hollow_replace_cursor(painter, 0, 0)
            else:
                self._draw_hollow_insert_cursor(painter, 0, 0)

        painter.restore()

    def _move_edit_cursor_trow(self):
        delta = self._horizontal_move_state.get_delta()
        assert delta != 0

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

        cur_column = self._sheet_manager.get_column_at_location(location)

        if row_ts not in cur_column.get_trigger_row_positions():
            # No triggers
            return

        notation = self._notation_manager.get_selected_notation()

        if delta < 0:
            if trigger_index == 0:
                # Already at the start of the row
                return

            # Previous trigger
            prev_trigger_index = trigger_index - 1
            new_location = TriggerPosition(
                    track, system, col_num, row_ts, prev_trigger_index)
            selection.set_location(new_location)
            return

        elif delta > 0:
            if trigger_index >= cur_column.get_trigger_count_at_row(row_ts):
                # Already at the end of the row
                return

            # Next trigger
            next_trigger_index = trigger_index + 1
            new_location = TriggerPosition(
                    track, system, col_num, row_ts, next_trigger_index)
            selection.set_location(new_location)
            return

    def _move_edit_cursor_trigger_index(self, index):
        module = self._ui_model.get_module()
        album = module.get_album()
        if not album or album.get_track_count() == 0:
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()

        test_location = TriggerPosition(
                location.get_track(),
                location.get_system(),
                location.get_col_num(),
                location.get_row_ts(),
                index)
        new_location = self._sheet_manager.get_clamped_location(test_location)

        selection.set_location(new_location)

    def _move_edit_cursor_column(self, delta):
        assert delta != 0

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

        new_col_num = min(max(0, col_num + delta), COLUMN_COUNT - 1)

        test_location = TriggerPosition(track, system, new_col_num, row_ts, 0)
        new_location = self._sheet_manager.get_clamped_location(test_location)
        selection.set_location(new_location)

    def _move_edit_cursor_tstamp(self):
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

        # Check moving to the previous system
        move_to_previous_system = False
        if px_delta < 0 and row_ts == 0:
            if track == 0 and system == 0:
                return
            else:
                move_to_previous_system = True

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
                new_ts = new_pattern.get_length()

                track = new_track
                system = new_system
                row_ts = new_ts

                location = TriggerPosition(
                        track, system, col_num, row_ts, trigger_index)

        cur_column = self._sheet_manager.get_column_at_location(location)

        # Get default trigger tstamp on the current pixel position
        cur_px_offset = utils.get_px_from_tstamp(row_ts, self._px_per_beat)
        def_ts = utils.get_tstamp_from_px(cur_px_offset, self._px_per_beat)
        assert utils.get_px_from_tstamp(def_ts, self._px_per_beat) == cur_px_offset

        # Get target tstamp
        new_px_offset = cur_px_offset + px_delta
        new_ts = utils.get_tstamp_from_px(new_px_offset, self._px_per_beat)
        assert utils.get_px_from_tstamp(new_ts, self._px_per_beat) != cur_px_offset

        # Get shortest movement between target tstamp and closest trigger row
        move_range_start = min(new_ts, row_ts)
        move_range_stop = max(new_ts, row_ts) + tstamp.Tstamp(0, 1)
        if px_delta < 0:
            if not move_to_previous_system:
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
        cur_song = album.get_song_by_track(track)
        cur_pattern = cur_song.get_pattern_instance(system).get_pattern()

        if new_ts <= 0:
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
                            track, system, col_num, new_ts, 0)
                    selection.set_location(new_location)
                    return

            # Next pattern
            self._vertical_move_state.try_snap_delay()
            new_ts = tstamp.Tstamp(0)
            new_location = TriggerPosition(
                    new_track, new_system, col_num, new_ts, 0)
            selection.set_location(new_location)
            return

        # Move inside pattern
        new_location = TriggerPosition(track, system, col_num, new_ts, 0)
        selection.set_location(new_location)

    def _get_trigger_index(self, column, row_ts, x_offset):
        if not column.has_trigger(row_ts, 0):
            return -1

        trigger_count = column.get_trigger_count_at_row(row_ts)
        triggers = (column.get_trigger(row_ts, i)
                for i in xrange(trigger_count))
        notation = self._notation_manager.get_selected_notation()
        rends = (TriggerRenderer(self._config, trigger, notation)
                for trigger in triggers)
        widths = [r.get_total_width() for r in rends]
        init_width = 0
        trigger_index = 0
        for width in widths:
            prev_init_width = init_width
            init_width += width
            if init_width >= x_offset:
                if not self._sheet_manager.get_replace_mode():
                    if (init_width - x_offset) < (x_offset - prev_init_width):
                        trigger_index += 1
                break
            trigger_index += 1
        else:
            hollow_rect = self._get_hollow_replace_cursor_rect()
            dist_from_last = x_offset - init_width
            trigger_padding = self._config['trigger']['padding']
            if dist_from_last > hollow_rect.width() + trigger_padding:
                return -1

        assert trigger_index <= trigger_count
        return trigger_index

    def _select_location(self, view_x_offset, view_y_offset):
        module = self._ui_model.get_module()
        album = module.get_album()
        if not album:
            return
        track_count = album.get_track_count()
        songs = (album.get_song_by_track(i) for i in xrange(track_count))
        if not songs:
            return

        # Get column number
        col_num = self._first_col + (view_x_offset // self._col_width)
        if col_num >= COLUMN_COUNT:
            return
        rel_x_offset = view_x_offset % self._col_width

        # Get pattern index
        y_offset = self._px_offset + view_y_offset
        pat_index = utils.get_first_visible_pat_index(y_offset, self._start_heights)
        pat_index = min(pat_index, len(self._pinsts) - 1)

        # Get track and system
        track = -1
        system = pat_index
        for song in songs:
            track += 1
            system_count = song.get_system_count()
            if system >= system_count:
                system -= system_count
            else:
                break
        if track < 0:
            return

        # Get row timestamp
        rel_y_offset = y_offset - self._start_heights[pat_index]
        assert rel_y_offset >= 0
        row_ts = utils.get_tstamp_from_px(rel_y_offset, self._px_per_beat)
        row_ts = min(row_ts, self._pinsts[pat_index].get_pattern().get_length())

        # Get current selection info
        selection = self._ui_model.get_selection()
        cur_location = selection.get_location()
        cur_column = self._sheet_manager.get_column_at_location(cur_location)

        # Select a trigger if its description overlaps with the mouse cursor
        trigger_index = 0
        tr_track, tr_system = track, system
        tr_pat_index = pat_index
        while tr_pat_index >= 0:
            tr_location = TriggerPosition(
                    tr_track, tr_system, col_num, tstamp.Tstamp(0), 0)
            column = self._sheet_manager.get_column_at_location(tr_location)
            if not column:
                break

            # Get range for checking
            start_ts = tstamp.Tstamp(0)
            if tr_pat_index == pat_index:
                stop_ts = row_ts
                tr_rel_y_offset = rel_y_offset
            else:
                stop_ts = self._pinsts[tr_pat_index].get_pattern().get_length()
                tr_rel_y_offset = self._heights[tr_pat_index] + rel_y_offset - 1
            stop_ts += tstamp.Tstamp(0, 1)

            # Get check location
            trow_tstamps = column.get_trigger_row_positions_in_range(
                    start_ts, stop_ts)
            if trow_tstamps:
                check_ts = max(trow_tstamps)
            else:
                check_ts = tstamp.Tstamp(0)

            # Get pixel distance to the click position
            check_y_offset = utils.get_px_from_tstamp(check_ts, self._px_per_beat)
            y_dist = tr_rel_y_offset - check_y_offset
            assert y_dist >= 0
            is_close_enough = (y_dist < self._config['tr_height'] - 1)
            if not is_close_enough:
                break

            # Override check location if we clicked on a currently overlaid trigger
            if (cur_column and
                    cur_location.get_track() <= tr_track and
                    cur_location.get_col_num() == col_num):
                # Get current location offset
                cur_track = cur_location.get_track()
                cur_system = cur_location.get_system()
                cur_pat_index = utils.get_pattern_index_at_location(
                        self._ui_model, cur_track, cur_system)

                cur_ts = cur_location.get_row_ts()
                cur_pat_y_offset = utils.get_px_from_tstamp(cur_ts, self._px_per_beat)
                cur_y_offset = self._start_heights[cur_pat_index] + cur_pat_y_offset

                tr_y_offset = self._start_heights[pat_index] + tr_rel_y_offset
                cur_y_dist = tr_y_offset - cur_y_offset

                if cur_y_dist >= 0 and cur_y_dist < self._config['tr_height'] - 1:
                    tr_rel_x_offset = rel_x_offset + self._trow_px_offset
                    new_trigger_index = self._get_trigger_index(
                            cur_column, cur_ts, tr_rel_x_offset)
                    if new_trigger_index >= 0:
                        trigger_index = new_trigger_index
                        track, system, row_ts = cur_track, cur_system, cur_ts
                        break

            if trow_tstamps:
                # If this trow is already selected, consider additional row offset
                if (cur_location.get_track() == tr_track and
                        cur_location.get_system() == tr_system and
                        cur_location.get_col_num() == col_num and
                        cur_location.get_row_ts() == check_ts):
                    tr_rel_x_offset = rel_x_offset + self._trow_px_offset
                else:
                    tr_rel_x_offset = rel_x_offset

                # Get trigger index
                new_trigger_index = self._get_trigger_index(
                        column, check_ts, tr_rel_x_offset)
                if new_trigger_index >= 0:
                    trigger_index = new_trigger_index
                    track, system, row_ts = tr_track, tr_system, check_ts
                    break

            # Check previous system
            tr_pat_index -= 1
            tr_system -= 1
            while tr_system < 0:
                tr_track -= 1
                if tr_track < 0:
                    assert tr_pat_index < 0
                    break
                song = album.get_song_by_track(tr_track)
                tr_system = song.get_system_count() - 1

        location = TriggerPosition(track, system, col_num, row_ts, trigger_index)
        selection.set_location(location)

    def _add_rest(self):
        trigger = Trigger('n-', None)
        self._sheet_manager.add_trigger(trigger)

    def _try_delete_selection(self):
        self._sheet_manager.try_remove_trigger()

    def _perform_backspace(self):
        if not self._sheet_manager.is_editing_enabled():
            return

        module = self._ui_model.get_module()
        album = module.get_album()
        if not album or album.get_track_count() == 0:
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        trigger_index = location.get_trigger_index()
        if trigger_index > 0:
            self._move_edit_cursor_trigger_index(trigger_index - 1)
            self._try_delete_selection()

    def _get_selected_coordinates(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()

        x_offset = self._get_col_offset(location.get_col_num())
        y_offset = self._get_row_offset(location)

        column = self._sheet_manager.get_column_at_location(location)
        if not column:
            return None
        row_ts = location.get_row_ts()
        notation = self._notation_manager.get_selected_notation()

        try:
            trigger_count = column.get_trigger_count_at_row(row_ts)
            triggers = [column.get_trigger(row_ts, i)
                    for i in xrange(min(trigger_count, location.get_trigger_index()))]
            rends = [TriggerRenderer(self._config, trigger, notation)
                    for trigger in triggers]
            widths = [r.get_total_width() for r in rends]
            init_width = sum(widths)
            x_offset += init_width
        except KeyError:
            pass

        return (x_offset, y_offset)

    def _start_trigger_type_entry(self):
        if (not self._sheet_manager.is_editing_enabled() or
                self._playback_manager.is_recording()):
            return

        self._follow_edit_cursor()

        coords = self._get_selected_coordinates()
        if not coords:
            return

        x_offset, y_offset = coords

        validator = TriggerTypeValidator()
        self._field_edit.start_editing(
                x_offset, y_offset, validator, self._finish_trigger_type_entry)
        self._field_edit.show()
        self._field_edit.setFocus()

    def _get_example_trigger_argument(self, event_name):
        info = events.trigger_events_by_name[event_name]

        # Special case: tempo values
        if info['name'] in ('m.t', 'm/t'):
            return u'120'

        ex = {
            None                    : None,
            events.EVENT_ARG_BOOL   : u'false',
            events.EVENT_ARG_INT    : u'0',
            events.EVENT_ARG_FLOAT  : u'0',
            events.EVENT_ARG_TSTAMP : u'0',
            events.EVENT_ARG_STRING : u"''",
            events.EVENT_ARG_PAT    : u'pat(0, 0)',
            events.EVENT_ARG_PITCH  : u'0',
        }

        return ex[info['arg_type']]

    def _finish_trigger_type_entry(self, text):
        selection = self._ui_model.get_selection()
        orig_location = selection.get_location()

        self.setFocus()
        arg = self._get_example_trigger_argument(text)
        trigger = Trigger(text, arg)
        self._sheet_manager.add_trigger(trigger)

        arg_type = events.trigger_events_by_name[text]['arg_type']
        if arg_type not in (None, events.EVENT_ARG_PITCH):
            selection.set_location(orig_location)
            self._start_trigger_argument_entry(new=True)

    def _start_trigger_argument_entry(self, new=False):
        if (not self._sheet_manager.is_editing_enabled() or
                self._playback_manager.is_recording()):
            return

        self._follow_edit_cursor()

        coords = self._get_selected_coordinates()
        if not coords:
            return

        x_offset, y_offset = coords

        trigger = self._sheet_manager.get_selected_trigger()
        if trigger.get_argument() == None:
            return

        # Offset field edit so that trigger type remains visible
        if trigger.get_type() not in ('n+', 'h'):
            notation = self._notation_manager.get_selected_notation()
            renderer = TriggerRenderer(self._config, trigger, notation)
            _, type_width = renderer.get_field_bounds(0)
            x_offset += type_width

        validator = TriggerArgumentValidator()
        start_text = trigger.get_argument()
        if new:
            start_text = self._get_example_trigger_argument(trigger.get_type())

        self._field_edit.start_editing(
                x_offset,
                y_offset,
                validator,
                self._finish_trigger_argument_entry,
                start_text)

    def _finish_trigger_argument_entry(self, text):
        self.setFocus()

        trigger = self._sheet_manager.get_selected_trigger()
        new_trigger = Trigger(trigger.get_type(), text)

        if not self._sheet_manager.get_replace_mode():
            self._sheet_manager.try_remove_trigger()
        self._sheet_manager.add_trigger(new_trigger)

    def event(self, ev):
        if ev.type() == QEvent.KeyPress and ev.key() in (Qt.Key_Tab, Qt.Key_Backtab):
            return self.keyPressEvent(ev) or False
        return QWidget.event(self, ev)

    def keyPressEvent(self, event):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        sheet_manager = self._ui_model.get_sheet_manager()
        control_id = sheet_manager.get_inferred_active_control_id_at_location(location)

        control_manager = self._ui_model.get_control_manager()
        control_manager.set_control_id_override(control_id)
        note_pressed = self._keyboard_mapper.process_typewriter_button_event(event)
        control_manager.set_control_id_override(None)
        if note_pressed:
            return

        if event.key() == Qt.Key_Tab:
            event.accept()
            self._move_edit_cursor_column(1)
            return True
        elif event.key() == Qt.Key_Backtab:
            event.accept()
            self._move_edit_cursor_column(-1)
            return True

        def handle_move_up():
            self._vertical_move_state.press_up()
            self._move_edit_cursor_tstamp()

        def handle_move_down():
            self._vertical_move_state.press_down()
            self._move_edit_cursor_tstamp()

        def handle_move_left():
            self._horizontal_move_state.press_left()
            self._move_edit_cursor_trow()

        def handle_move_right():
            self._horizontal_move_state.press_right()
            self._move_edit_cursor_trow()

        def handle_rest():
            if not event.isAutoRepeat():
                self._add_rest()

        def handle_typewriter_connection():
            if not event.isAutoRepeat():
                connected = self._sheet_manager.get_typewriter_connected()
                self._sheet_manager.set_typewriter_connected(not connected)

        def handle_replace_mode():
            if not event.isAutoRepeat():
                is_replace = self._sheet_manager.get_replace_mode()
                self._sheet_manager.set_replace_mode(not is_replace)

        def handle_field_edit():
            if (self._sheet_manager.get_replace_mode() and
                    self._sheet_manager.is_at_trigger()):
                self._start_trigger_argument_entry()
            else:
                self._start_trigger_type_entry()

        def handle_decrease_column_width():
            if cmdline.get_experimental():
                self._sheet_manager.set_column_width(
                        self._sheet_manager.get_column_width() - 1)

        def handle_increase_column_width():
            if cmdline.get_experimental():
                self._sheet_manager.set_column_width(
                        self._sheet_manager.get_column_width() + 1)

        def handle_reset_column_width():
            if cmdline.get_experimental():
                self._sheet_manager.set_column_width(0)

        keymap = {
            int(Qt.NoModifier): {
                Qt.Key_Up:      handle_move_up,
                Qt.Key_Down:    handle_move_down,
                Qt.Key_Left:    handle_move_left,
                Qt.Key_Right:   handle_move_right,

                Qt.Key_Home:    lambda: self._move_edit_cursor_trigger_index(0),
                Qt.Key_End: lambda: self._move_edit_cursor_trigger_index(2**24), # :-P

                # TODO: Some rare keyboard layouts have the 1 key in a location
                #       that interferes with the typewriter
                Qt.Key_1:       handle_rest,

                Qt.Key_Delete:  lambda: self._try_delete_selection(),
                Qt.Key_Backspace: lambda: self._perform_backspace(),

                Qt.Key_Space:   handle_typewriter_connection,
                Qt.Key_Insert:  handle_replace_mode,

                Qt.Key_Return:  handle_field_edit,
            },

            int(Qt.ControlModifier): {
                Qt.Key_Minus:   lambda: self._sheet_manager.set_zoom(
                                    self._sheet_manager.get_zoom() - 1),
                Qt.Key_Plus:    lambda: self._sheet_manager.set_zoom(
                                    self._sheet_manager.get_zoom() + 1),
                Qt.Key_0:       lambda: self._sheet_manager.set_zoom(0),
            },

            int(Qt.ControlModifier | Qt.AltModifier): {
                Qt.Key_Minus:   handle_decrease_column_width,
                Qt.Key_Plus:    handle_increase_column_width,
                Qt.Key_0:       handle_reset_column_width,
            },
        }

        is_handled = False
        mod_map = keymap.get(int(event.modifiers()))
        if mod_map:
            func = mod_map.get(event.key())
            if func:
                func()
                is_handled = True

        if not is_handled:
            QWidget.keyPressEvent(self, event)

    def keyReleaseEvent(self, ev):
        if self._keyboard_mapper.process_typewriter_button_event(ev):
            return

        if ev.isAutoRepeat():
            return

        if ev.key() == Qt.Key_Up:
            self._vertical_move_state.release_up()
        elif ev.key() == Qt.Key_Down:
            self._vertical_move_state.release_down()
        elif ev.key() == Qt.Key_Left:
            self._horizontal_move_state.release_left()
        elif ev.key() == Qt.Key_Right:
            self._horizontal_move_state.release_right()

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
        if self._sheet_manager.get_edit_mode():
            self._draw_edit_cursor()

        if pixmaps_created == 0:
            pass # TODO: update was easy, predraw a likely next pixmap
        else:
            print('{} column pixmap{} created'.format(
                pixmaps_created, 's' if pixmaps_created != 1 else ''))

        end = time.time()
        elapsed = end - start
        memory_usage = sum(cr.get_memory_usage() for cr in self._col_rends)
        print('View updated in {:.2f} ms, cache size {:.2f} MB'.format(
            elapsed * 1000, memory_usage / float(2**20)))

    def focusInEvent(self, ev):
        self._sheet_manager.set_edit_mode(True)

    def focusOutEvent(self, ev):
        allow_signals = (
                self._visibility_manager.is_show_allowed() and
                not self._ui_model.get_module().is_saving())

        if allow_signals:
            self._sheet_manager.set_edit_mode(False)
            self._sheet_manager.set_chord_mode(False)

    def mousePressEvent(self, event):
        if event.buttons() == Qt.LeftButton:
            self._select_location(event.x(), event.y())


