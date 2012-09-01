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

from kunquat.tracker.connections import Connections
from dsps import DSPs
import kunquat.tracker.kqt_limits as lim


class EffEditor(QtGui.QWidget):

    def __init__(self, project, base, parent=None):
        QtGui.QWidget.__init__(self, parent)

        self._project = project
        self._base = base
        self._cur_eff = 0
        top_layout = QtGui.QVBoxLayout(self)
        top_layout.setMargin(0)
        top_layout.setSpacing(0)
        layout = QtGui.QHBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(0)

        load = QtGui.QPushButton('Load')
        save = QtGui.QPushButton('Save')
        remove = QtGui.QPushButton('Remove')

        layout.addWidget(load, 0)
        layout.addWidget(save, 0)
        layout.addWidget(remove, 0)

        top_layout.addLayout(layout)

        tabs = QtGui.QTabWidget()
        self._dsps = DSPs(project, self._base +
                'eff_{0:02x}/kqte{1}/'.format(self._cur_eff,
                                              lim.FORMAT_VERSION))
        self._connections = Connections(project,
                                        self._base + 'p_connections.json')
        tabs.addTab(self._dsps, 'DSPs')
        tabs.addTab(self._connections, 'Connections')
        top_layout.addWidget(tabs)
        QtCore.QObject.connect(load,
                               QtCore.SIGNAL('clicked()'),
                               self.load)
        QtCore.QObject.connect(save,
                               QtCore.SIGNAL('clicked()'),
                               self.save)
        QtCore.QObject.connect(remove,
                               QtCore.SIGNAL('clicked()'),
                               self.remove)

    def init(self):
        self._dsps.init()
        self._connections.init()

    def eff_changed(self, num):
        self._cur_eff = num
        self.set_base(self._base)

    def load(self):
        slot = self._cur_eff
        fname = QtGui.QFileDialog.getOpenFileName(
                caption='Load Kunquat effect (to index {0})'.format(slot),
                filter='Kunquat effects (*.kqte *.kqte.gz *.kqte.bz2)')
        if fname:
            self._project.import_kqte(self._base, slot, str(fname))

    def save(self):
        slot = self._cur_eff
        fname = QtGui.QFileDialog.getSaveFileName(
                caption='Save Kunquat effect (of index {0})'.format(slot),
                filter='Kunquat effects (*.kqte *.kqte.gz *.kqte.bz2)')
        if fname:
            self._project.export_kqte(self._base, slot, str(fname))

    def remove(self):
        self._project.remove_dir('{0}eff_{1:02x}'.format(self._base,
                                                         self._cur_eff))

    def set_base(self, base):
        self._base = base
        eff_key = self._base + 'eff_{0:02x}/kqte{1}/'.format(
                self._cur_eff, lim.FORMAT_VERSION)
        self._dsps.set_base(eff_key)
        self._connections.set_key(eff_key + 'p_connections.json')

    def sync(self):
        self.set_base(self._base)


