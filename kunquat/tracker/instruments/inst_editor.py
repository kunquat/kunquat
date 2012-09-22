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

from __future__ import print_function

from PyQt4 import QtCore, QtGui

import filter_opts
import force
import generators
import panning
import kunquat.tracker.connections as connections
import kunquat.tracker.effects.effects as effects
import kunquat.tracker.kqt_limits as lim
#import kunquat.tracker.envelope as envelope


class InstEditor(QtGui.QWidget):

    def __init__(self, p, project, instrument, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.p = p


        self._project = project
        self._cur_inst = 0
        self._ins_key_base = 'ins_{0:02x}/'
        top_layout = QtGui.QVBoxLayout(self)
        top_layout.setMargin(0)
        top_layout.setSpacing(0)
        layout = QtGui.QHBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(0)

        test = QtGui.QPushButton('Test')

        layout.addWidget(test, 0)

        top_layout.addLayout(layout)

        tabs = QtGui.QTabWidget()
        self._force = force.Force(project)
        self._panning = panning.Panning(project)
        self._filter = filter_opts.Filter(project)
        self._generators = generators.Generators(project)
        self._effects = effects.Effects(project,
                                self._ins_key_base.format(self._cur_inst))
        self._connections = connections.Connections(project,
                                self._ins_key_base.format(self._cur_inst) +
                                'p_connections.json')
        tabs.addTab(self._force, 'Force')
        tabs.addTab(self._panning, 'Panning')
        tabs.addTab(self._filter, 'Filter')
        tabs.addTab(self._generators, 'Generators')
        tabs.addTab(self._effects, 'Effects')
        tabs.addTab(self._connections, 'Connections')
        top_layout.addWidget(tabs)

        QtCore.QObject.connect(test,
                               QtCore.SIGNAL('pressed()'),
                               self.test_note_on)
        QtCore.QObject.connect(test,
                               QtCore.SIGNAL('released()'),
                               self.test_note_off)

        self.inst_changed(instrument)
        inst = self._project._composition.get_instrument(self._cur_inst)
        title = u'Instrument %s' % inst.get_id_name()
        self.setWindowTitle(title)

    def inst_changed(self, ins_id):
        if ins_id == '':
            return
        parts = ins_id.split('_')
        num = int(parts[1] )

        self._cur_inst = num
        self._force.inst_changed(num)
        self._panning.inst_changed(num)
        self._filter.inst_changed(num)
        self._generators.inst_changed(num)
        ins_key = self._ins_key_base.format(self._cur_inst)
        self._effects.set_base(ins_key)
        self._connections.set_key(ins_key + 'p_connections.json')

    def init(self):
        self._force.init()
        self._panning.init()
        self._filter.init()
        self._generators.init()
        self._effects.init()
        self._connections.init()

    def test_note_on(self):
        self.p._tw.press_random()

    def test_note_off(self):
        self.p._tw.release_random()

    def sync(self):
        self._force.sync()
        self._panning.sync()
        self._filter.sync()
        self._generators.sync()
        self._effects.sync()
        self._connections.sync()


