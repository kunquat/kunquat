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

import math

from kqt_limits import TIMESTAMP_BEAT


class Timestamp(object):

    def __init__(self, beats=0, rem=0):
        self.beats = int(math.floor(beats) + rem // TIMESTAMP_BEAT)
        self.rem = int((beats - math.floor(beats)) * TIMESTAMP_BEAT +
                       rem % TIMESTAMP_BEAT)

    def __lt__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return (self.beats, self.rem) < (other.beats, other.rem)

    def __le__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return (self.beats, self.rem) <= (other.beats, other.rem)

    def __eq__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return self.beats == other.beats and self.rem == other.rem

    def __ne__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return self.beats != other.beats or self.rem != other.rem

    def __gt__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return (self.beats, self.rem) > (other.beats, other.rem)

    def __ge__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return (self.beats, self.rem) >= (other.beats, other.rem)

    def __hash__(self):
        return (self.beats, self.rem).__hash__()

    def __nonzero__(self):
        return self.beats != 0 or self.rem != 0

    def __add__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return Timestamp(self.beats + other.beats, self.rem + other.rem)

    def __sub__(self, other):
        if not isinstance(other, Timestamp):
            other = Timestamp(other)
        return Timestamp(self.beats - other.beats, self.rem - other.rem)

    def __mul__(self, other):
        other = float(other)
        rems = self.beats * TIMESTAMP_BEAT + self.rem
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
        return Timestamp(self.beats, self.rem)

    def __abs__(self):
        if self < 0:
            return -self
        return self

    def __repr__(self):
        return 'Timestamp({0}, {1})'.format(self.beats, self.rem)

    def __int__(self):
        return self.beats

    def __float__(self):
        return self.beats + float(self.rem) / TIMESTAMP_BEAT


