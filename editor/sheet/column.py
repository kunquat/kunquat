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
from collections import defaultdict
from itertools import izip_longest
import json
import re

import kunquat
from PyQt4 import QtGui, QtCore

import kqt_limits
import timestamp as ts
import trigger
import trigger_row as tr


column_path = re.compile('(pat_[0-9a-f]{3})/(gcol|ccol_[0-9a-f]{2})/')


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

    def __init__(self, num, triggers, theme):
        assert num >= -1
        self.num = num
        self.colours = theme[0]
        self.fonts = theme[1]
        self.triggers = self.arrange_triggers(triggers)
        self._width = 128
        self.height = 0
        self.set_dimensions()
        self.set_cursor()

    def get_num(self):
        return self.num

    def get_triggers(self):
        return self.triggers

    def width(self):
        return self._width + 1

    def arrange_triggers(self, triggers):
        if not triggers:
            return {}
        theme = self.colours, self.fonts
        d = defaultdict(lambda: tr.Trigger_row(theme))
        for (t, event) in triggers:
            d[ts.Timestamp(t)].append(trigger.Trigger(event, theme))
        return dict(d)

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

    def set_view_start(self, start):
        self.view_start = start

    def paint(self, ev, paint, x, focus):
        focus = focus and self.cursor
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
                            if view_start < p < view_end]
        visible_triggers.sort(lambda x, y: (y - x)[0] * ts.TIMESTAMP_BEAT +
                                           (y - x)[1])
        next_pos = None
        if visible_triggers:
            for pos in visible_triggers:
                pix_pos = float((pos - self.view_start) * self.beat_len +
                                col_head_height)
                next_pix_pos = pix_pos + trigger_height
                if next_pos and (not focus or
                                 pos != self.cursor.get_pos()):
                    next_pix_pos = float((next_pos - self.view_start) *
                                         self.beat_len + col_head_height) - 1
                row_height = next_pix_pos - pix_pos
                rect = QtCore.QRectF(x, pix_pos, self._width, row_height)
                self.triggers[pos].paint(paint, rect, self.cursor
                             if focus and pos == self.cursor.get_pos()
                             else None)
                next_pos = pos

        if focus:
            pix_cur_pos = (self.cursor.get_pix_pos() -
                           self.view_start * self.beat_len)
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
                       '%02d' % self.num if self.num >= 0 else 'Global',
                       header_style)
        
        paint.setPen(self.colours['column_border'])
        paint.drawLine(x + self._width, 0, x + self._width, self.height - 1)

    def set_beat_len(self, length):
        self.beat_len = float(length)

    def set_length(self, length):
        self.length = length

    def set_view_start(self, start):
        self.view_start = start
        self.pix_view_start = start * self.beat_len


