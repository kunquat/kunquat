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

from __future__ import print_function

from PyQt4 import QtCore, QtGui

from env_pitch_pan import EnvPitchPan


class Panning(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._cur_inst = 0
        self._project = project
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._env_pitch_pan = EnvPitchPan(project)
        layout.addWidget(self._env_pitch_pan, 1)

    def init(self):
        self._env_pitch_pan.init()

    def inst_changed(self, num):
        self._cur_inst = num
        self._env_pitch_pan.inst_changed(num)

    def sync(self):
        self._env_pitch_pan.sync()


