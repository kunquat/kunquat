# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4 import QtCore, QtGui

import kunquat.tracker.kqt_limits as lim
from kunquat.tracker.envelope import Envelope
from kunquat.tracker.param_check import ParamCheck


class EnvPitchPan(QtGui.QWidget):

    def __init__(self,
                 project,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._cur_inst = 0
        self._project = project
        self._key_base = 'ins_{0:02x}/p_envelope_pitch_pan.json'
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        top_layout = QtGui.QHBoxLayout()
        top_layout.setMargin(0)
        top_layout.setSpacing(0)
        label = QtGui.QLabel('Pitch-pan envelope:')
        self._enabled = ParamCheck(project,
                                   'Enabled',
                                   False,
                                   self._key_base.format(self._cur_inst),
                                   'enabled')
        top_layout.addSpacing(10)
        top_layout.addWidget(label)
        top_layout.addSpacing(10)
        top_layout.addWidget(self._enabled)

        self._env = Envelope(project,
                             (-6000, 6000),
                             (-1, 1),
                             (True, True),
                             (False, False),
                             [(-6000, 0), (6000, 0)],
                             32,
                             self._key_base.format(0),
                             'envelope')
        layout.addLayout(top_layout)
        layout.addWidget(self._env, 1)

        self._widgets = [self._enabled, self._env]

    def init(self):
        for widget in self._widgets:
            widget.init()

    def inst_changed(self, num):
        self._cur_inst = num
        for widget in self._widgets:
            widget.set_key(self._key_base.format(self._cur_inst))

    def sync(self):
        for widget in self._widgets:
            widget.sync()


