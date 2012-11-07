# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtGui, QtCore

import kunquat.tracker.kqt_limits as lim
from kunquat.tracker.param_check import ParamCheck
from kunquat.tracker.param_line import ParamLine
from kunquat.tracker.param_slider import ParamSlider
from kunquat.tracker.param_spin import ParamSpin


class CompParams(QtGui.QGroupBox):

    def __init__(self, project):
        QtGui.QGroupBox.__init__(self, 'album')
        self._project = project
        self._layout = QtGui.QVBoxLayout(self)

        self._title = ParamLine(project,
                                'Title:',
                                '',
                                'm_title.json')
        self._authors = ParamLine(project,
                                  'Author(s):',
                                  [],
                                  'm_authors.json')
        self._mix_vol = ParamSlider(project,
                                    'Mixing volume:',
                                    (-48, 0),
                                    -8,
                                    'p_composition.json',
                                    'mix_vol',
                                    decimals=1,
                                    unit='dB')
        self._random_source = RandomSource(project)
        self._layout.addWidget(self._title)
        self._layout.addWidget(self._authors)
        self._layout.addWidget(self._mix_vol)
        self._layout.addWidget(self._random_source)

    def init(self):
        self._title.init()
        self._authors.init()
        self._mix_vol.init()
        self._random_source.init()

    def sync(self):
        self._title.sync()
        self._authors.sync()
        self._mix_vol.sync()
        self._random_source.sync()


class RandomSource(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        layout = QtGui.QHBoxLayout(self)
        self._custom_enabled = ParamCheck(project,
                                          'Custom random seed',
                                          False,
                                          'i_custom_random_seed.json')
        self._seed = ParamSpin(project,
                               '',
                               (0, 2**52 - 1),
                               0,
                               'p_random_seed.json')
        layout.addWidget(self._custom_enabled)
        layout.addWidget(self._seed)

    def init(self):
        self._custom_enabled.init()
        self._seed.init()
        QtCore.QObject.connect(self._custom_enabled,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._custom_changed)
        self.sync()
        self._seed.setEnabled(self._custom_enabled.checkState() ==
                              QtCore.Qt.Checked)

    def _custom_changed(self, state):
        self._seed.setEnabled(state == QtCore.Qt.Checked)

    def sync(self):
        self._custom_enabled.sync()
        self._seed.sync()


