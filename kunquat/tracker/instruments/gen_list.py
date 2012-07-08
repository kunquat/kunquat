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

from PyQt4 import QtCore, QtGui

import kunquat.tracker.kqt_limits as lim


class GenList(QtGui.QTableWidget):

    genChanged = QtCore.pyqtSignal(int, name='genChanged')

    def __init__(self, project, parent=None):
        QtGui.QTableWidget.__init__(self, lim.GENERATORS_MAX, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self._project = project
        self._cur_inst = 0
        self._cur_gen = 0
        self._ins_key_base = 'ins_{{0:02x}}/kqti{0}/'.format(lim.FORMAT_VERSION)
        self._gen_key_base = 'gen_{{0:02x}}/kqtg{0}/'.format(lim.FORMAT_VERSION)
        self.setVerticalHeaderLabels([str(num) for num in
                                      xrange(lim.GENERATORS_MAX)])
        self.setCornerButtonEnabled(False)
        self.setTabKeyNavigation(False)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()
        self._lock_update = False
        QtCore.QObject.connect(self,
                QtCore.SIGNAL('currentCellChanged(int, int, int, int)'),
                self._cell_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._name_changed)

    def inst_changed(self, num):
        self._cur_inst = num
        self.sync()
        """
        index = self.model().index(num, 0)
        select = self.selectionModel()
        select.clear()
        select.select(index, QtGui.QItemSelectionModel.Select)
        select.setCurrentIndex(index, QtGui.QItemSelectionModel.Select)
        """

    def _cell_changed(self, cur_row, cur_col, prev_row, prev_col):
        if cur_row == self._cur_gen:
            return
        self._cur_gen = cur_row
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('genChanged(int)'),
                            self._cur_gen)

    def _name_changed(self, num, col):
        assert num >= 0
        assert num < lim.GENERATORS_MAX
        if self._lock_update:
            return
        item = self.item(num, 0)
        gen_name_key = self._gen_key_base.format(num) + 'm_name.json'
        key = self._ins_key_base.format(self._cur_inst) + gen_name_key
        if item:
            self._project[key] = unicode(item.text())
        else:
            self._project[key] = None

    def sync(self):
        self._lock_update = True
        ins_key = self._ins_key_base.format(self._cur_inst)
        for i in xrange(lim.GENERATORS_MAX):
            gen_name_key = self._gen_key_base.format(i) + 'm_name.json'
            key = ins_key + gen_name_key
            name = self._project[key]
            item = self.item(i, 0)
            if name:
                if not item:
                    item = QtGui.QTableWidgetItem(name)
                    self.setItem(i, 0, item)
                else:
                    item.setText(name)
            elif item:
                item.setText('')
        self._lock_update = False


