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

from PyQt4 import QtCore, QtGui

from dsp_generic import DSPGeneric
import kunquat.tracker.kqt_limits as lim
from kunquat.tracker.param_check import ParamCheck
from kunquat.tracker.param_combo import ParamCombo


class DSPEditor(QtGui.QWidget):

    def __init__(self, project, base, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._base = base
        self._cur_dsp = 0
        self._dsp_key_base = 'dsp_{{0:02x}}/kqtd{0}/'.format(lim.FORMAT_VERSION)
        common_layout = QtGui.QHBoxLayout()
        common_layout.setMargin(0)
        common_layout.setSpacing(0)
        key_base = self._base + self._dsp_key_base.format(self._cur_dsp)
        self._type = ParamCombo(project,
                                'DSP type',
                                [('No DSP', None),
                                    ('Chorus', 'chorus'),
                                    ('Convolution', 'convolution'),
                                    ('Tap delay', 'delay'),
                                    ('Freeverb', 'freeverb'),
                                    ('Gain compression', 'gaincomp'),
                                    ('Volume', 'volume'),
                                ],
                                'No DSP',
                                key_base + 'p_dsp_type.json')
        common_layout.addWidget(self._type)
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._generic = DSPGeneric(project, base)
        layout.addLayout(common_layout)
        layout.addWidget(self._generic, 1)
        QtCore.QObject.connect(self._type,
                               QtCore.SIGNAL('currentIndexChanged(int)'),
                               self._type_changed)

    def dsp_changed(self, num):
        self._cur_dsp = num
        key_base = self._base + self._dsp_key_base.format(self._cur_dsp)
        self._type.set_key(key_base + 'p_dsp_type.json')
        self._generic.dsp_changed(num)

    def set_base(self, base):
        self._base = base
        key_base = self._base + self._dsp_key_base.format(self._cur_dsp)
        self._type.set_key(key_base + 'p_dsp_type.json')
        self._generic.set_base(self._base)

    def _type_changed(self, dummy):
        self._generic.sync()

    def sync(self):
        self.set_base(self._base)


