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

from PyQt4 import QtGui, QtCore

from param_combo import ParamCombo
from param_spin import ParamSpin


class ParamSampleHeader(QtGui.QWidget):

    def __init__(self,
                 project,
                 key,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._format = ParamCombo(project, 'Format',
                                  [('WavPack', 'WavPack')],
                                  'WavPack',
                                  key, 'format')
        self._freq = ParamSpin(project, 'Middle frequency',
                               (1, 2**24),
                               48000,
                               key,
                               'freq',
                               0)
        loop_layout = QtGui.QHBoxLayout()
        self._loop_mode = ParamCombo(project, '',
                                     [('No loop', 'off'),
                                      ('Unidirectional', 'uni'),
                                      ('Bidirectional', 'bi')],
                                     'No loop',
                                     key, 'loop_mode')
        self._loop_start = ParamSpin(project, 'Loop start',
                                     (0, 2**24),
                                     0,
                                     key,
                                     'loop_start',
                                     0)
        self._loop_end = ParamSpin(project, 'Loop end',
                                   (0, 2**24),
                                   0,
                                   key,
                                   'loop_end',
                                   0)
        loop_layout.addWidget(self._loop_mode)
        loop_layout.addWidget(self._loop_start)
        loop_layout.addWidget(self._loop_end)
        layout.addWidget(self._format)
        layout.addWidget(self._freq)
        layout.addLayout(loop_layout)
        self._key = key
        self._widgets = [self._format, self._freq, self._loop_mode,
                         self._loop_start, self._loop_end]

    def init(self):
        for widget in self._widgets:
            widget.init()

    def set_key(self, key):
        self._key = key
        for widget in self._widgets:
            widget.set_key(key)

    def sync(self):
        self.set_key(self._key)


