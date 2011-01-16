# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
from itertools import izip_longest, takewhile
import math

from PyQt4 import QtGui, QtCore

import kunquat.editor.kqt_limits as lim
import scale
import kunquat.editor.timestamp as ts


note_off_str = u'══'


class Trigger(list):

    def __init__(self, data, theme):
        list.__init__(self, data)
        self.set_type(data[0])
        self.colours = theme[0]
        self.fonts = theme[1]
        self.metrics = QtGui.QFontMetrics(self.fonts['trigger'])
        m_width = self.metrics.width('m')
        self.margin = m_width * 0.3
        self.padding = m_width * 0.15

    def set_type(self, ttype):
        self[0] = TriggerType(ttype)
        self.type_info = None
        if self[0].valid:
            if self[0] in channel_triggers:
                self.type_info = channel_triggers[self[0]]
            elif self[0] in global_triggers:
                self.type_info = global_triggers[self[0]]
            else:
                self.type_info = general_triggers[self[0]]
            lv = takewhile(lambda x: x[0],
                           izip_longest(self.type_info, self[1]))
            self[1] = []
            for limits, value in lv:
                cons, valid, default = limits
                try:
                    if valid(value):
                        self[1].append(cons(value))
                    else:
                        self[1].append(default)
                except TypeError:
                    self[1].append(default)
        else:
            self[1] = list(fields)

    def set_value(self, cursor_pos, value):
        if self[0] != 'cn+':
            if cursor_pos == 0:
                self.set_type(value)
                return
            cursor_pos -= 1
        cons, valid, default = self.type_info[cursor_pos]
        self[1][cursor_pos] = cons(value)

    def cursor_area(self, index):
        start = self.margin
        if self[0] != 'cn+':
            hw = self.metrics.width(self[0])
            if index == 0:
                return start, hw
            start += hw
            index -= 1
        else:
            start -= self.padding
        for field in self[1]:
            fw = self.field_width(field)
            if index == 0:
                return start + self.padding, fw - self.padding
            start += fw
            index -= 1
        assert round(start + self.margin - self.width()) == 0
        return start + self.margin, 0

    def get_field_info(self, cursor_pos):
        if self[0] != 'cn+':
            if cursor_pos == 0:
                return self[0], None
            cursor_pos -= 1
        if cursor_pos >= 0 and cursor_pos < len(self[1]):
            return self[1][cursor_pos], self.type_info[cursor_pos][1]
        return None

    def paint(self, paint, rect, offset=0, cursor_pos=-1):
        init_offset = offset
        paint.setPen(self.colours['trigger_fg'])
        opt = QtGui.QTextOption()
        opt.setWrapMode(QtGui.QTextOption.NoWrap)
        opt.setAlignment(QtCore.Qt.AlignRight)

        offset += self.margin
        if self[0] != 'cn+':
            head_rect = QtCore.QRectF(rect)
            head_rect.moveLeft(head_rect.left() + offset)
            if self[0] == 'cn-':
                type_width = self.metrics.width(note_off_str)
            else:
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
        else:
            offset -= self.padding

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
            if self[0] == 'cn+':
                paint.setPen(self.colours['trigger_note_on_fg'])
            elif self[0] == 'cn-':
                paint.setPen(self.colours['trigger_note_off_fg'])
            else:
                paint.setPen(self.colours['trigger_fg'])
            left = max(rect.left() + init_offset, rect.left())
            right = min(rect.left() + offset, rect.right()) - 1
            if left < right:
                paint.drawLine(left, rect.top(),
                               right, rect.top())
#        if round((offset - init_offset) - self.width()) != 0:
#            print('offset change and width differ in', self, end=' -- ')
#            print(offset - init_offset, '!=', self.width(), end=', ')
#            print('diff:', (offset - init_offset) - self.width())
        return offset

    def paint_field(self, paint, field, rect, opt, cursor):
        s = self.field_str(field)
        back = self.colours['bg']
        if self[0] == 'cn+':
            fore = self.colours['trigger_note_on_fg']
        else:
            fore = self.colours['trigger_fg']
        if cursor:
            back, fore = fore, back
        paint.setBackground(back)
        paint.setPen(fore)
        paint.drawText(rect, s, opt)

    def field_str(self, field):
        if isinstance(field, Note):
            n, o, c = default_scale.get_display_info(field)
            c = int(round(c))
            if c == 0:
                return '{0}{1}'.format(n, o)
            return '{0}{1}{2:+d}'.format(n, o, c)
        if isinstance(field, int):
            return str(field)
        elif isinstance(field, float):
            return '{0:.1f}'.format(field)
        elif isinstance(field, ts.Timestamp):
            return '{0:.2f}'.format(float(field))
        return repr(field)

    def field_width(self, field):
        return self.padding + self.metrics.width(self.field_str(field))

    def slots(self):
        if self[0] == 'cn+':
            return len(self[1])
        return 1 + len(self[1])

    def width(self):
        fields_width = sum(self.field_width(f) for f in self[1])
        type_width = -self.padding
        if self[0] == 'cn-':
            type_width = self.metrics.width(note_off_str)
        elif self[0] != 'cn+':
            type_width = self.metrics.width(self[0])
        return type_width + fields_width + 2 * self.margin


class TriggerType(str):

    def __init__(self, name):
        self.valid = is_channel(name) or is_global(name) # FIXME

    def paint(self, colours, paint, rect, opt, cursor):
        if self == 'cn-':
            fore = colours['trigger_note_off_fg']
        elif self.valid:
            fore = colours['trigger_type_fg']
        else:
            fore = colours['trigger_invalid_fg']
        back = colours['bg']
        if cursor:
            fore, back = back, fore
        paint.setBackground(back)
        paint.setPen(fore)
        paint.drawText(rect, self if self != 'cn-' else note_off_str, opt)


default_scale = scale.Scale({
        'ref_pitch': 440 * 2**(3.0/12),
        'octave_ratio': ['/', [2, 1]],
        'notes': list(zip(('C', 'C#', 'D', 'D#', 'E', 'F',
                           'F#', 'G', 'G#', 'A', 'A#', 'B'),
                          (['c', cents] for cents in range(0, 1200, 100))))
    })


class Note(float):
    pass


def isfinite(x):
    return not (math.isinf(x) or math.isnan(x))


def is_global(ttype):
    ttype = str(ttype)
    return ttype in global_triggers or ttype in general_triggers


def is_channel(ttype):
    ttype = str(ttype)
    return ttype in channel_triggers or ttype in general_triggers


def is_key(value):
    if value is None:
        return False
    return all(x in '_./' or x.isalpha() for x in str(value))


nonneg_ts = (ts.Timestamp, lambda x: x >= 0, ts.Timestamp(0))
any_ts = (ts.Timestamp, lambda x: True, ts.Timestamp(0))
finite_float = (float, isfinite, 0.0)
nonneg_float = (float, lambda x: x >= 0 and isfinite(x), 0.0)
pos_float = (float, lambda x: x > 0 and isfinite(x), 1.0)
force = (float, lambda x: x <= 18 and not math.isnan(x), 0.0)
volume = (float, lambda x: x <= 0 and not math.isnan(x), 0.0)
any_float = (float, lambda x: True, 0.0)
any_bool = (bool, lambda x: True, False)
any_int = (int, lambda x: True, 0)
key = (str, is_key, '')
pitch = (Note, isfinite, Note(0))
note_entry = (int, lambda x: x >= 0, 0) # FIXME

global_triggers = {
        'wpd': [nonneg_ts],
        'w.jc': [(int, lambda x: x >= 0 and x < 65536, 0)],
        'w.jr': [nonneg_ts],
        'w.js': [(int, lambda x: x >= -1 and x < lim.SECTIONS_MAX, -1)],
        'w.jss': [(int, lambda x: x >= -1 and x < lim.SUBSONGS_MAX, -1)],
        'wj': [],

        'w.s': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, 0)],
        'w.so': [finite_float],
        'wms': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, 0)],
        'wssi': [note_entry, note_entry],

        'w.t': [(float, lambda x: x >= 1.0 and x <= 999.0, 120.0)],
        'w/t': [(float, lambda x: x >= 1.0 and x <= 999.0, 120.0)],
        'w/=t': [nonneg_ts],
        'w.v': [volume],
        'w/v': [volume],
        'w/=v': [nonneg_ts],
}

channel_triggers = {
        'c.i': [(int, lambda x: x >= 0 and x < lim.INSTRUMENTS_MAX, 0)],
        'c.g': [(int, lambda x: x >= 0 and x < lim.GENERATORS_MAX, 0)],
        'c.e': [(int, lambda x: x >= 0 and x < lim.EFFECTS_MAX, 0)],
        'c.ge': [],
        'c.ie': [],
        'c.d': [(int, lambda x: x >= 0 and x < lim.DSPS_MAX, 0)],

        'cn+': [pitch],
        'cn-': [],

        'c.f': [force],
        'c/f': [force],
        'c/=f': [nonneg_ts],
        'cTs': [nonneg_float],
        #'cTsd': [nonneg_ts],
        'cTd': [(float, lambda x: x >= 0.0 and x <= 24.0, 0.0)],
        'cTdd': [nonneg_ts],

        'c/p': [pitch],
        'c/=p': [nonneg_ts],
        'cVs': [nonneg_float],
        #'cVsd': [nonneg_ts],
        'cVd': [nonneg_float],
        'cVdd': [nonneg_ts],
        'cArp': [pos_float, finite_float, finite_float, finite_float],

        'c.l': [(float, isfinite, 0.0)],
        'c/l': [(float, isfinite, 0.0)],
        'c/=l': [nonneg_ts],
        'cAs': [nonneg_float],
        #'cAsd': [nonneg_ts],
        'cAd': [nonneg_float],
        'cAdd': [nonneg_ts],

        'c.r': [(float, lambda x: x >= 0 and x <= 99, 0.0)],
        #'c/r': [(float, lambda x: x >= 0 and x <= 99, 0.0)],
        #'c/=r': [nonneg_ts],

        'c.P': [(float, lambda x: x >= -1 and x <= 1, 0.0)],
        'c/P': [(float, lambda x: x >= -1 and x <= 1, 0.0)],
        'c/=P': [nonneg_ts],

        'c.gB': [key, any_bool],
        'c.gI': [key, any_int],
        'c.gF': [key, any_float],
        'c.gT': [key, any_ts],

        'i.ped': [(float, lambda x: x >= 0 and x <= 1, 0.0)],

        'g.B': [key, any_bool],
        'g.I': [key, any_int],
        'g.F': [key, any_float],
        'g.T': [key, any_ts],

        'e+': [],
        'e-': [],

        'd.B': [key, any_bool],
        'd.I': [key, any_int],
        'd.F': [key, any_float],
        'd.T': [key, any_ts],
}

general_triggers = {
}


