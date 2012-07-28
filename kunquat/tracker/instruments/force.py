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

from env_force import EnvForce
from env_force_rel import EnvForceRel
import kunquat.tracker.kqt_limits as lim
from kunquat.tracker.param_slider import ParamSlider


class Force(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._cur_inst = 0
        self._project = project
        self._inst_key = 'ins_{0:02x}/p_instrument.json'
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._gforce = ParamSlider(project,
                                   'Global force:',
                                   (-96, 8),
                                   0,
                                   self._inst_key.format(self._cur_inst),
                                   'global_force',
                                   decimals=1,
                                   unit='dB')
        self._force = ParamSlider(project,
                                  'Default force:',
                                  (-96, 8),
                                  0,
                                  self._inst_key.format(self._cur_inst),
                                  'force',
                                  decimals=1,
                                  unit='dB')
        self._force_var = ParamSlider(project,
                                      'Force variation:',
                                      (0, 48),
                                      0,
                                      self._inst_key.format(self._cur_inst),
                                      'force_variation',
                                      decimals=1,
                                      unit='dB')
        self._env_force = EnvForce(project)
        self._env_force_rel = EnvForceRel(project)
        layout.addWidget(self._gforce)
        layout.addWidget(self._force)
        layout.addWidget(self._force_var)
        layout.addWidget(self._env_force, 1)
        layout.addWidget(self._env_force_rel, 1)
        self._sliders = [self._gforce,
                         self._force,
                         self._force_var,
                        ]

    def inst_changed(self, num):
        self._cur_inst = num
        for widget in self._sliders:
            widget.set_key(self._inst_key.format(self._cur_inst))
        self._env_force.inst_changed(num)
        self._env_force_rel.inst_changed(num)

    def sync(self):
        for widget in self._sliders:
            widget.sync()
        self._env_force.sync()
        self._env_force_rel.sync()


