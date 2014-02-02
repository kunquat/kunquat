# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import count, takewhile


_STEP_BASE = 1.15
_STEP_MAX = 16

_STEPS = [0] + [min(int(_STEP_BASE**e), _STEP_MAX) for e in
        takewhile(lambda n: _STEP_BASE**(n-1) <= _STEP_MAX, count(0))]

_SNAP_DELAYS = list(reversed(xrange(7)))


class VerticalMoveState():

    def __init__(self):
        self._dir = 0
        self._step_index = 0
        self._up_pressed = False
        self._down_pressed = False

        self._is_snap_delay_enabled = False
        self._snap_delay_index = 0
        self._snap_delay_counter = 0

    def get_delta(self):
        if self._is_snap_delay_enabled and self._snap_delay_counter > 0:
            self._snap_delay_counter -= 1
            return 0
        return self._dir * _STEPS[min(self._step_index, len(_STEPS) - 1)]

    def press_up(self):
        if self._up_pressed:
            self._is_snap_delay_enabled = True

        self._up_pressed = True
        if self._dir >= 0:
            self._dir = -1
            self._step_index = 1
            self._snap_delay_index = 0
        else:
            if self._snap_delay_counter == 0:
                self._step_index += 1

    def press_down(self):
        if self._down_pressed:
            self._is_snap_delay_enabled = True

        self._down_pressed = True
        if self._dir <= 0:
            self._dir = 1
            self._step_index = 1
            self._snap_delay_index = 0
        else:
            if self._snap_delay_counter == 0:
                self._step_index += 1

    def release_up(self):
        self._up_pressed = False
        if self._down_pressed:
            if self._dir < 0:
                self.press_down()
        else:
            self._dir = 0
            self._is_snap_delay_enabled = False
            self._snap_delay_index = 0

    def release_down(self):
        self._down_pressed = False
        if self._up_pressed:
            if self._dir > 0:
                self.press_up()
        else:
            self._dir = 0
            self._is_snap_delay_enabled = False
            self._snap_delay_index = 0

    def try_snap_delay(self):
        if self._is_snap_delay_enabled:
            self._snap_delay_counter = _SNAP_DELAYS[self._snap_delay_index]
            self._snap_delay_index = min(
                    self._snap_delay_index + 1, len(_SNAP_DELAYS) - 1)


