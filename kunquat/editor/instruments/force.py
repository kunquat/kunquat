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

from __future__ import print_function

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim
from kunquat.editor.param_slider import ParamSlider


class Force(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._cur_inst = 0
        self._project = project
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._force = ParamSlider(project,
                                  'Default force:',
                                  (-96, 8),
                                  0,
                                  'ins_00/kqti{0}/p_instrument.json'.format(
                                          lim.FORMAT_VERSION),
                                  'force',
                                  decimals=1,
                                  unit='dB')
        self._force_var = ParamSlider(project,
                                      'Force variation:',
                                      (0, 48),
                                      0,
                                      'ins_00/kqti{0}/p_instrument.json'.format(
                                              lim.FORMAT_VERSION),
                                      'force_variation',
                                      decimals=1,
                                      unit='dB')
        layout.addWidget(self._force)
        layout.addWidget(self._force_var)
        self._widgets = [self._force, self._force_var]

    def inst_changed(self, num):
        self._cur_inst = num
        for widget in self._widgets:
            widget.set_key('ins_{0:02x}/kqti{1}/p_instrument.json'.format(
                           self._cur_inst, lim.FORMAT_VERSION))

    def sync(self):
        for widget in self._widgets:
            widget.sync()


