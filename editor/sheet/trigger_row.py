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
        self.arrow_size = self.metrics.height() * 0.25
        self.arrow_left = QtGui.QPolygon([
                            QtCore.QPoint(0, 0),
                            QtCore.QPoint(self.arrow_size, self.arrow_size),
                            QtCore.QPoint(self.arrow_size, 0)])
        self.arrow_right = QtGui.QPolygon([
                            QtCore.QPoint(-self.arrow_size, 0),
                            QtCore.QPoint(-self.arrow_size, self.arrow_size),
                            QtCore.QPoint(0, 0)])

    """
    def get_active_trigger(self):
        if self.gap or self.cursor_pos == len(self):
            return None
        return self[self.cursor_pos]
    """

    def get_slot(self, cursor):
        index = 0
        cursor_pos = cursor.get_index()
        for t in self:
            slots = t.slots()
            if cursor_pos < slots:
                if cursor.insert:
                    return index, 0
                return index, cursor_pos
            index += 1
            cursor_pos -= slots
        return index, 0

    def get_field_info(self, cursor):
        type_validator = (trigger.is_global if cursor.col.get_num() == -1
                          else trigger.is_channel)
        if cursor.insert:
            return trigger.TriggerType(''), type_validator
        index, pos = self.get_slot(cursor)
        if index < len(self):
            return self[index].get_field_info(pos)
        return trigger.TriggerType(''), type_validator

    def set_value(self, cursor, value):
        cursor_pos = cursor.get_index()
        if cursor.insert or cursor_pos >= self.slots():
            assert isinstance(value, trigger.TriggerType)
            index = 0
            for t in self:
                cursor_pos -= 1 + len(t[1])
                if cursor_pos < 0:
                    break
                index += 1
            theme = self.colours, self.fonts
            t = trigger.Trigger([value, []], theme)
            self.insert(index, t)
        else:
            for t in self:
                assert cursor_pos >= 0
                slots = 1 + len(t[1])
                if cursor_pos < slots:
                    t.set_value(cursor_pos, value)
                    break
                cursor_pos -= slots

    def key_press(self, ev):
        pass

    def cursor_area(self, cursor):
        cursor_pos = cursor.get_index()
        if cursor_pos == 0:
            return 0, 1
        if cursor.insert:
            w = 0
            for t in self:
                cursor_pos -= 1 + len(t[1])
                if cursor_pos < 0:
                    break
                w += t.width()
            return w, self.empty_cursor_width
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
        return sum(t.slots() for t in self)

    def paint(self, paint, rect, cursor=None, focus=False):
        offset = 0
        cursor_pos = -1
        left_arrow, right_arrow = False, False
        insert_pos = None
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
            if cursor.insert:
                cp = cursor_pos
                for t in self:
                    if cp < 1 + len(t[1]):
                        insert_pos = t
                        cursor_pos = -1
                        break
                    cp -= 1 + len(t[1])
            cursor.set_geometry(rect.left() + cursor_left + offset,
                                rect.top(), max(cursor_width,
                                    self.empty_cursor_width * 4),
                                self.metrics.height())
        if offset < 0:
            left_arrow = True
        for t in self:
            if t == insert_pos:
                height = self.metrics.height()
                cursor_rect = QtCore.QRectF(rect.left() + offset, rect.top() + 1,
                                            self.empty_cursor_width, height)
                if focus:
                    self.paint_empty_cursor(paint, cursor_rect)
                offset += self.empty_cursor_width
            offset = t.paint(paint, rect, offset, cursor_pos if focus else -1)
            cursor_pos -= 1 + len(t[1])
        if offset > rect.width():
            right_arrow = True
        if cursor_pos >= 0 and focus:
            height = self.metrics.height()
            cursor_rect = QtCore.QRectF(rect.left() + offset, rect.top(),
                                        self.empty_cursor_width, height)
            self.paint_empty_cursor(paint, cursor_rect)
        if left_arrow:
            self.paint_left_arrow(paint, rect)
        if right_arrow:
            self.paint_right_arrow(paint, rect)

    def paint_empty_cursor(self, paint, rect):
        paint.fillRect(rect, self.colours['trigger_type_fg'])

    def paint_left_arrow(self, paint, rect):
        arrow = self.arrow_left.translated(rect.left(), rect.top() + 1)
        paint.setPen(QtCore.Qt.NoPen)
        paint.setBrush(self.colours['cursor_arrow'])
        paint.drawConvexPolygon(arrow)

    def paint_right_arrow(self, paint, rect):
        arrow = self.arrow_right.translated(rect.right(), rect.top() + 1)
        paint.setPen(QtCore.Qt.NoPen)
        paint.setBrush(self.colours['cursor_arrow'])
        paint.drawConvexPolygon(arrow)

    def width(self):
        return sum(t.width() for t in self)


