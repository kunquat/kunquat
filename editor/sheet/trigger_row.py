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

    def get_active_trigger(self):
        if self.gap or self.cursor_pos == len(self):
            return None
        return self[self.cursor_pos]

    def key_press(self, ev):
        pass

    def slots(self):
        s = 0
        for t in self:
            s += 1 + len(t[1])
        return s

    def paint(self, paint, rect, cursor=None):
        offset = 0
        cursor_pos = -1
        if cursor:
            cursor_pos = cursor.get_index()
            paint.eraseRect(rect)
        for t in self:
            offset = t.paint(paint, rect, offset, cursor_pos)
            cursor_pos -= 1 + len(t[1])
        if cursor_pos >= 0:
            metrics = QtGui.QFontMetrics(self.fonts['trigger'])
            width = metrics.width('n')
            height = metrics.height()
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


