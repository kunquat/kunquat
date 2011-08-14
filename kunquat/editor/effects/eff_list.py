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

import kunquat.editor.kqt_limits as lim


class EffList(QtGui.QTableWidget):

    eff_changed = QtCore.pyqtSignal(int, name='effectChanged')

    def __init__(self, project, base, parent=None):
        self._max = lim.INST_EFFECTS_MAX if base.startswith('ins') \
                                         else lim.EFFECTS_MAX
        QtGui.QTableWidget.__init__(self, self._max, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)

        self._project = project
        self._base = base
        self.setVerticalHeaderLabels([str(num) for num in
                                      xrange(self._max)])
        self.setCornerButtonEnabled(False)
        self.setTabKeyNavigation(False)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()

        QtCore.QObject.connect(self,
                    QtCore.SIGNAL('currentCellChanged(int, int, int, int)'),
                               self._cell_changed)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('cellChanged(int, int)'),
                               self._name_changed)
        self.set_base(base)

    def eff_changed(self, num):
        index = self.model().index(num, 0)
        select = self.selectionModel()
        select.clear()
        select.select(index, QtGui.QItemSelectionModel.Select)
        select.setCurrentIndex(index, QtGui.QItemSelectionModel.Select)

    def _cell_changed(self, cur_row, cur_col, prev_row, prev_col):
        QtCore.QObject.emit(self, QtCore.SIGNAL('effectChanged(int)'),
                            cur_row)

    def _name_changed(self, num, col):
        assert num >= 0
        assert num < self._max
        item = self.item(num, 0)
        key = self._key_base.format(num) + 'm_name.json'
        if item:
            self._project[key] = str(item.text())
        else:
            self._project[key] = None

    def set_base(self, base):
        assert base.startswith('ins') == self._base.startswith('ins')
        self._base = base
        base = base if not base else base + '/'
        self._key_base = '{0}eff_{{0:02x}}/kqte{1}/'.format(base,
                                                     lim.FORMAT_VERSION)
        self.blockSignals(True)
        for i in xrange(self._max):
            name = self._project[self._key_base.format(i) + 'm_name.json']
            if name:
                item = self.item(i, 0)
                if not item:
                    item = QtGui.QTableWidgetItem(name)
                    self.setItem(i, 0, item)
                else:
                    item.setText(name)
            else:
                item = self.item(i, 0)
                if item:
                    item.setText('')
        self.blockSignals(False)

    def sync(self):
        self.set_base(self._base)


