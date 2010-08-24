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

from PyQt4 import QtGui, QtCore

import kqt_limits as lim
import timestamp as ts


class Trigger(list):

    def __init__(self, data, theme):
        list.__init__(self, data)
        self[0] = TriggerType(self[0])
        if self[0].valid:
            param_limits = type_desc[self[0]]
            for i, v in enumerate([x for x in self[1]]):
                cons, valid, default = param_limits[i]
                self[1][i] = cons(v)
                if not valid(self[1][i]):
                    self[1][i] = default
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

    def get_field(self, cursor_pos):
        if cursor_pos == 0:
            return self[0]
        elif cursor_pos > 0 and cursor_pos <= len(self[1]):
            return self[1][cursor_pos - 1]
        return None

    def paint(self, paint, rect, offset=0, cursor_pos=-1):
        init_offset = offset
        paint.setPen(self.colours['trigger_fg'])
        opt = QtGui.QTextOption()
        opt.setWrapMode(QtGui.QTextOption.NoWrap)
        opt.setAlignment(QtCore.Qt.AlignRight)

        offset += self.margin
        head_rect = QtCore.QRectF(rect)
        head_rect.moveLeft(head_rect.left() + offset)
        type_width = self.metrics.width(self[0])
        head_rect.setWidth(type_width)
        head_rect = head_rect.intersect(rect)
        if offset < 0:
            if offset > -type_width:
                self[0].paint(self.colours, paint, head_rect,
                              opt, cursor_pos == 0)
        else:
            if head_rect.width() < type_width and \
                    head_rect.right() == rect.right():
                opt.setAlignment(QtCore.Qt.AlignLeft)
            self[0].paint(self.colours, paint, head_rect,
                          opt, cursor_pos == 0)
        cursor_pos -= 1
        offset += type_width

        paint.setBackground(self.colours['bg'])
        paint.setPen(self.colours['trigger_fg'])
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
            offset += field_width
            cursor_pos -= 1

        offset += self.margin
        if offset > 0:
            left = max(rect.left() + init_offset, rect.left())
            right = min(rect.left() + offset, rect.right()) - 1
            paint.drawLine(left, rect.top(),
                           right, rect.top())
#        if round((offset - init_offset) - self.width()) != 0:
#            print('offset change and width differ in', self, end=' -- ')
#            print(offset - init_offset, '!=', self.width())
        return offset

    def paint_field(self, paint, field, rect, opt, cursor):
        s = self.field_str(field)
        if cursor:
            paint.setBackground(self.colours['trigger_fg'])
#            paint.setBackgroundMode(QtCore.Qt.OpaqueMode)
            paint.setPen(self.colours['bg'])
        paint.drawText(rect, s, opt)
        if cursor:
            paint.setBackground(self.colours['bg'])
#            paint.setBackgroundMode(QtCore.Qt.TransparentMode)
            paint.setPen(self.colours['trigger_fg'])

    def field_str(self, field):
        if isinstance(field, int):
            return str(field)
        elif isinstance(field, float):
            return '{0:.1f}'.format(field)
        elif isinstance(field, ts.Timestamp):
            return '{0:.2f}'.format(float(field))

    def field_width(self, field):
        return self.padding + self.metrics.width(self.field_str(field))

    def width(self):
        return self.metrics.width(self[0]) + sum(
                self.field_width(f) for f in self[1]) + 2 * self.margin


class TriggerType(str):

    def __init__(self, name):
        self.valid = name in type_desc

    def paint(self, colours, paint, rect, opt, cursor):
        if self.valid:
            fore = colours['trigger_type_fg']
        else:
            fore = colours['trigger_invalid_fg']
        back = colours['bg']
        if cursor:
            fore, back = back, fore
        paint.setBackground(back)
        paint.setPen(fore)
        paint.drawText(rect, self, opt)


class note(float):
    pass


def isfinite(x):
    return not (math.isinf(x) or math.isnan(x))


nonneg_ts = (ts.Timestamp, lambda x: x >= 0, ts.Timestamp(0))
any_ts = (ts.Timestamp, lambda x: True, ts.Timestamp(0))
finite_float = (float, isfinite, 0.0)
nonneg_float = (float, lambda x: x >= 0 and isfinite(x), 0.0)
pos_float = (float, lambda x: x > 0 and isfinite(x), 0.0)
force = (float, lambda x: x <= 18 and not math.isnan(x), 0.0)
volume = (float, lambda x: x <= 0 and not math.isnan(x), 0.0)
any_float = (float, lambda x: True, 0.0)
any_bool = (bool, lambda x: True, False)
any_int = (int, lambda x: True, 0)

type_desc = {
        'Wpd': [nonneg_ts],
        'W.jc': [(int, lambda x: x >= 0 and x < 65536, 0)],
        'W.jr': [nonneg_ts],
        'W.js': [(int, lambda x: x >= -1 and x < lim.SECTIONS_MAX, -1)],
        'W.jss': [(int, lambda x: x >= -1 and x < lim.SUBSONGS_MAX, -1)],
        'Wj': [],

        'W.s': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, 0)],
        'W.so': [finite_float],
        'Wms': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, 0)],
        'Wssi': [note, note],

        'W.t': [(float, lambda x: x >= 1.0 and x <= 999.0, 120.0)],
        'W/t': [(float, lambda x: x >= 1.0 and x <= 999.0, 120.0)],
        'W/=t': [nonneg_ts],
        'W.v': [volume],
        'W/v': [volume],
        'W/=v': [nonneg_ts],

        'C.i': [(int, lambda x: x >= 0 and x < lim.INSTRUMENTS_MAX, 0)],
        'C.g': [(int, lambda x: x >= 0 and x < lim.GENERATORS_MAX, 0)],
        'C.d': [(int, lambda x: x >= 0 and x < lim.DSP_EFFECTS_MAX, 0)],
        'C.dc': [(int, lambda x: x >= -1 and x < lim.INSTRUMENTS_MAX, -1)],

        'Cn+': [finite_float],
        'Cn-': [],

        'C.f': [force],
        'C/f': [force],
        'C/=f': [nonneg_ts],
        'CTs': [nonneg_float],
        'CTsd': [nonneg_ts],
        'CTd': [(float, lambda x: x >= 0.0 and x <= 24.0, 0.0)],
        'CTdd': [nonneg_ts],

        'C/p': [finite_float],
        'C/=p': [nonneg_ts],
        'CVs': [nonneg_float],
        'CVsd': [nonneg_ts],
        'CVd': [nonneg_float],
        'CVdd': [nonneg_ts],
        'CArp': [pos_float, finite_float, finite_float, finite_float],

        'C.l': [(float, isfinite, 0.0)],
        'C/l': [(float, isfinite, 0.0)],
        'C/=l': [nonneg_ts],
        'CAs': [nonneg_float],
        'CAsd': [nonneg_ts],
        'CAd': [nonneg_float],
        'CAdd': [nonneg_ts],

        'C.r': [(float, lambda x: x >= 0 and x <= 99, 0.0)],

        'C.P': [(float, lambda x: x >= -1 and x <= 1, 0.0)],
        'C/P': [(float, lambda x: x >= -1 and x <= 1, 0.0)],
        'C/=P': [nonneg_ts],

        'C.gB': [any_bool],
        'C.gI': [any_int],
        'C.gF': [any_float],
        'C.gT': [any_ts],

        'I.ped': [(float, lambda x: x >= 0 and x <= 1, 0.0)],

        'G.B': [any_bool],
        'G.I': [any_int],
        'G.F': [any_float],
        'G.T': [any_ts],

        'D.B': [any_bool],
        'D.I': [any_int],
        'D.F': [any_float],
        'D.T': [any_ts],
}


