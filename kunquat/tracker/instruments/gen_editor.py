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

from gen_generic import GenGeneric
import kunquat.tracker.kqt_limits as lim
from kunquat.tracker.param_check import ParamCheck
from kunquat.tracker.param_combo import ParamCombo


class GenEditor(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._cur_inst = 0
        self._cur_gen = 0
        self._ins_key_base = 'ins_{{0:02x}}/kqti{0}/'.format(lim.FORMAT_VERSION)
        self._gen_key_base = 'gen_{{0:02x}}/kqtg{0}/'.format(lim.FORMAT_VERSION)
        common_layout = QtGui.QHBoxLayout()
        common_layout.setMargin(0)
        common_layout.setSpacing(0)
        key_base = self._ins_key_base.format(self._cur_inst) + \
                   self._gen_key_base.format(self._cur_gen)
        self._enabled = ParamCheck(project,
                                   'Enabled',
                                   False,
                                   key_base + 'p_generator.json',
                                   'enabled')
        self._type = ParamCombo(project,
                                'Generator type',
                                [('No generator', None),
                                    ('Additive', 'add'),
                                    ('Sample', 'pcm')],
                                'No generator',
                                key_base + 'p_gen_type.json')
        common_layout.addWidget(self._enabled)
        common_layout.addWidget(self._type)
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._generic = GenGeneric(project)
        layout.addLayout(common_layout)
        layout.addWidget(self._generic, 1)

    def init(self):
        self._enabled.init()
        self._type.init()
        self._generic.init()
        QtCore.QObject.connect(self._type,
                               QtCore.SIGNAL('currentIndexChanged(int)'),
                               self._type_changed)

    def inst_changed(self, num):
        self._cur_inst = num
        self._update_keys()
        self._generic.inst_changed(self._cur_inst)

    def gen_changed(self, num):
        self._cur_gen = num
        self._update_keys()
        self._generic.gen_changed(self._cur_gen)

    def _type_changed(self, dummy):
        self._generic.sync()

    def _update_keys(self):
        key_base = self._ins_key_base.format(self._cur_inst) + \
                   self._gen_key_base.format(self._cur_gen)
        self._enabled.set_key(key_base + 'p_generator.json')
        self._type.set_key(key_base + 'p_gen_type.json')

    def sync(self):
        self._enabled.sync()
        self._type.sync()
        self._generic.sync()


