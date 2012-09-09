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

    def __init__(self, p, project, instrument_spin, parent=None):
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
        load = QtGui.QPushButton('Load')
        save = QtGui.QPushButton('Save')
        remove = QtGui.QPushButton('Remove')

        layout.addWidget(test, 0)
        layout.addWidget(load, 0)
        layout.addWidget(save, 0)
        layout.addWidget(remove, 0)

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
        QtCore.QObject.connect(load,
                               QtCore.SIGNAL('clicked()'),
                               self.load)
        QtCore.QObject.connect(save,
                               QtCore.SIGNAL('clicked()'),
                               self.save)
        QtCore.QObject.connect(remove,
                               QtCore.SIGNAL('clicked()'),
                               self.remove)

        QtCore.QObject.connect(instrument_spin,
                               QtCore.SIGNAL('currentIndexChanged(const QString&)'),
                               self.inst_changed)

    def init(self):
        self._force.init()
        self._panning.init()
        self._filter.init()
        self._generators.init()
        self._effects.init()
        self._connections.init()

    def inst_changed(self, text):
        if text == '':
            return
        parts = text.split(':')
        num = int(parts[0] )

        self._cur_inst = num
        self._force.inst_changed(num)
        self._panning.inst_changed(num)
        self._filter.inst_changed(num)
        self._generators.inst_changed(num)
        ins_key = self._ins_key_base.format(self._cur_inst)
        self._effects.set_base(ins_key)
        self._connections.set_key(ins_key + 'p_connections.json')

    def test_note_on(self):
        ev = QtGui.QKeyEvent(QtCore.QEvent.KeyPress,
                             QtCore.Qt.Key_section,
                             QtCore.Qt.NoModifier)
        QtCore.QCoreApplication.postEvent(self, ev)

    def test_note_off(self):
        ev = QtGui.QKeyEvent(QtCore.QEvent.KeyRelease,
                             QtCore.Qt.Key_section,
                             QtCore.Qt.NoModifier)
        QtCore.QCoreApplication.postEvent(self, ev)

    def load(self):
        slot = 0
        ids = self.p.project._composition.instrument_ids()
        numbers = [int(i.split('_')[1]) for i in ids]
        while slot in numbers:
            slot += 1
        fname = QtGui.QFileDialog.getOpenFileName(
                caption='Load Kunquat instrument (to index {0})'.format(slot),
                filter='Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)')
        if fname:
            self._project.import_kqti(slot, str(fname))

    def save(self):
        slot = self._cur_inst
        fname = QtGui.QFileDialog.getSaveFileName(
                caption='Save Kunquat instrument (of index {0})'.format(slot),
                filter='Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)')
        if fname:
            self._project.export_kqti(slot, str(fname))

    def remove(self):
        inst = self._project._composition.get_instrument(self._cur_inst)
        inst.delall()        

    def sync(self):
        self._force.sync()
        self._panning.sync()
        self._filter.sync()
        self._generators.sync()
        self._effects.sync()
        self._connections.sync()


