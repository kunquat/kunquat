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


class SubsongParams(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._layout = QtGui.QVBoxLayout(self)
        self._subsong = 0

        key = 'subs_00/p_subsong.json'
        self._tempo = ParamSlider(project,
                                  'Initial tempo:',
                                  (6, 360),
                                  120,
                                  key,
                                  'tempo',
                                  unit='BPM')
        self._layout.addWidget(self._tempo)
        self._global_volume = ParamSlider(project,
                                          'Initial global volume:',
                                          (-96, 8),
                                          0,
                                          key,
                                          'global_vol',
                                          decimals=1,
                                          unit='dB')
        self._layout.addWidget(self._global_volume)

    def subsong_changed(self, subsong):
        self._subsong = subsong
        key = 'subs_{0:02d}/p_subsong.json'.format(subsong)
        self._tempo.set_key(key)
        self._global_volume.set_key(key)

    def sync(self):
        self._tempo.sync()
        self._global_volume.sync()


