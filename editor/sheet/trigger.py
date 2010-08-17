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

from PyQt4 import QtGui, QtCore

import timestamp as ts


class Trigger(list):

    def __init__(self, data, theme):
        list.__init__(self, data)
        self.colours = theme[0]
        self.fonts = theme[1]
        self.metrics = QtGui.QFontMetrics(self.fonts['trigger'])
        m_width = self.metrics.width('m')
        self.margin = m_width * 0.3
        self.padding = m_width * 0.15

    def cursor_area(self, index):
        start = self.margin
        hw = self.metrics.width(self[0])
        if index == 0:
            return start, hw
        start += hw
        index -= 1
        for field in self[1]:
            fw = self.field_width(field)
            if index == 0:
                return start + self.padding, fw - self.padding
            start += fw
            index -= 1
        assert round(start + self.margin - self.width()) == 0
        return start + self.margin, 0

    def paint(self, paint, rect, offset=0, cursor_pos=-1):
        paint.setPen(self.colours['trigger_fg'])
        opt = QtGui.QTextOption()
        opt.setWrapMode(QtGui.QTextOption.NoWrap)
        opt.setAlignment(QtCore.Qt.AlignRight)

        head_rect = QtCore.QRectF(rect)
        head_rect.moveLeft(head_rect.left() + offset)
        type_width = self.metrics.width(self[0]) + self.margin
        head_rect.setWidth(type_width)
        head_rect = head_rect.intersect(rect)
        if cursor_pos == 0:
            paint.setBackground(self.colours['trigger_fg'])
            paint.setBackgroundMode(QtCore.Qt.OpaqueMode)
            paint.setPen(self.colours['bg'])
        if offset < 0:
            if offset > -type_width:
                paint.drawText(head_rect, self[0], opt)
        else:
            paint.drawText(head_rect, self[0], opt)
        if cursor_pos == 0:
            paint.setBackground(self.colours['bg'])
            paint.setBackgroundMode(QtCore.Qt.TransparentMode)
            paint.setPen(self.colours['trigger_fg'])
        cursor_pos -= 1
        offset += type_width

        for field in self[1]:
            field_rect = QtCore.QRectF(rect)
            field_rect.moveLeft(rect.left() + offset + self.padding)
            field_width = self.field_width(field)
            field_rect.setWidth(field_width)
            field_rect = field_rect.intersect(rect)
            if field_rect.isValid():
                if field_rect.width() < field_width and \
                        field_rect.right() == rect.right():
                    opt.setAlignment(QtCore.Qt.AlignLeft)
                self.paint_field(paint, field, field_rect, opt,
                                 cursor_pos == 0)
                offset += field_rect.width()
            cursor_pos -= 1

        offset += self.margin
        if offset > 0:
            left = max(head_rect.left(), rect.left())
            right = min(rect.left() + offset, rect.right()) - 1
            paint.drawLine(left, rect.top(),
                           right, rect.top())
        return offset

    def paint_field(self, paint, field, rect, opt, cursor):
        s = None
        if isinstance(field, int):
            s = str(field)
        elif isinstance(field, float):
            s = '{0:.2}'.format(field)
        elif isinstance(field, ts.Timestamp):
            s = '{0:.2f}'.format(field)
        if cursor:
            paint.setBackground(self.colours['trigger_fg'])
            paint.setBackgroundMode(QtCore.Qt.OpaqueMode)
            paint.setPen(self.colours['bg'])
        paint.drawText(rect, s, opt)
        if cursor:
            paint.setBackground(self.colours['bg'])
            paint.setBackgroundMode(QtCore.Qt.TransparentMode)
            paint.setPen(self.colours['trigger_fg'])

    def field_width(self, field):
        w = self.padding
        if isinstance(field, int):
            return w + self.metrics.width(str(field))
        elif isinstance(field, float):
            return w + self.metrics.width('{0:.2}'.format(field))
        elif isinstance(field, ts.Timestamp):
            return w + self.metrics.width('{0:.2f}'.format(float(field)))

    def width(self):
        w = self.metrics.width(self[0])
        for field in self[1]:
            w += self.field_width(field)
        return w + 2 * self.margin


class note(float):
    pass


type_desc = {
        'Wpd': [ts.Timestamp],
        'W.jc': [int],
        'W.jr': [ts.Timestamp],
        'W.js': [int],
        'W.jss': [int],
        'Wj': [],

        'W.s': [int],
        'W.so': [float],
        'Wms': [int],
        'Wssi': [note, note],

        'W.t': [float],
        'W/t': [float],
        'W/=t': [ts.Timestamp],
        'W.v': [float],
        'W/v': [float],
        'W/=v': [ts.Timestamp],

        'C.i': [int],
        'C.g': [int],
        'C.d': [int],
        'C.dc': [int],

        'Cn+': [float],
        'Cn-': [],

        'C.f': [float],
        'C/f': [float],
        'C/=f': [ts.Timestamp],
        'CTs': [float],
        'CTsd': [ts.Timestamp],
        'CTd': [float],
        'CTdd': [ts.Timestamp],

        'C/p': [float],
        'C/=p': [ts.Timestamp],
        'CVs': [float],
        'CVsd': [ts.Timestamp],
        'CVd': [float],
        'CVdd': [ts.Timestamp],
        'CArp': [float, float, float, float],

        'C.l': [float],
        'C/l': [float],
        'C/=l': [ts.Timestamp],
        'CAs': [float],
        'CAsd': [ts.Timestamp],
        'CAd': [float],
        'CAdd': [ts.Timestamp],

        'C.r': [float],

        'C.P': [float],
        'C/P': [float],
        'C/=P': [ts.Timestamp],

        'C.gB': [bool],
        'C.gI': [int],
        'C.gF': [float],
        'C.gT': [ts.Timestamp],

        'I.ped': [float],

        'G.B': [bool],
        'G.I': [int],
        'G.F': [float],
        'G.T': [ts.Timestamp],

        'D.B': [bool],
        'D.I': [int],
        'D.F': [float],
        'D.T': [ts.Timestamp],
}


