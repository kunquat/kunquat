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

from PyQt4 import QtCore

import kqt_limits as lim
import timestamp


class Cursor(object):

    def __init__(self, length, beat_len):
        self.init_speed = 1
        self.max_speed = 12
        self.init_trigger_delay = 6
        self.cur_trigger_delay = self.init_trigger_delay
        self.trigger_delay_left = 0
        self.set_length(length)
        self.set_beat_len(beat_len)
        self.set_col(-1)
        self.set_pos(timestamp.Timestamp())
        self.set_index(0)
        self.set_accel(1.18)
        self.direction = 0

    def key_press(self, ev):
        if ev.key() == QtCore.Qt.Key_Up:
            self.set_direction(-1)
            self.step()
        elif ev.key() == QtCore.Qt.Key_Down:
            self.set_direction(1)
            self.step()
        else:
            ev.ignore()

    def key_release(self, ev):
        if ev.key() in (QtCore.Qt.Key_Up, QtCore.Qt.Key_Down):
            self.set_direction()
        else:
            ev.ignore()

    def set_beat_len(self, beat_len):
        assert beat_len > 0
        self.beat_len = beat_len

    def set_length(self, length):
        assert length >= 0
        self.length = length

    def set_col(self, col):
        assert col >= -1
        assert col < lim.COLUMNS_MAX
        self.col = col

    def set_pos(self, ts):
        assert ts >= 0
        self.ts = ts
        self.pix_pos = ts * self.beat_len

    def set_pix_pos(self, pix_pos):
        self.pix_pos = max(0, pix_pos)
        self.ts = timestamp.Timestamp(self.pix_pos / self.beat_len)
        if self.ts > self.length:
            self.set_pos(self.length)

    def get_pix_pos(self):
        return self.pix_pos

    def set_index(self, index):
        assert index >= 0
        self.index = index

    def set_accel(self, accel):
        assert accel >= 1
        self.accel = accel

    def set_direction(self, direction=0):
        assert direction in [-1, 0, 1]
        if self.direction == direction:
            return
        self.direction = direction
        self.cur_speed = self.init_speed * direction

    def step(self):
        self.set_pix_pos(self.pix_pos + self.cur_speed)
        if abs(self.cur_speed) == self.max_speed:
            return
        self.cur_speed *= self.accel
        if abs(self.cur_speed) > self.max_speed:
            self.cur_speed = (self.max_speed if self.cur_speed > 0
                                             else -self.max_speed)


