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

    def paint(self, paint, rect):
        # TODO: remove the line after implementing trigger printing
        #paint.setPen(self.colours['trigger_fg'])
        #paint.drawLine(rect.left(), rect.top(), rect.right(), rect.top())
        offset = 0
        for t in self:
            offset += t.paint(paint, rect, offset)


