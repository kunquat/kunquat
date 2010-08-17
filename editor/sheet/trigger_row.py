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

from PyQt4 import QtGui, QtCore

import trigger


class Trigger_row(list):

    def __init__(self, theme):
        list.__init__(self)
        self.colours = theme[0]
        self.fonts = theme[1]
        self.metrics = QtGui.QFontMetrics(self.fonts['trigger'])
        self.empty_cursor_width = self.metrics.width('  ')

    def get_active_trigger(self):
        if self.gap or self.cursor_pos == len(self):
            return None
        return self[self.cursor_pos]

    def key_press(self, ev):
        pass

    def cursor_area(self, cursor):
        cursor_pos = cursor.get_index()
        if cursor_pos == 0:
            return 0, 1
        start = 0
        for t in self:
            fstart, width = t.cursor_area(cursor_pos)
            if width > 0:
                return start + fstart, width
            cursor_pos -= 1 + len(t[1])
            start += fstart
        assert round(start - self.width()) == 0
        return start, self.empty_cursor_width

    def slots(self):
        return sum(1 + len(t[1]) for t in self)

    def paint(self, paint, rect, cursor=None):
        offset = 0
        cursor_pos = -1
        if cursor:
            cursor_pos = cursor.get_index()
            paint.eraseRect(rect)
            cursor_left, cursor_width = self.cursor_area(cursor)
            cursor_right = cursor_left + cursor_width
            offset = -cursor.get_view_start()
            if cursor_right + offset > rect.width():
                offset = rect.width() - cursor_right
            if cursor_left + offset < 0:
                offset = -cursor_left
            if offset > 0:
                offset = 0
            full_width = self.width()
            if cursor.get_index() >= self.slots():
                full_width += self.empty_cursor_width
            if full_width <= rect.width():
                offset = 0
            cursor.set_view_start(-offset)
        for t in self:
            offset = t.paint(paint, rect, offset, cursor_pos)
            cursor_pos -= 1 + len(t[1])
        if cursor_pos >= 0:
            width = self.metrics.width('n')
            height = self.metrics.height()
            cursor_rect = QtCore.QRectF(rect.left() + offset, rect.top(),
                                        width, height)
            paint.setBackground(self.colours['trigger_fg'])
            paint.setBackgroundMode(QtCore.Qt.OpaqueMode)
            paint.drawText(cursor_rect, '  ')
            paint.setBackground(self.colours['bg'])
            paint.setBackgroundMode(QtCore.Qt.TransparentMode)
#            paint.fillRect(QtCore.QRectF(rect.left() + offset, rect.top(),
#                                         width, height),
#                           self.colours['trigger_fg'])

    def width(self):
        return sum(t.width() for t in self)


