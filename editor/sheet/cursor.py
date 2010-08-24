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
import sys

from PyQt4 import QtCore

import accessors as acc
import kqt_limits as lim
import timestamp
import trigger


class Cursor(object):

    def __init__(self, length, beat_len, accessors):
        self.init_speed = 1
        self.max_speed = 12
        self.init_trigger_delay = 6
        self.cur_trigger_delay = self.init_trigger_delay
        self.trigger_delay_left = 0
        self.ts = timestamp.Timestamp()
        self.set_length(length)
        self.set_beat_len(beat_len)
        self.insert = False
        self.edit = False
        self.col = None
        self.set_index(0)
        self.set_view_start(0)
        self.set_accel(1.18)
        self.direction = 0
        self.triggers = []
        self.accessors = accessors
        self.active_accessor = None

    def key_press(self, ev):
        if ev.key() == QtCore.Qt.Key_Up:
            self.set_direction(-1)
            self.step()
        elif ev.key() == QtCore.Qt.Key_Down:
            self.set_direction(1)
            self.step()
        elif ev.key() == QtCore.Qt.Key_PageUp:
            self.set_direction()
            self.set_pos(self.ts - 4)
        elif ev.key() == QtCore.Qt.Key_PageDown:
            self.set_direction()
            self.set_pos(self.ts + 4)
        elif ev.key() == QtCore.Qt.Key_Home:
            self.set_direction()
            self.set_pos(self.ts * 0)
        elif ev.key() == QtCore.Qt.Key_End:
            self.set_direction()
            self.set_pos(self.length)
        elif ev.key() == QtCore.Qt.Key_Left:
            if self.ts in self.col.get_triggers():
                slots = self.col.get_triggers()[self.ts].slots()
                if self.index <= 0:
                    if self.insert:
                        self.index = 0
                    else:
                        self.index = sys.maxsize
                        ev.ignore()
                elif self.index >= slots:
                    self.index = slots - 1
                else:
                    self.index -= 1
            else:
                self.index = sys.maxsize
                ev.ignore()
        elif ev.key() == QtCore.Qt.Key_Right:
            if self.ts in self.col.get_triggers():
                if not self.insert:
                    slots = self.col.get_triggers()[self.ts].slots()
                    if self.index >= slots:
                        self.index = 0
                        ev.ignore()
                    else:
                        self.index += 1
            else:
                ev.ignore()
        elif ev.key() == QtCore.Qt.Key_Insert:
            self.insert = True
            return
        elif ev.key() == QtCore.Qt.Key_Escape:
            self.edit = False
            self.insert = False
            if self.active_accessor:
                self.active_accessor.hide()
                self.active_accessor = None
        elif ev.key() == QtCore.Qt.Key_Delete:
            if self.insert:
                self.insert = False
            else:
                pass # TODO
        elif ev.key() == QtCore.Qt.Key_Return:
            self.edit = True
            tr = self.col.get_triggers()
            if self.ts in tr:
                field = tr[self.ts].get_field(self)
            else:
                field = trigger.TriggerType('')
            self.active_accessor = self.accessors[type(field)]
            self.active_accessor.set_value(field)
            self.active_accessor.show()
            self.active_accessor.setFocus()
            return
        else:
            ev.ignore()
        self.insert = False

    def key_release(self, ev):
        if ev.key() in (QtCore.Qt.Key_Up, QtCore.Qt.Key_Down):
            self.set_direction()
        else:
            ev.ignore()

    def set_beat_len(self, beat_len):
        assert beat_len > 0
        self.beat_len = beat_len
        self.set_pos(self.ts)

    def set_geometry(self, x, y, w, h):
        self.geom = QtCore.QRect(x, y, w, h)
        for a in self.accessors:
            self.accessors[a].setGeometry(self.geom)

    def set_length(self, length):
        assert length >= 0
        self.length = length

    def set_triggers(self, triggers):
        self.triggers = triggers

    def set_col(self, col):
        assert col
        self.col = col

    def set_pos(self, ts):
        self.ts = min(max(timestamp.Timestamp(), ts), self.length)
        self.pix_pos = self.ts * self.beat_len

    def set_pix_pos(self, pix_pos):
        self.pix_pos = float(max(0, pix_pos))
        self.ts = timestamp.Timestamp(self.pix_pos / self.beat_len)
        if self.ts > self.length:
            self.set_pos(self.length)

    def get_pos(self):
        return self.ts

    def get_pix_pos(self):
        return self.pix_pos

    def set_index(self, index):
        assert index >= 0
        self.index = index

    def get_index(self):
        return self.index

    def set_view_start(self, start):
        assert start >= 0
        self.view_start = start

    def get_view_start(self):
        return self.view_start

    def set_accel(self, accel):
        assert accel >= 1
        self.accel = accel

    def set_direction(self, direction=0):
        assert direction in (-1, 0, 1)
        if self.direction == direction:
            return
        self.direction = direction
        self.cur_speed = self.init_speed * direction
        self.cur_trigger_delay = self.init_trigger_delay
        self.trigger_delay_left = 0

    def step(self):
        if not self.cur_speed:
            return
        if self.trigger_delay_left:
            self.trigger_delay_left -= 1
            return
        orig_pos = self.ts
        self.set_pix_pos(self.pix_pos + self.cur_speed)
        if self.ts == orig_pos:
            self.set_pos(self.ts + timestamp.Timestamp(0,
                                       1 if self.cur_speed > 0 else -1))

        first, second = ((orig_pos, self.ts) if orig_pos < self.ts
                                             else (self.ts, orig_pos))
        rows = [p for p in self.col.get_triggers() if first <= p <= second
                                                      and p != orig_pos]
        if rows:
            pos = min(rows) if orig_pos < self.ts else max(rows)
            self.set_pos(pos)
            self.trigger_delay_left = self.cur_trigger_delay
            if self.cur_trigger_delay:
                self.cur_trigger_delay -= 1

        if abs(self.cur_speed) == self.max_speed:
            return
        self.cur_speed *= self.accel
        if abs(self.cur_speed) > self.max_speed:
            self.cur_speed = (self.max_speed if self.cur_speed > 0
                                             else -self.max_speed)


