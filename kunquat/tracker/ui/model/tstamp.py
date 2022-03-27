# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2022
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections.abc import Sequence
import math
import numbers

from kunquat.kunquat.limits import TSTAMP_BEAT


BEAT = TSTAMP_BEAT


class Tstamp(numbers.Real, tuple):

    def __new__(cls, beats=0, rem=0):
        if isinstance(beats, Sequence):
            beats, rem = beats[0], beats[1]
        beats_floor = math.floor(beats)
        _beats = int(beats_floor + rem // BEAT)
        _rem = int((beats - beats_floor) * BEAT + rem % BEAT)
        return tuple.__new__(cls, (_beats, _rem))

    @property
    def beats(self):
        return self[0]

    @property
    def rem(self):
        return self[1]

    def __abs__(self):
        return max(self, -self)

    def __add__(self, other):
        other_ts = Tstamp(other)
        return Tstamp(self.beats + other_ts.beats, self.rem + other_ts.rem)

    def __eq__(self, other):
        if other == None:
            return False
        other_ts = Tstamp(other)
        return self.beats == other_ts.beats and self.rem == other_ts.rem

    def __float__(self):
        return float(self.beats) + float(self.rem) / BEAT

    def __floordiv__(self, other):
        if isinstance(other, Tstamp):
            self_rems = self.beats * BEAT + self.rem
            other_rems = other.beats * BEAT + other.rem
            return Tstamp(self_rems // other_rems, 0)

        div_ts = self / other
        return Tstamp(div_ts, 0)

    def __ge__(self, other):
        other_ts = Tstamp(other)
        return other_ts <= self

    def __gt__(self, other):
        other_ts = Tstamp(other)
        return other_ts < self

    def __hash__(self):
        if self.rem == 0:
            return hash(self.beats)
        fv = float(self)
        if self == fv:
            return hash(fv)
        return hash(tuple(self))

    def __le__(self, other):
        other_ts = Tstamp(other)
        if self.beats < other_ts.beats:
            return True
        elif self.beats > other_ts.beats:
            return False
        return self.rem <= other_ts.rem

    def __lt__(self, other):
        other_ts = Tstamp(other)
        if self.beats < other_ts.beats:
            return True
        elif self.beats > other_ts.beats:
            return False
        return self.rem < other_ts.rem

    def __mod__(self, other):
        if isinstance(other, Tstamp):
            self_rems = self.beats * BEAT + self.rem
            other_rems = other.beats * BEAT + other.rem
            mod_rems = self_rems % other_rems
            return Tstamp(0, mod_rems)

        raise NotImplementedError

    def __mul__(self, other):
        if isinstance(other, Tstamp):
            total_rems = (
                    self.beats * other.beats * BEAT +
                    self.beats * other.rem +
                    self.rem * other.beats +
                    self.rem * other.rem / BEAT)
            return Tstamp(0, total_rems)

        rems = self.beats * BEAT + self.rem
        return Tstamp(0, rems * other)

    def __neg__(self):
        return Tstamp(-self.beats, -self.rem)

    def __pos__(self):
        return self

    def __pow__(self):
        raise NotImplementedError

    def __radd__(self, other):
        return self.__add__(other)

    def __rdiv__(self):
        raise NotImplementedError

    def __repr__(self):
        return 'Tstamp{}'.format(tuple(self))

    def __rfloordiv__(self):
        raise NotImplementedError

    def __rmod__(self):
        raise NotImplementedError

    def __rmul__(self, other):
        return self.__mul__(other)

    def __rpow__(self):
        raise NotImplementedError

    def __rtruediv__(self):
        raise NotImplementedError

    def __truediv__(self, other):
        if isinstance(other, Tstamp):
            raise NotImplementedError

        rems = self.beats * BEAT + self.rem
        return Tstamp(0, rems / other)

    def __trunc__(self):
        if self.beats < 0 and self.rem > 0:
            return self.beats + 1
        return self.beats


