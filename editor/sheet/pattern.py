# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
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
import kqt_limits as lim
import note_input as ni
import scale
import timestamp as ts
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

    def __init__(self, project, section, playback, octave_spin, parent=None):
        QtGui.QWidget.__init__(self, parent)
        section.connect(self.section_changed)
        self.number = 0
        self.section_manager = section
        self.playback_manager = playback
        self.setSizePolicy(QtGui.QSizePolicy.Ignored,
                           QtGui.QSizePolicy.Ignored)
        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.first_column = -1
        self.colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'column_border': QtGui.QColor(0xcc, 0xcc, 0xcc),
                'column_head_bg': QtGui.QColor(0x33, 0x77, 0x22),
                'column_head_fg': QtGui.QColor(0xff, 0xee, 0xee),
                'cursor_bg': QtGui.QColor(0xff, 0x66, 0x22, 0x77),
                'cursor_line': QtGui.QColor(0xff, 0xee, 0x88),
                'cursor_arrow': QtGui.QColor(0xff, 0x44, 0x22),
                'ruler_bg': QtGui.QColor(0x11, 0x22, 0x55),
                'ruler_fg': QtGui.QColor(0xaa, 0xcc, 0xff),
                'trigger_fg': QtGui.QColor(0xaa, 0xaa, 0xaa),
                'trigger_note_on_fg': QtGui.QColor(0xee, 0xcc, 0xaa),
                'trigger_note_off_fg': QtGui.QColor(0xaa, 0x88, 0x66),
                'trigger_type_fg': QtGui.QColor(0xcc, 0xcc, 0xaa),
                'trigger_invalid_fg': QtGui.QColor(0xff, 0x33, 0x11),
                }
        self.fonts = {
                'column_head': QtGui.QFont('Decorative', 10),
                'ruler': QtGui.QFont('Decorative', 8),
                'trigger': QtGui.QFont('Decorative', 10),
                }
        self.length = ts.Timestamp(16)
        self.beat_len = 96
        self.view_start = ts.Timestamp(0)
        self.ruler = Ruler((self.colours, self.fonts))
        self.ruler.set_length(self.length)
        self.ruler.set_beat_len(self.beat_len)
        self.ruler.set_view_start(self.view_start)
        self.columns = [Column(num, (self.colours, self.fonts))
                        for num in xrange(-1, lim.COLUMNS_MAX)]
        for col in self.columns:
            col.set_length(self.length)
            col.set_beat_len(self.beat_len)
            col.set_view_start(self.view_start)

        self.accessors = {
                trigger.TriggerType: acc.TypeEdit(self),
                trigger.Note: acc.NoteEdit(self),
                bool: acc.BoolEdit(self),
                float: acc.FloatEdit(self),
                int: acc.IntEdit(self),
                ts.Timestamp: acc.TimestampEdit(self),
                str: acc.StringEdit(self),
                }
        for a in self.accessors:
            self.accessors[a].hide()
            QtCore.QObject.connect(self.accessors[a],
                                   QtCore.SIGNAL('returnPressed()'),
                                   self.value_changed)

        self.cursor = Cursor(self.length,
                             self.beat_len,
                             self.accessors,
                             playback)
        self.set_project(project)
        self.cursor.set_scale(default_scale)
        self.note_input = default_input
        self.cursor.set_input(self.note_input)
        QtCore.QObject.connect(octave_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.octave_changed)
        self.columns[0].set_cursor(self.cursor)
        self.cursor.set_col(self.columns[0])
        self.cursor_col = -1
        self.view_columns = []
        self.width = 0
        self.height = 0
        self.cursor_center_area = 0.3
        self.zoom_factor = 1.2

    def autoinst_changed(self, value):
        if value:
            self.cursor.inst_auto = True
        else:
            self.cursor.inst_auto = False

    def length_changed(self, *flength):
        self.set_pattern(self.number)
        self.update()

    def section_changed(self, *args):
        subsong, section = args
        pattern = self.project.get_pattern(subsong, section)
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
        self.cursor.set_path(self.path)
        self.columns[0].arrange_triggers(self.project['/'.join(
                        (self.path, 'gcol', 'p_global_events.json'))])
        for col in self.columns[1:]:
            col_dir = 'ccol_{0:02x}'.format(col.get_num())
            path = '/'.join((self.path, col_dir, 'p_channel_events.json'))
            col.arrange_triggers(self.project[path])
        if self.number != num:
            QtCore.QObject.emit(self, QtCore.SIGNAL('patternChanged(int)'),
                                num)
        self.number = num

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
            for col in self.columns:
                col.set_view_start(self.view_start)
            return True
        return False

    def keyPressEvent(self, ev):
        if self.cursor.edit:
            if ev.key() in (QtCore.Qt.Key_Escape,):
                self.setFocus()
        if ev.modifiers() == QtCore.Qt.ControlModifier:
            if ev.key() == QtCore.Qt.Key_Up:
                self.zoom(self.zoom_factor)
            elif ev.key() == QtCore.Qt.Key_Down:
                self.zoom(1 / self.zoom_factor)
            elif ev.key() == QtCore.Qt.Key_Left:
                for col in self.columns:
                    col.set_width(col.width() / self.zoom_factor)
                self.view_columns = list(self.get_viewable_columns(self.width))
                self.follow_cursor_horizontal()
                self.update()
            elif ev.key() == QtCore.Qt.Key_Right:
                for col in self.columns:
                    col.set_width(col.width() * self.zoom_factor)
                self.view_columns = list(self.get_viewable_columns(self.width))
                self.follow_cursor_horizontal()
                self.update()
            else:
                ev.ignore()
            return
        elif ev.modifiers() == QtCore.Qt.ShiftModifier:
            if ev.key() in (QtCore.Qt.Key_Insert, QtCore.Qt.Key_Delete):
                self.cursor.set_index(0)
                shift_pos = self.cursor.get_pos()
                self.cursor.set_direction(1)
                self.cursor.step()
                if ev.key() == QtCore.Qt.Key_Insert:
                    self.cursor.clear_delay()
                shift = self.cursor.get_pos() - shift_pos
                self.columns[self.cursor_col + 1].shift(shift_pos,
                        -shift if ev.key() == QtCore.Qt.Key_Delete else shift)
                self.cursor.set_pos(shift_pos)
                self.project[self.cursor.col_path] = \
                        self.columns[self.cursor_col + 1].flatten()
                self.update()
            else:
                ev.ignore()
            return

        if ev.key() == QtCore.Qt.Key_Left:
            self.cursor.key_press(ev)
            if not ev.isAccepted():
                ev.accept()
                if self.cursor_col > -1:
                    if (self.cursor.get_pos() not in
                            self.columns[self.cursor_col].get_triggers()):
                        self.cursor.set_index(0)
                    self.columns[self.cursor_col + 1].set_cursor()
                    self.columns[self.cursor_col].set_cursor(self.cursor)
                    self.cursor.set_col(self.columns[self.cursor_col])
                    self.cursor_col -= 1
                    self.follow_cursor_horizontal()
                    self.update()
                else:
                    assert self.cursor_col == -1
                    self.cursor.set_index(0)
            else:
                self.update()
        elif ev.key() == QtCore.Qt.Key_Right:
            self.cursor.key_press(ev)
            if not ev.isAccepted():
                ev.accept()
                if self.cursor_col < lim.COLUMNS_MAX - 1:
                    self.columns[self.cursor_col + 1].set_cursor()
                    self.columns[self.cursor_col + 2].set_cursor(self.cursor)
                    self.cursor.set_col(self.columns[self.cursor_col + 2])
                    self.cursor_col += 1
                    self.follow_cursor_horizontal()
                    self.update()
                else:
                    assert self.cursor_col == lim.COLUMNS_MAX - 1
                    self.cursor.set_index(sys.maxsize)
            else:
                self.update()
        elif ev.key() == QtCore.Qt.Key_Plus:
            subsong = self.section_manager.subsong
            section = self.section_manager.section
            if section < lim.SECTIONS_MAX - 1:
                self.section_manager.set(subsong, section + 1)
        elif ev.key() == QtCore.Qt.Key_Minus:
            subsong = self.section_manager.subsong
            section = self.section_manager.section
            if section > 0:
                self.section_manager.set(subsong, section - 1)
        else:
            self.cursor.key_press(ev)
            if ev.isAccepted():
                self.follow_cursor_vertical()
                self.update()
#        else:
#            print('press:', ev.key())

    def keyReleaseEvent(self, ev):
        if ev.isAutoRepeat():
            return
        if ev.key() in (QtCore.Qt.Key_Insert, QtCore.Qt.Key_Delete):
            self.cursor.set_direction()
            return
        if ev.key() in (QtCore.Qt.Key_Up, QtCore.Qt.Key_Down):
            self.cursor.key_release(ev)
#        else:
#            print('release:', ev.key())

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self.colours['bg'])
        paint.eraseRect(ev.rect())
        self.ruler.paint(ev, paint)
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
        for column in self.columns:
            column.resize(self.height)

    def get_viewable_columns(self, total_width, start=None, direction=1):
        if start == None:
            start = self.first_column
        assert direction in (1, -1)
        used = self.ruler.width()
        for (width, column) in ((c.width(), c) for c in \
                           self.columns[start + 1::direction]):
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
        for col in self.columns:
            col.set_beat_len(self.beat_len)
            col.set_view_start(self.view_start)
        self.update()


class Ruler(object):

    def __init__(self, theme):
        self.colours = theme[0]
        self.fonts = theme[1]
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

    def paint(self, ev, paint):
        ruler_area = QtCore.QRect(0, 0, self._width, self.height)
        real_area = ev.rect().intersect(ruler_area)
        if real_area.isEmpty() or self.ruler_height <= 0:
            return

        view_start = float(self.view_start)
        view_len = self.ruler_height / self.beat_len
        view_end = view_start + view_len

        # paint background, including start and end borders if visible
        bg_start = self.col_head_height
        bg_end = min(self.height, (float(self.length) - view_start) *
                                   self.beat_len + self.col_head_height)
        paint.setBrush(self.colours['ruler_bg'])
        paint.setPen(QtCore.Qt.NoPen)
        paint.drawRect(0, bg_start, self._width, bg_end - bg_start)
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


