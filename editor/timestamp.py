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

import math

from kqt_limits import TIMESTAMP_BEAT


class Timestamp(tuple):

    def __new__(cls, beats=0, rem=0):
        _beats = int(math.floor(beats) + rem // TIMESTAMP_BEAT)
        _rem = int((beats - math.floor(beats)) * TIMESTAMP_BEAT +
                   rem % TIMESTAMP_BEAT)
        return tuple.__new__(cls, (_beats, _rem))

    def __lt__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return tuple(self) < tuple(other)

    def __le__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return tuple(self) <= tuple(other)

    def __eq__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return tuple(self) == tuple(other)

    def __ne__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return tuple(self) != tuple(other)

    def __gt__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return tuple(self) > tuple(other)

    def __ge__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return tuple(self) >= tuple(other)

    def __nonzero__(self):
        return self[0] != 0 or self[1] != 0

    __bool__ = __nonzero__

    def __add__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return Timestamp(self[0] + other[0], self[1] + other[1])

    def __sub__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return Timestamp(self[0] - other[0], self[1] - other[1])

    def __mul__(self, other):
        other = float(other)
        rems = self[0] * TIMESTAMP_BEAT + self[1]
        return Timestamp(rem=rems * other)

    def __div__(self, other):
        return self.__mul__(1 / float(other))

    def __radd__(self, other):
        return self.__add__(other)

    def __rsub__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return other - self

    def __rmul__(self, other):
        return self.__mul__(other)

    def __rdiv__(self, other):
        return NotImplemented

    def __neg__(self):
        return 0 - self

    def __pos__(self):
        return self

    def __abs__(self):
        if self < 0:
            return -self
        return self

    def __repr__(self):
        return 'Timestamp{0}'.format(tuple.__repr__(self))

    def __int__(self):
        return self[0]

    def __float__(self):
        return self[0] + float(self[1]) / TIMESTAMP_BEAT


