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
from itertools import izip_longest, takewhile
import math
import string

from PyQt4 import QtGui, QtCore

import kunquat.editor.kqt_limits as lim
import kunquat.editor.scale as scale
import kunquat.editor.timestamp as ts
import kunquat.editor.trigtypes as ttypes


note_off_str = u'══'

hidden_types = ('cn+', 'ch')


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
            self.type_info = ttypes.triggers[self[0]]
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
            if self[0] in ('c.gBn', 'g.Bn', 'd.Bn'):
                self[1][0] = key_to_param(self[1][0], 'p_', '.jsonb')
            elif self[0] in ('c.gIn', 'g.In', 'd.In'):
                self[1][0] = key_to_param(self[1][0], 'p_', '.jsoni')
            elif self[0] in ('c.gFn', 'g.Fn', 'd.Fn'):
                self[1][0] = key_to_param(self[1][0], 'p_', '.jsonf')
            elif self[0] in ('c.gTn', 'g.Tn', 'd.Tn'):
                self[1][0] = key_to_param(self[1][0], 'p_', '.jsont')
        else:
            self[1] = list(fields)

    def flatten(self):
        if self[0] in ('c.gBn', 'g.Bn', 'd.Bn'):
            return [self[0], [param_to_key(self[1][0], 'p_', '.jsonb')] +
                              self[1][1:]]
        elif self[0] in ('c.gIn', 'g.In', 'd.In'):
            return [self[0], [param_to_key(self[1][0], 'p_', '.jsoni')] +
                              self[1][1:]]
        elif self[0] in ('c.gFn', 'g.Fn', 'd.Fn'):
            return [self[0], [param_to_key(self[1][0], 'p_', '.jsonf')] +
                              self[1][1:]]
        elif self[0] in ('c.gTn', 'g.Tn', 'd.Tn'):
            return [self[0], [param_to_key(self[1][0], 'p_', '.jsont')] +
                              self[1][1:]]
        return self

    def set_value(self, cursor_pos, value):
        if self[0] not in hidden_types:
            if cursor_pos == 0:
                self.set_type(value)
                return
            cursor_pos -= 1
        cons, valid, default = self.type_info[cursor_pos]
        self[1][cursor_pos] = cons(value)

    def cursor_area(self, index):
        start = self.margin
        if self[0] not in hidden_types:
            hw = self.metrics.width(self[0])
            if index == 0:
                return start, hw
            start += hw
            index -= 1
        else:
            # compensate for removal of trigger name
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
        if self[0] not in hidden_types:
            # ignore the trigger name
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
        # paint the trigger type name
        if self[0] not in hidden_types:
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

        # paint the fields
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
            elif self[0] == 'ch':
                paint.setPen(self.colours['trigger_hit_fg'])
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
        elif self[0] == 'ch':
            fore = self.colours['trigger_hit_fg']
        else:
            fore = self.colours['trigger_fg']
        if cursor:
            back, fore = fore, back
        paint.setBackground(back)
        paint.setPen(fore)
        paint.drawText(rect, s, opt)

    def field_str(self, field):
        if isinstance(field, ttypes.Note):
            n, o, c = default_scale.get_display_info(field)
            c = int(round(c))
            if c == 0:
                return '{0}{1}'.format(n, o)
            return '{0}{1}{2:+d}'.format(n, o, c)
        if isinstance(field, ttypes.HitIndex):
            return str(field)
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
        if self[0] in hidden_types:
            return len(self[1])
        return 1 + len(self[1])

    def width(self):
        fields_width = sum(self.field_width(f) for f in self[1])
        type_width = -self.padding
        if self[0] == 'cn-':
            type_width = self.metrics.width(note_off_str)
        elif self[0] not in hidden_types:
            type_width = self.metrics.width(self[0])
        return type_width + fields_width + 2 * self.margin


class TriggerType(str):

    def __init__(self, name):
        self.valid = lambda x: str(x) in ttypes.triggers

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


def key_to_param(key, prefix, suffix):
    last_index = key.index('/') + 1 if '/' in key else 0
    last_part = key[last_index:]
    if last_part.startswith(prefix) and last_part.endswith(suffix):
        last_part = last_part[len(prefix):-len(suffix)]
        return key[:last_index] + last_part
    return key


def param_to_key(param, prefix, suffix):
    last_index = param.index('/') + 1 if '/' in param else 0
    last_part = param[last_index:]
    if not last_part.startswith(prefix):
        last_part = 'p_' + last_part
    if not last_part.endswith(suffix):
        last_part = last_part + suffix
    return param[:last_index] + last_part


