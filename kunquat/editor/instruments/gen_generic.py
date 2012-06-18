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

from gen_constraints import constraints
from kunquat.editor.dev_editor import DevEditor
import kunquat.editor.kqt_limits as lim


class GenGeneric(DevEditor):

    def __init__(self, project, parent=None):
        key_temp = ('ins_{{0:02x}}/kqti{0}/'
                    'gen_{{1:02x}}/kqtg{0}/'.format(lim.FORMAT_VERSION))
        DevEditor.__init__(self, project, constraints, key_temp.format(0, 0),
                           parent)
        self._key_temp = key_temp
        self._cur_inst = 0
        self._cur_gen = 0

    def inst_changed(self, num):
        self._cur_inst = num
        self._update_keys()

    def gen_changed(self, num):
        self._cur_gen = num
        self._update_keys()

    def _update_keys(self):
        self.set_key_base(self._key_temp.format(self._cur_inst, self._cur_gen))


