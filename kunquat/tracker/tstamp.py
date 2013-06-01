# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import Sequence
import math
import numbers


BEAT = 882161280


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
        raise NotImplementedError

    def __add__(self):
        raise NotImplementedError

    def __div__(self):
        raise NotImplementedError

    def __eq__(self):
        raise NotImplementedError

    def __float__(self):
        raise NotImplementedError

    def __floordiv__(self):
        raise NotImplementedError

    def __le__(self):
        raise NotImplementedError

    def __lt__(self):
        raise NotImplementedError

    def __mod__(self):
        raise NotImplementedError

    def __mul__(self):
        raise NotImplementedError

    def __neg__(self):
        raise NotImplementedError

    def __pos__(self):
        raise NotImplementedError

    def __pow__(self):
        raise NotImplementedError

    def __radd__(self):
        raise NotImplementedError

    def __rdiv__(self):
        raise NotImplementedError

    def __rfloordiv__(self):
        raise NotImplementedError

    def __rmod__(self):
        raise NotImplementedError

    def __rmul__(self):
        raise NotImplementedError

    def __rpow__(self):
        raise NotImplementedError

    def __rtruediv__(self):
        raise NotImplementedError

    def __truediv__(self):
        raise NotImplementedError

    def __trunc__(self):
        raise NotImplementedError


