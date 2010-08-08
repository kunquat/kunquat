#!/usr/bin/env python
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

from kqt_limits import TIMESTAMP_BEAT


class Timestamp(object):

    def __init__(self, ts=(0, 0)):
        assert isinstance(ts, tuple)
        assert len(ts) == 2
        assert isinstance(ts[0], int)
        assert isinstance(ts[1], int)
        assert ts[1] >= 0
        assert ts[1] < TIMESTAMP_BEAT
        self.beats = ts[0]
        self.rem = ts[1]

    def __lt__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        if self.beats > other.beats:
            return False
        return self.beats < other.beats or self.rem < other.rem

    def __le__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        if self.beats > other.beats:
            return False
        return self.beats < other.beats or self.rem <= other.rem

    def __eq__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        return self.beats == other.beats and self.rem == other.rem

    def __ne__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        return self.beats != other.beats or self.rem != other.rem

    def __gt__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        if self.beats < other.beats:
            return False
        return self.beats > other.beats or self.rem > other.rem

    def __ge__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        if self.beats < other.beats:
            return False
        return self.beats > other.beats or self.rem >= other.rem

    def __hash__(self):
        return (self.beats, self.rem).__hash__()

    def __nonzero__(self):
        return self.beats != 0 or self.rem != 0

    def __add__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        beats = self.beats + other.beats
        rem = self.rem + other.rem
        if rem >= TIMESTAMP_BEAT:
            beats += 1
            rem -= TIMESTAMP_BEAT
        elif rem < 0:
            beats -= 1
            rem += TIMESTAMP_BEAT
        return Timestamp((beats, rem))

    def __sub__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
        beats = self.beats - other.beats
        rem = self.rem - other.rem
        if rem >= TIMESTAMP_BEAT:
            beats += 1
            rem -= TIMESTAMP_BEAT
        elif rem < 0:
            beats -= 1
            rem += TIMESTAMP_BEAT
        return Timestamp((beats, rem))

    def __radd__(self, other):
        return self.__add__(other)

    def __rsub__(self, other):
        if isinstance(other, int):
            other = Timestamp((other, 0))
            return other - self
        return NotImplemented

    def __neg__(self):
        return 0 - self

    def __pos__(self):
        return Timestamp((self.beats, self.rem))

    def __abs__(self):
        if self < 0:
            return -self
        return self

    def __repr__(self):
        return '(%d, %d)' % (self.beats, self.rem)

    def __int__(self):
        return self.beats

    def __float__(self):
        return self.beats + float(self.rem) / TIMESTAMP_BEAT


