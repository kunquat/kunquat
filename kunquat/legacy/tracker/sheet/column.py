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
from collections import defaultdict
from itertools import izip_longest
import json
import re

import kunquat
from PyQt4 import QtGui, QtCore

import kunquat.tracker.kqt_limits
import kunquat.tracker.timestamp as ts
import trigger
import trigger_row as tr


column_path = re.compile('(pat_[0-9a-f]{3})/(col_[0-9a-f]{2})/')


class Column(object):

    """
    def __init__(self, handle, path):
        assert handle
        self.handle = handle
        self.set_path(path)

    def set_path(self, path):
        assert column_path.match(path)
        self.path = path
    """

    def __init__(self, num, theme):
        assert num >= 0
        self.num = num
        self.colours = theme[0]
        self.fonts = theme[1]
        self.arrange_triggers(None)
        self._width = 128
        self.height = 0
        self.set_dimensions()
        self.set_cursor()

    def arrange_triggers(self, triggers):
        if not triggers:
            self.triggers = {}
            return
        theme = self.colours, self.fonts
        d = defaultdict(lambda: tr.Trigger_row(theme))
        for (t, event) in triggers:
            d[ts.Timestamp(t)].append(trigger.Trigger(event, theme))
        self.triggers = dict(d)

    def flatten(self):
        if not self.triggers:
            return None
        l = []
        for row, trigs in self.triggers.iteritems():
            for trig in trigs:
                l.append([list(row), trig.flatten()])
        return l

    def get_num(self):
        return self.num

    def get_triggers(self):
        return self.triggers

    def width(self):
        return self._width + 1

    def set_width(self, width):
        if width < 50:
            width = 50
        self._width = width - 1

    def resize(self, height):
        self.height = height
        self.set_dimensions()

    def set_cursor(self, cursor=None):
        self.cursor = cursor

    def set_dimensions(self):
        self.col_head_height = QtGui.QFontMetrics(
                                   self.fonts['column_head']).height()
        self.trigger_height = QtGui.QFontMetrics(
                                  self.fonts['trigger']).height()
        self.edit_area_height = self.height - self.col_head_height

    def set_triggers(self, triggers):
        self.triggers = self.arrange_triggers(triggers)

    def set_value(self, cursor, value):
        assert self.cursor == cursor
        assert cursor
        if cursor.get_pos() in self.triggers:
            self.triggers[cursor.get_pos()].set_value(cursor, value)
        else:
            assert isinstance(value, trigger.TriggerType)
            theme = self.colours, self.fonts
            row = tr.Trigger_row(theme)
            row.append(trigger.Trigger([value, None], theme))
            self.triggers[cursor.get_pos()] = row

    def set_view_start(self, start):
        self.view_start = start

    def paint(self, ev, paint, x, focus):
        col_area = QtCore.QRect(x, 0, self._width, self.height)
        real_area = ev.rect().intersect(col_area)
        if real_area.isEmpty() or (self.edit_area_height <=
                                   self.trigger_height):
            return
#        paint.drawRect(real_area)

        paint.setBackgroundMode(QtCore.Qt.OpaqueMode)
        col_head_height = QtGui.QFontMetrics(
                              self.fonts['column_head']).height()
        view_start = self.view_start - col_head_height / self.beat_len
        view_end = self.edit_area_height / self.beat_len + self.view_start
        paint.setFont(self.fonts['trigger'])
        trigger_height = QtGui.QFontMetrics(self.fonts['trigger']).height() - 1
        visible_triggers = [p for p in self.triggers
                            if view_start < p <= min(view_end, self.length)]
        visible_triggers.sort(lambda x, y: (y - x).signum())
        next_pos = None
        edit_cursor_drawn = False
        if visible_triggers:
            for pos in visible_triggers:
                pix_pos = float((pos - self.view_start) * self.beat_len +
                                col_head_height)
                next_pix_pos = pix_pos + trigger_height
                if next_pos > 0 and (not self.cursor or
                                 pos != self.cursor.get_pos()):
                    next_pix_pos = float((next_pos - self.view_start) *
                                         self.beat_len + col_head_height) - 1
                row_height = next_pix_pos - pix_pos
                rect = QtCore.QRectF(x, pix_pos, self._width, row_height)
                draw_edit_cursor = self.cursor and pos == self.cursor.get_pos()
                edit_cursor_drawn = edit_cursor_drawn or draw_edit_cursor
                self.triggers[pos].paint(paint, rect, self.cursor
                             if draw_edit_cursor else None, focus)
                next_pos = pos
        if self.cursor and not edit_cursor_drawn:
            pix_pos = float((self.cursor.get_pos() - self.view_start) *
                            self.beat_len + col_head_height)
            rect = QtCore.QRectF(x, pix_pos, self._width, trigger_height)
            tr.Trigger_row((self.colours, self.fonts)).paint(paint, rect,
                                                       self.cursor, focus)

        if focus and self.cursor:
            pix_cur_pos = (self.cursor.get_pix_pos() -
                           self.view_start * self.beat_len)
            metrics = QtGui.QFontMetrics(self.fonts['trigger'])
            edit_width = metrics.width('  ') * 4
            self.cursor.set_geometry(x, pix_cur_pos + self.col_head_height,
                                     edit_width, metrics.height())
            if 0 <= pix_cur_pos < self.edit_area_height:
                pix_cur_pos += self.col_head_height
                paint.setPen(self.colours['cursor_line'])
                paint.drawLine(x, pix_cur_pos, x + self._width - 1, pix_cur_pos)

        paint.setBackgroundMode(QtCore.Qt.TransparentMode)
        paint.setBrush(self.colours['column_head_bg'])
        paint.setPen(QtCore.Qt.NoPen)
        paint.drawRect(x, 0, self._width, self.col_head_height)

        header_style = QtGui.QTextOption(QtCore.Qt.AlignHCenter)
        paint.setPen(self.colours['column_head_fg'])
        paint.setFont(self.fonts['column_head'])
        paint.drawText(QtCore.QRectF(x, 0, self._width, self.col_head_height),
                       '{0:02d}'.format(self.num), header_style)

        paint.setPen(self.colours['column_border'])
        paint.drawLine(x + self._width, 0, x + self._width, self.height - 1)

    def set_beat_len(self, length):
        self.beat_len = float(length)

    def set_length(self, length):
        self.length = length

    def set_view_start(self, start):
        self.view_start = start
        self.pix_view_start = start * self.beat_len

    def shift(self, pos, shift):
        shifted = {}
        for key in self.triggers:
            if key >= pos:
                shifted[key] = self.triggers[key]
        for key in shifted:
            del self.triggers[key]
        for key in shifted:
            if key + shift >= pos:
                self.triggers[key + shift] = shifted[key]


