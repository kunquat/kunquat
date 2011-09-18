# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function
from itertools import takewhile
import re

from PyQt4 import QtCore, QtGui

from dsp_constraints import constraints
from kunquat.editor.dev_editor import DevEditor
import kunquat.editor.kqt_limits as lim


class DSPGeneric(DevEditor):

    def __init__(self, project, base, parent=None):
        DevEditor.__init__(self, project, constraints, base, parent)
        self._base = base
        self._cur_dsp = 0

    def dsp_changed(self, num):
        self._cur_dsp = num
        self.set_base(self._base)

    def set_base(self, base):
        self._base = base
        dsp_base = '{0}dsp_{1:02x}/kqtd{2}/'.format(base, self._cur_dsp,
                               lim.FORMAT_VERSION)
        self.set_key_base(dsp_base)


