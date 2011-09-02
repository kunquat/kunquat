# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtGui, QtCore

import kunquat.editor.kqt_limits as lim
from kunquat.editor.param_check import ParamCheck
from kunquat.editor.param_slider import ParamSlider
from kunquat.editor.param_spin import ParamSpin


class CompParams(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._layout = QtGui.QVBoxLayout(self)

        self._mix_vol = ParamSlider(project,
                                    'Mixing volume:',
                                    (-48, 0),
                                    -8,
                                    'p_composition.json',
                                    'mix_vol',
                                    decimals=1,
                                    unit='dB')
        self._random_source = RandomSource(project)
        self._layout.addWidget(self._mix_vol)
        self._layout.addWidget(self._random_source)

    def sync(self):
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


