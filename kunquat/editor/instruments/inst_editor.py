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

from __future__ import print_function

from PyQt4 import QtCore, QtGui

import force
import panning
import kunquat.editor.kqt_limits as lim
#import kunquat.editor.envelope as envelope


class InstEditor(QtGui.QWidget):

    def __init__(self, project, instrument_spin, parent=None):
        QtGui.QWidget.__init__(self, parent)

        self._project = project
        self._cur_inst = instrument_spin.value()
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
        QtCore.QObject.connect(test,
                               QtCore.SIGNAL('pressed()'),
                               self.test_note_on)
        QtCore.QObject.connect(test,
                               QtCore.SIGNAL('released()'),
                               self.test_note_off)
        layout.addWidget(load, 0)
        QtCore.QObject.connect(load,
                               QtCore.SIGNAL('clicked()'),
                               self.load)
        layout.addWidget(save, 0)
        QtCore.QObject.connect(save,
                               QtCore.SIGNAL('clicked()'),
                               self.save)
        layout.addWidget(remove, 0)
        QtCore.QObject.connect(remove,
                               QtCore.SIGNAL('clicked()'),
                               self.remove)

        QtCore.QObject.connect(instrument_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.inst_changed)

        top_layout.addLayout(layout)

        tabs = QtGui.QTabWidget()
        self._force = force.Force(project)
        self._panning = panning.Panning(project)
        tabs.addTab(self._force, 'Force')
        tabs.addTab(self._panning, 'Panning')
        tabs.addTab(QtGui.QWidget(), 'Filter')
        tabs.addTab(QtGui.QWidget(), 'Generators')
        tabs.addTab(QtGui.QWidget(), 'Effects')
        tabs.addTab(QtGui.QWidget(), 'Connections')
        top_layout.addWidget(tabs)

    def inst_changed(self, num):
        self._cur_inst = num
        self._force.inst_changed(num)

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
        slot = self._cur_inst
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
        self._project.remove_dir('ins_{0:02x}'.format(self._cur_inst))

    def sync(self):
        self._force.sync()


