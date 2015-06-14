# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *


def lerp_val(v1, v2, t):
    assert 0 <= t <= 1
    return v1 + (v2 - v1) * t

def lerp_colour(c1, c2, t):
    assert 0 <= t <= 1
    return QColor(
            lerp_val(c1.red(), c2.red(), t),
            lerp_val(c1.green(), c2.green(), t),
            lerp_val(c1.blue(), c2.blue(), t))


