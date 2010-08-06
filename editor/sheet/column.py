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

import json
import re

import kunquat
from PyQt4 import Qt, QtGui, QtCore

import kqt_limits


column_path = re.compile('(pat_[0-9a-f]{3})/(gcol|ccol_[0-9a-f]{2})/')


class Column(object):

    """
    def __init__(self, handle, path):
        assert handle
        self.handle = handle
        self.set_path(path)

    def set_path(self, path):
        assert column_path.match(path)
        self.path = path
    """

    def __init__(self, num, triggers):
        assert num >= -1
        self.num = num
        self.triggers = sorted(triggers) if triggers else []
        self.width = 128
        self.height = 0

    def resize(self, ev):
        self.height = ev.size().height()

    def paint(self, ev, paint, x):
        col_area = QtCore.QRect(x, 0, self.width, self.height)
        real_area = ev.rect().intersect(col_area)
        if real_area.isEmpty():
            return
        paint.drawRect(real_area)


