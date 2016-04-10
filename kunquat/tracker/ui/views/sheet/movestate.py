# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import count, takewhile


class AbstractMoveState():

    def __init__(self):
        self._dir = 0
        self._pressed = { -1: False, 1: False }

    def get_delta(self):
        return self._dir

    # Press and release control

    def press(self, delta):
        if self._pressed[delta]:
            self._on_repeat()

        self._pressed[delta] = True
        if self._dir != delta:
            self._dir = delta
            self._on_dir_change()
        else:
            self._on_step()

    def release(self, delta):
        self._pressed[delta] = False
        if self._pressed[-delta]:
            if self._dir == delta:
                self.press(-delta)
        else:
            self._dir = 0
            self._on_release()

    # Protected callbacks

    def _on_repeat(self):
        pass

    def _on_dir_change(self):
        pass

    def _on_step(self):
        pass

    def _on_release(self):
        pass


class HorizontalMoveState(AbstractMoveState):

    def __init__(self):
        AbstractMoveState.__init__(self)

    def press_left(self):
        self.press(-1)

    def press_right(self):
        self.press(1)

    def release_left(self):
        self.release(-1)

    def release_right(self):
        self.release(1)


# Vertical movement parameters

_STEP_BASE = 1.15
_STEP_MAX = 16

_STEPS = [0] + [min(int(_STEP_BASE**e), _STEP_MAX) for e in
        takewhile(lambda n: _STEP_BASE**(n-1) <= _STEP_MAX, count(0))]

_SNAP_DELAYS = list(reversed(range(7)))


class VerticalMoveState(AbstractMoveState):

    def __init__(self):
        AbstractMoveState.__init__(self)
        self._step_index = 0

        self._is_snap_delay_enabled = False
        self._snap_delay_index = 0
        self._snap_delay_counter = 0

    def get_delta(self):
        if self._is_snap_delay_enabled and self._snap_delay_counter > 0:
            self._snap_delay_counter -= 1
            return 0
        return self._dir * _STEPS[min(self._step_index, len(_STEPS) - 1)]

    def press_up(self):
        self.press(-1)

    def press_down(self):
        self.press(1)

    def release_up(self):
        self.release(-1)

    def release_down(self):
        self.release(1)

    def _on_repeat(self):
        self._is_snap_delay_enabled = True

    def _on_dir_change(self):
        self._step_index = 1
        self._snap_delay_index = 0

    def _on_step(self):
        if self._snap_delay_counter == 0:
            self._step_index += 1

    def _on_release(self):
        self._is_snap_delay_enabled = False
        self._snap_delay_index = 0

    def try_snap_delay(self):
        if self._is_snap_delay_enabled:
            self._snap_delay_counter = _SNAP_DELAYS[self._snap_delay_index]
            self._snap_delay_index = min(
                    self._snap_delay_index + 1, len(_SNAP_DELAYS) - 1)


