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

from PyQt4 import QtGui, QtCore

import kunquat.editor.kqt_limits as lim
from param_slider import ParamSlider


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
        self._layout.addWidget(self._mix_vol)

    def sync(self):
        self._mix_vol.sync()


