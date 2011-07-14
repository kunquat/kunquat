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

import kunquat.editor.kqt_limits as lim


class GenGeneric(QtGui.QSplitter):

    def __init__(self, project, parent=None):
        QtGui.QSplitter.__init__(self, parent)
        self._project = project
        self._cur_inst = 0
        self._cur_gen = 0
        self._key_temp = ('ins_{{0:02x}}/kqti{0}/'
                          'gen_{{1:02x}}/kqtg{0}/'.format(lim.FORMAT_VERSION))
        self._key_base = self._key_temp.format(self._cur_inst, self._cur_gen)
        self._key_list = KeyList(project,
                                 self._key_base + 'c/')
        self._key_editor = QtGui.QLabel('[key editor]')
        self.addWidget(self._key_list)
        self.addWidget(self._key_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([280, 1])

    def inst_changed(self, num):
        self._cur_inst = num
        self._update_keys()

    def gen_changed(self, num):
        self._cur_gen = num
        self._update_keys()

    def _update_keys(self):
        self._key_base = self._key_temp.format(self._cur_inst, self._cur_gen)
        self._key_list.set_key(self._key_base + 'c/')

    def sync(self):
        self._key_list.sync()


class KeyList(QtGui.QTableWidget):

    def __init__(self, project, key, parent=None):
        QtGui.QTableWidget.__init__(self, 1, 1, parent)
        self._project = project
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()
        self.verticalHeader().hide()
        self.set_key(key)

    def set_key(self, key):
        self._key = key
        while self.rowCount() > 0:
            self.removeRow(0)
        self.setItem(0, 0, QtGui.QTableWidgetItem())
        for key in self._project.subtree(self._key):
            key_trunc = key[len(self._key):]
            item = QtGui.QTableWidgetItem(key_trunc)
            self.insertRow(self.rowCount())
            self.setItem(self.rowCount() - 1, 0, item)
        self.sortItems(0)
        self.insertRow(self.rowCount())

    def sync(self):
        self.set_key(self._key)


