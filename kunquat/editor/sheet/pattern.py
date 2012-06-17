# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division
from __future__ import print_function
import math
import sys

import kunquat
from PyQt4 import QtGui, QtCore

import accessors as acc
from column import Column
from cursor import Cursor
import kunquat.editor.keymap as keymap
import kunquat.editor.kqt_limits as lim
import kunquat.editor.note_input as ni
import kunquat.editor.scale as scale
import kunquat.editor.timestamp as ts
import kunquat.editor.trigtypes as ttypes
import trigger


default_scale = scale.Scale({
        'ref_pitch': 440 * 2**(3.0/12),
        'octave_ratio': ['/', [2, 1]],
        'notes': list(zip(('C', 'C#', 'D', 'D#', 'E', 'F',
                           'F#', 'G', 'G#', 'A', 'A#', 'B'),
                          (['c', cents] for cents in range(0, 1200, 100))))
    })


default_input = ni.NoteInput()


class Pattern(QtGui.QWidget):

    pattern_changed = QtCore.pyqtSignal(int, name='patternChanged')

    def __init__(self,
                 project,
                 section,
                 playback,
                 pattern_offset_changed_slot,
                 octave_spin,
                 instrument_spin,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        section.connect(self.section_changed)
        self.number = 0
        self.section_manager = section
        self.playback_manager = playback
        self.setSizePolicy(QtGui.QSizePolicy.Ignored,
                           QtGui.QSizePolicy.Ignored)
        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.first_column = 0
        self.colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'column_border': QtGui.QColor(0xcc, 0xcc, 0xcc),
                'column_head_bg': QtGui.QColor(0x33, 0x77, 0x22),
                'column_head_fg': QtGui.QColor(0xff, 0xee, 0xee),
                'cursor_bg': QtGui.QColor(0xff, 0x66, 0x22, 0x77),
                'cursor_line': QtGui.QColor(0xff, 0xee, 0x88),
                'cursor_arrow': QtGui.QColor(0xff, 0x44, 0x22),
                'grid': QtGui.QPen(QtGui.QColor(0x66, 0x66, 0x66)),
                'ruler_bg': QtGui.QColor(0x11, 0x22, 0x55),
                'ruler_cur': QtGui.QColor(0x77, 0x99, 0xbb),
                'ruler_fg': QtGui.QColor(0xaa, 0xcc, 0xff),
                'ruler_play_cur': QtGui.QColor(0xff, 0xaa, 0x77),
                'trigger_fg': QtGui.QColor(0xaa, 0xaa, 0xaa),
                'trigger_note_on_fg': QtGui.QColor(0xee, 0xcc, 0xaa),
                'trigger_hit_fg': QtGui.QColor(0xaa, 0xee, 0xaa),
                'trigger_note_off_fg': QtGui.QColor(0xaa, 0x88, 0x66),
                'trigger_type_fg': QtGui.QColor(0xcc, 0xcc, 0xaa),
                'trigger_invalid_fg': QtGui.QColor(0xff, 0x33, 0x11),
                }
        self.colours['grid'].setStyle(QtCore.Qt.DashLine)
        self.fonts = {
                'column_head': QtGui.QFont('Decorative', 10),
                'ruler': QtGui.QFont('Decorative', 8),
                'trigger': QtGui.QFont('Decorative', 10),
                }
        self._keys = keymap.KeyMap('Pattern editing keys', {
                (QtCore.Qt.Key_Up, QtCore.Qt.ControlModifier):
                        (self._zoom_in, None),
                (QtCore.Qt.Key_Down, QtCore.Qt.ControlModifier):
                        (self._zoom_out, None),
                (QtCore.Qt.Key_Left, QtCore.Qt.ControlModifier):
                        (self._shrink_columns, None),
                (QtCore.Qt.Key_Right, QtCore.Qt.ControlModifier):
                        (self._expand_columns, None),
                (QtCore.Qt.Key_Plus, QtCore.Qt.ControlModifier):
                        (self._next_section, None),
                (QtCore.Qt.Key_Minus, QtCore.Qt.ControlModifier):
                        (self._prev_section, None),
                (QtCore.Qt.Key_Insert, QtCore.Qt.ShiftModifier):
                        (self._shift_down, self._reset_shift),
                (QtCore.Qt.Key_Delete, QtCore.Qt.ShiftModifier):
                        (self._shift_up, self._reset_shift),
                (QtCore.Qt.Key_Insert, None):
                        (None, self._reset_shift),
                (QtCore.Qt.Key_Delete, None):
                        (None, lambda ev: self._reset_shift(ev)), # id crisis
                (QtCore.Qt.Key_Left, QtCore.Qt.ShiftModifier):
                        (self._prev_column, None),
                (QtCore.Qt.Key_Right, QtCore.Qt.ShiftModifier):
                        (self._next_column, None),
                (QtCore.Qt.Key_Left, QtCore.Qt.NoModifier):
                        (self._cursor_left, None),
                (QtCore.Qt.Key_Right, QtCore.Qt.NoModifier):
                        (self._cursor_right, None),
                })
        self.length = ts.Timestamp(16)
        self.beat_len = 80
        self.view_start = ts.Timestamp(0)
        self.columns = [Column(num, (self.colours, self.fonts))
                        for num in xrange(0, lim.COLUMNS_MAX)]
        for col in self.columns:
            col.set_length(self.length)
            col.set_beat_len(self.beat_len)
            col.set_view_start(self.view_start)

        self.accessors = {
                trigger.TriggerType: acc.TypeEdit(self),
                ttypes.Note: acc.NoteEdit(self),
                ttypes.HitIndex: acc.HitIndexEdit(self),
                bool: acc.BoolEdit(self),
                float: acc.FloatEdit(self),
                int: acc.IntEdit(self),
                ts.Timestamp: acc.TimestampEdit(self),
                str: acc.StringEdit(self),
                unicode: acc.StringEdit(self),
                }
        acc_palette = QtGui.QPalette(self.colours['trigger_fg'],
                                     self.colours['bg'],
                                     self.colours['column_border'],
                                     self.colours['column_border'],
                                     self.colours['column_border'],
                                     self.colours['trigger_fg'],
                                     self.colours['bg'])
        for a in self.accessors:
            self.accessors[a].hide()
            self.accessors[a].setPalette(acc_palette)
            QtCore.QObject.connect(self.accessors[a],
                                   QtCore.SIGNAL('returnPressed()'),
                                   self.value_changed)

        self.cursor = Cursor(self.length,
                             self.beat_len,
                             self.accessors,
                             playback,
                             instrument_spin)
        QtCore.QObject.connect(self.cursor,
                               QtCore.SIGNAL('fieldEdit(bool)'),
                               self.field_edit)
        QtCore.QObject.connect(self.cursor,
                               QtCore.SIGNAL('nextCol()'),
                               self.visit_next_col)
        QtCore.QObject.connect(self.cursor,
                            QtCore.SIGNAL('patternOffsetChanged(int, int)'),
                            pattern_offset_changed_slot)
        self.ruler = Ruler((self.colours, self.fonts), self.cursor)
        self.ruler.set_length(self.length)
        self.ruler.set_beat_len(self.beat_len)
        self.ruler.set_view_start(self.view_start)

        self._grid = Grid((self.colours, self.fonts), self.ruler.width())
        self._grid.length = self.length
        self._grid.beat_len = self.beat_len
        self._grid.view_start = self.view_start
        self.cursor.grid = self._grid
        self._grid.enabled = True
        self._grid.snap = True

        self.set_project(project)
        self.cursor.set_scale(default_scale)
        self.note_input = default_input
        self.cursor.set_input(self.note_input)
        QtCore.QObject.connect(octave_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.octave_changed)
        QtCore.QObject.connect(instrument_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.inst_changed)
        self.columns[0].set_cursor(self.cursor)
        self.cursor.set_col(self.columns[0])
        self.cursor_col = 0
        self.orig_cursor_col = None
        self.view_columns = []
        self.width = 0
        self.height = 0
        self.cursor_center_area = 0.3
        self.zoom_factor = 1.2

        self._play_pattern = -1
        self._play_row = ts.Timestamp(0)
        self.project.set_callback('Apattern', self._update_play_pattern)
        self.project.set_callback('Arow', self._update_play_row)

        self.setAutoFillBackground(False)
        self.setAttribute(QtCore.Qt.WA_OpaquePaintEvent)
        self.setAttribute(QtCore.Qt.WA_NoSystemBackground)

    def autoinst_changed(self, value):
        self.cursor.inst_auto = bool(value)

    def grid_changed(self, value):
        self._grid.enabled = bool(value)
        self.update()

    def snap_to_grid_changed(self, value):
        self._grid.snap = bool(value)

    def field_edit(self, edit):
        if edit:
            self.setFocusPolicy(QtCore.Qt.NoFocus)
        else:
            self.setFocusPolicy(QtCore.Qt.StrongFocus)

    def inst_changed(self, num):
        self.cursor.inst_num = num

    def length_changed(self, *flength):
        self.set_pattern(self.number)
        self.update()

    def section_changed(self, *args):
        subsong, section = args
        pattern = self.project._composition.get_pattern(subsong, section)
        if pattern != None:
            self.set_pattern(pattern)
            self.update()

    def octave_changed(self, octave):
        self.note_input.base_octave = octave

    def set_pattern(self, num):
        self.path = 'pat_{0:03x}'.format(num)
        pat_info = self.project['/'.join((self.path, 'p_pattern.json'))]
        if pat_info and 'length' in pat_info:
            self.length = ts.Timestamp(pat_info['length'])
        else:
            self.length = ts.Timestamp(16)
        self.cursor.set_length(self.length)
        self.ruler.set_length(self.length)
        self._grid.length = self.length
        self.cursor.set_path(self.path)
        for col in self.columns:
            col.set_length(self.length)
        #self.columns[0].arrange_triggers(self.project['/'.join(
        #                (self.path, 'gcol', 'p_global_events.json'))])
        for col in self.columns:
            col_dir = 'col_{0:02x}'.format(col.get_num())
            path = '/'.join((self.path, col_dir, 'p_events.json'))
            col.arrange_triggers(self.project[path])
        if self.number != num:
            QtCore.QObject.emit(self, QtCore.SIGNAL('patternChanged(int)'),
                                num)
        self.number = num

    def sync(self):
        self.set_pattern(self.number)
        self.update()

    def set_project(self, project):
        self.project = project
        self.cursor.set_project(project)
        self.set_pattern(0)

    def value_changed(self):
        self.cursor.set_value()
        self.setFocus()
        self.update()

#    def sizeHint(self):
#        return QtCore.QSize(100, 100)

    def follow_cursor_horizontal(self):
        if not self.view_columns or self.cursor_col in self.view_columns:
            return False
        elif self.cursor_col < self.view_columns[0].get_num():
            self.first_column = self.cursor_col
            self.view_columns = list(self.get_viewable_columns(self.width))
            return True
        elif self.cursor_col > self.view_columns[-1].get_num():
            left_cols = list(self.get_viewable_columns(self.width,
                                                       self.cursor_col, -1))
            if left_cols:
                self.first_column = left_cols[-1].get_num()
                self.view_columns = list(self.get_viewable_columns(self.width))
            return True
        return False

    def follow_cursor_vertical(self):
        col_head_height = QtGui.QFontMetrics(
                              self.fonts['column_head']).height()
        trigger_height = QtGui.QFontMetrics(self.fonts['trigger']).height()
        pix_view_start = self.view_start * self.beat_len
        pix_pat_end = (self.length - self.view_start) * self.beat_len
        pix_pat_len = self.length * self.beat_len
        cur_view_pos = self.cursor.get_pix_pos() - pix_view_start
        area_height = self.height - col_head_height
        neg_pix_shift = cur_view_pos - area_height * (0.5 -
                                           (self.cursor_center_area / 2))
        neg_pix_shift += trigger_height / 2
        pos_pix_shift = cur_view_pos - area_height * (0.5 +
                                           (self.cursor_center_area / 2))
        pos_pix_shift += trigger_height / 2
        if neg_pix_shift < 0:
            pix_view_start = max(0, pix_view_start + neg_pix_shift)
        elif pos_pix_shift > 0:
            pix_view_start += pos_pix_shift
            if pix_pat_len - pix_view_start < area_height - trigger_height:
                pix_view_start = pix_pat_len - area_height + trigger_height
                pix_view_start = max(0, pix_view_start)
        if neg_pix_shift < 0 or pos_pix_shift > 0:
            self.view_start = ts.Timestamp(pix_view_start / self.beat_len)
            self.ruler.set_view_start(self.view_start)
            self._grid.view_start = self.view_start
            for col in self.columns:
                col.set_view_start(self.view_start)
            return True
        return False

    def keyPressEvent(self, ev):
        if self.cursor.edit:
            if ev.key() in (QtCore.Qt.Key_Escape,):
                self.cursor.edit = False
                self.setFocus()
            return
        self._keys.call(ev)
        if not ev.isAccepted():
            self.cursor.key_press(ev)
            if ev.isAccepted():
                self.follow_cursor_vertical()
                self.update()

    def keyReleaseEvent(self, ev):
        if ev.isAutoRepeat():
            return
        self._keys.rcall(ev)
        if ev.isAccepted():
            return
        if ev.key() in (QtCore.Qt.Key_Up, QtCore.Qt.Key_Down):
            self.cursor.key_release(ev)
        if not ev.isAccepted() and ev.key() == QtCore.Qt.Key_Shift:
            if self.orig_cursor_col != None and False:
                # FIXME: recognise shift+number
                while self.cursor_col != self.orig_cursor_col and \
                        self.cursor_col > -1:
                    self._prev_column(ev)
                self.orig_cursor_col = None

    def visit_next_col(self):
        return # FIXME: recognise shift+number
        if self.orig_cursor_col == None:
            self.orig_cursor_col = self.cursor_col
        self._next_column(None)

    def _zoom_in(self, ev):
        self.zoom(self.zoom_factor)

    def _zoom_out(self, ev):
        self.zoom(1 / self.zoom_factor)

    def _shrink_columns(self, ev):
        for col in self.columns:
            col.set_width(col.width() / self.zoom_factor)
        self.view_columns = list(self.get_viewable_columns(self.width))
        self.follow_cursor_horizontal()
        self._grid.width = sum(x.width() for x in self.view_columns)
        self.update()

    def _expand_columns(self, ev):
        for col in self.columns:
            col.set_width(col.width() * self.zoom_factor)
        self.view_columns = list(self.get_viewable_columns(self.width))
        self.follow_cursor_horizontal()
        self._grid.width = sum(x.width() for x in self.view_columns)
        self.update()

    def _next_section(self, ev):
        subsong = self.section_manager.subsong
        section = self.section_manager.section
        if section < lim.SECTIONS_MAX - 1:
            self.section_manager.set(subsong, section + 1)

    def _prev_section(self, ev):
        subsong = self.section_manager.subsong
        section = self.section_manager.section
        if section > 0:
            self.section_manager.set(subsong, section - 1)

    def _shift(self, direction):
        self.cursor.set_index(0)
        shift_pos = self.cursor.get_pos()
        self.cursor.set_direction(1)
        self.cursor.step()
        if direction > 0:
            self.cursor.clear_delay()
        shift = self.cursor.get_pos() - shift_pos
        self.columns[self.cursor_col].shift(shift_pos, shift * direction)
        self.cursor.set_pos(shift_pos)
        self.project[self.cursor.col_path] = \
                self.columns[self.cursor_col].flatten()
        self.update()

    def _shift_down(self, ev):
        self._shift(1)

    def _shift_up(self, ev):
        self._shift(-1)

    def _reset_shift(self, ev):
        assert ev.type() == QtCore.QEvent.KeyRelease
        self.cursor.set_direction()

    def _prev_column(self, ev):
        if self.cursor_col > 0:
            self.columns[self.cursor_col].set_cursor()
            self.columns[self.cursor_col - 1].set_cursor(self.cursor)
            self.cursor.set_col(self.columns[self.cursor_col - 1])
            self.cursor_col -= 1
            self.follow_cursor_horizontal()
            self.update()

    def _next_column(self, ev):
        if self.cursor_col < lim.COLUMNS_MAX - 1:
            self.columns[self.cursor_col].set_cursor()
            self.columns[self.cursor_col + 1].set_cursor(self.cursor)
            self.cursor.set_col(self.columns[self.cursor_col + 1])
            self.cursor_col += 1
            self.follow_cursor_horizontal()
            self.update()

    def _cursor_left(self, ev):
        self.cursor.key_press(ev)
        if not ev.isAccepted():
            ev.accept()
            if self.cursor_col > 0:
                if (self.cursor.get_pos() not in
                        self.columns[self.cursor_col - 1].get_triggers()):
                    self.cursor.set_index(0)
                self.columns[self.cursor_col].set_cursor()
                self.columns[self.cursor_col - 1].set_cursor(self.cursor)
                self.cursor.set_col(self.columns[self.cursor_col - 1])
                self.cursor_col -= 1
                self.follow_cursor_horizontal()
                self.update()
            else:
                assert self.cursor_col == 0
                self.cursor.set_index(0)
        else:
            self.update()

    def _cursor_right(self, ev):
        self.cursor.key_press(ev)
        if not ev.isAccepted():
            ev.accept()
            if self.cursor_col < lim.COLUMNS_MAX - 1:
                self.columns[self.cursor_col].set_cursor()
                self.columns[self.cursor_col + 1].set_cursor(self.cursor)
                self.cursor.set_col(self.columns[self.cursor_col + 1])
                self.cursor_col += 1
                self.follow_cursor_horizontal()
                self.update()
            else:
                assert self.cursor_col == lim.COLUMNS_MAX - 1
                self.cursor.set_index(sys.maxsize)
        else:
            self.update()

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self.colours['bg'])
        paint.eraseRect(ev.rect())
        self._grid.paint(ev.rect(), paint)
        self.ruler.paint(ev.rect(), paint, self._play_row
                         if self._play_pattern == self.number else None)
        col_pos = self.ruler.width()
        for column in self.view_columns:
            column.paint(ev, paint, col_pos, self.hasFocus())
            col_pos += column.width()
#        self.edit.update()
        paint.end()

    def resizeEvent(self, ev):
        self.width = ev.size().width()
        self.height = ev.size().height()
        self.view_columns = list(self.get_viewable_columns(self.width))
        self.ruler.resize(ev)
        self._grid.height = ev.size().height()
        self._grid.width = sum(x.width() for x in self.view_columns)
        for column in self.columns:
            column.resize(self.height)

    def get_viewable_columns(self, total_width, start=None, direction=1):
        if start == None:
            start = self.first_column
        assert direction in (1, -1)
        used = self.ruler.width()
        for (width, column) in ((c.width(), c) for c in \
                           self.columns[start::direction]):
            used += width
            if used > total_width:
                break
            yield column

    def zoom(self, factor):
        pix_view_start = self.view_start * self.beat_len
        cur_view_pos = self.cursor.get_pix_pos() - pix_view_start
        self.beat_len = min(max(1, self.beat_len * factor),
                            lim.TIMESTAMP_BEAT * 3)
        self.cursor.set_beat_len(self.beat_len)
        self.view_start = ts.Timestamp((self.cursor.get_pix_pos() -
                                        cur_view_pos) / self.beat_len)
        if self.view_start < 0:
            self.view_start = ts.Timestamp()
        pix_view_start = self.view_start * self.beat_len
        pix_pat_len = self.length * self.beat_len
        col_head_height = QtGui.QFontMetrics(
                              self.fonts['column_head']).height()
        area_height = self.height - col_head_height
        trigger_height = QtGui.QFontMetrics(self.fonts['trigger']).height()
        if pix_pat_len - pix_view_start < area_height - trigger_height:
            self.follow_cursor_vertical()
        self.ruler.set_beat_len(self.beat_len)
        self.ruler.set_view_start(self.view_start)
        self._grid.beat_len = self.beat_len
        self._grid.view_start = self.view_start
        for col in self.columns:
            col.set_beat_len(self.beat_len)
            col.set_view_start(self.view_start)
        self.update()

    def _update_play_pattern(self, ch, event):
        self._play_pattern = event[1]
        if self._play_pattern != self.number:
            self.update(0, 0, self.ruler.width(), self.height)

    def _update_play_row(self, ch, event):
        self._play_row = ts.Timestamp(event[1])
        if self._play_pattern == self.number:
            self.update(0, 0, self.ruler.width(), self.height)


class Grid(object):

    def __init__(self, theme, ruler_width):
        self._colours = theme[0]
        self._fonts = theme[1]
        self._height = 0
        self._width = 0
        self._view_start = ts.Timestamp()
        self._beat_div_base = 2
        self.enabled = True
        self.snap = True
        self.set_dimensions(ruler_width)

    def set_dimensions(self, ruler_width):
        self._grid_min_dist = QtGui.QFontMetrics(self._fonts['trigger']).height()
        self._col_head_height = QtGui.QFontMetrics(
                                    self._fonts['column_head']).height()
        self._ruler_width = ruler_width

    @property
    def interval(self):
        line_min_time = self._grid_min_dist / self._beat_len
        return self._beat_div_base**math.ceil(
                        math.log(line_min_time, self._beat_div_base))

    @property
    def height(self):
        return self._height

    @height.setter
    def height(self, value):
        self._height = value

    @property
    def length(self):
        return self._length

    @length.setter
    def length(self, value):
        self._length = value

    @property
    def beat_len(self):
        return self._beat_len

    @beat_len.setter
    def beat_len(self, value):
        self._beat_len = float(value)

    @property
    def view_start(self):
        return self._view_start

    @view_start.setter
    def view_start(self, value):
        self._view_start = value

    @property
    def width(self):
        return self._width

    @width.setter
    def width(self, value):
        self._width = value

    def prev_pos(self, pos):
        return self._adjacent(pos, -1)

    def next_pos(self, pos):
        return self._adjacent(pos, 1)

    def _adjacent(self, pos, direction):
        assert direction in (-1, 1)
        round_func = math.floor if direction > 0 else math.ceil
        beats, rem = pos
        interval = self.interval
        if interval >= 1:
            return ts.Timestamp(round_func(beats / interval + direction) *
                                interval, 0)
        rem_interval = interval * ts.TIMESTAMP_BEAT
        new_index = round_func(rem / rem_interval + direction)
        new_rem = round(new_index * rem_interval)
        if new_rem == rem:
            new_rem = round((new_index + direction) * rem_interval)
        assert new_rem < rem if direction < 0 else new_rem > rem
        return ts.Timestamp(beats, new_rem)

    def paint(self, rect, paint):
        if not self.enabled:
            return
        paint.setPen(self._colours['grid'])
        for line_pos in self._get_viewable_positions(self.interval):
            self._paint_line(line_pos, paint)

    def _paint_line(self, pos, paint):
        view_start = float(self._view_start)
        view_end = view_start + self._height / self._beat_len
        y = self._col_head_height + (pos - view_start) * self._beat_len
        if not self._col_head_height <= y < self._height:
            return
        #if pos == 0 or pos == float(self._length):
        #    return
        paint.drawLine(self._ruler_width, y,
                       self._ruler_width + self._width, y)

    def _get_viewable_positions(self, interval):
        view_end = float(self._view_start) + self._height / self._beat_len
        #print(view_end, float(self._length))
        view_end = min(view_end, float(self._length))
        error = interval / 2
        pos = math.ceil(float(self._view_start) / interval) * interval
        while pos <= view_end:
            if abs(pos - round(pos)) < error:
                pos = round(pos)
            yield pos
            pos += interval


class Ruler(object):

    def __init__(self, theme, cursor):
        self.colours = theme[0]
        self.fonts = theme[1]
        self._cursor = cursor
        self.height = 0
        self.view_start = ts.Timestamp()
        self.beat_div_base = 2
        self.set_dimensions()

    def get_viewable_positions(self, interval):
        view_end = float(self.view_start) + (self.ruler_height +
                                             self.num_height) / self.beat_len
        view_end = min(view_end, float(self.length))
        error = interval / 2
        pos = math.ceil(float(self.view_start) / interval) * interval
        while pos < view_end:
            if abs(pos - round(pos)) < error:
                pos = round(pos)
            yield pos
            pos += interval

    def paint(self, rect, paint, play_row):
        ruler_area = QtCore.QRect(0, 0, self._width, self.height)
        real_area = rect.intersect(ruler_area)
        if real_area.isEmpty() or self.ruler_height <= 0:
            return

        view_start = float(self.view_start)
        view_len = self.ruler_height / self.beat_len
        view_end = view_start + view_len

        # paint background
        bg_start = self.col_head_height
        bg_end = min(self.height, (float(self.length) - view_start) *
                                   self.beat_len + self.col_head_height)
        paint.setBrush(self.colours['ruler_bg'])
        paint.setPen(QtCore.Qt.NoPen)
        paint.drawRect(0, bg_start, self._width, bg_end - bg_start)

        # paint cursor position marker
        cursor_pos = float(self._cursor.get_pos())
        if view_start <= cursor_pos <= view_end:
            paint.setPen(self.colours['ruler_cur'])
            pix_pos = (self._cursor.get_pix_pos() + self.col_head_height -
                       self.view_start * self.beat_len)
            paint.drawLine(0, pix_pos, self._width - 2, pix_pos)

        # paint start and end borders if visible
        paint.setPen(self.colours['ruler_fg'])
        if self.view_start == 0:
            paint.drawLine(0, self.col_head_height,
                           self._width - 2, self.col_head_height)
        if bg_end < self.height:
            paint.drawLine(0, bg_end, self._width - 2, bg_end)

        # resolve intervals
        line_min_time = self.line_min_dist / self.beat_len
        line_interval = self.beat_div_base**math.ceil(
                                math.log(line_min_time, self.beat_div_base))
        num_min_time = self.num_min_dist / self.beat_len
        num_interval = self.beat_div_base**math.ceil(
                               math.log(num_min_time, self.beat_div_base))

        # paint ruler lines
        for line_pos in set(self.get_viewable_positions(
                line_interval)).difference(
                        self.get_viewable_positions(num_interval)):
            self.paint_line(line_pos, paint)

        # paint ruler beat numbers
        paint.setFont(self.fonts['ruler'])
        for num_pos in self.get_viewable_positions(num_interval):
            self.paint_number(num_pos, paint)

        # paint play cursor
        if view_start <= play_row <= view_end:
            paint.setPen(self.colours['ruler_play_cur'])
            pix_pos = (self.beat_len * float(play_row - self.view_start) +
                       self.col_head_height)
            paint.drawLine(0, pix_pos, self._width - 2, pix_pos)

        # paint right border
        paint.setPen(self.colours['column_border'])
        paint.drawLine(self._width - 1, 0,
                       self._width - 1, self.height - 1)

    def paint_line(self, pos, paint):
        view_start = float(self.view_start)
        view_len = self.ruler_height / self.beat_len
        view_end = view_start + view_len
        y = self.col_head_height + (pos - view_start) * self.beat_len
        if y < self.col_head_height or y >= self.height:
            return
        if pos == 0 or pos == float(self.length):
            return
        x = self._width - 3
        paint.drawLine(x, y, self._width - 1, y)

    def paint_number(self, pos, paint):
        view_start = float(self.view_start)
        view_len = self.ruler_height / self.beat_len
        view_end = view_start + view_len
        y = self.col_head_height + (pos - view_start) * self.beat_len
        if y < self.col_head_height:
            return
        if pos == 0 or pos == float(self.length):
            return
        x = self._width - 6
        if y < self.height:
            paint.drawLine(x, y, self._width - 1, y)
        y -= self.num_height // 2
        if y >= self.height:
            return
        rect = QtCore.QRectF(0, y, self._width - 8, self.num_height)
        text = str(round(pos, 3))
        text = str(int(pos)) if pos == int(pos) else str(round(pos, 3))
        paint.drawText(rect, text, QtGui.QTextOption(QtCore.Qt.AlignRight))

    def resize(self, ev):
        self.height = ev.size().height()
        self.set_dimensions()

    def set_beat_len(self, length):
        self.beat_len = float(length)

    def set_dimensions(self):
        space = QtGui.QFontMetrics(
                    self.fonts['ruler']).boundingRect('00.000')
        self._width = space.width() + 8
        self.num_height = space.height()
        self.num_min_dist = space.height() * 2.0
        self.line_min_dist = 4.0
        self.col_head_height = QtGui.QFontMetrics(
                                   self.fonts['column_head']).height()
        self.ruler_height = self.height - self.col_head_height

    def set_length(self, length):
        self.length = length

    def set_view_start(self, start):
        self.view_start = start

    def width(self):
        return self._width


