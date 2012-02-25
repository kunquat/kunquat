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

from itertools import islice

from PyQt4 import QtGui, QtCore

import kunquat.editor.trigtypes as ttypes
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

    def get_slot(self, cursor):
        """Return the location of the cursor inside a Trigger row.

        Return value:
        A tuple with the index of the selected trigger as the first element,
        and the cursor position inside that trigger as the second element.

        """
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
        type_validator = lambda x: str(x) in ttypes.triggers
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
            index, _ = self.get_slot(cursor)
            theme = self.colours, self.fonts
            t = trigger.Trigger([value, None], theme)
            self.insert(index, t)
        else:
            trig, field = self.get_slot(cursor)
            self[trig].set_value(field, value)

    def key_press(self, ev):
        pass

    def cursor_area(self, cursor):
        cursor_pos = cursor.get_index()
        if cursor_pos == 0:
            return 0, 1
        trig, field = self.get_slot(cursor)
        left = sum(t.width() for t in islice(self, trig))
        if cursor.insert:
            return left, self.empty_cursor_width
        if trig < len(self):
            fstart, width = self[trig].cursor_area(field)
            return left + fstart, width
        assert round(left - self.width()) == 0
        return left, self.empty_cursor_width

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
                index, _ = self.get_slot(cursor)
                if index < len(self):
                    insert_pos = self[index]
                    cursor_pos = -1
            cursor.set_geometry(rect.left() + cursor_left + offset,
                                rect.top(), max(cursor_width,
                                    self.empty_cursor_width * 4),
                                self.metrics.height())
        if offset < 0:
            left_arrow = True
        for t in self:
            if t is insert_pos:
                height = self.metrics.height()
                cursor_rect = QtCore.QRectF(rect.left() + offset, rect.top() + 1,
                                            self.empty_cursor_width, height)
                if focus:
                    self.paint_empty_cursor(paint, cursor_rect)
                offset += self.empty_cursor_width
            offset = t.paint(paint, rect, offset, cursor_pos if focus else -1)
            cursor_pos -= t.slots()
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


