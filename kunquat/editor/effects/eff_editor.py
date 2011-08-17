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


class EffEditor(QtGui.QWidget):

    def __init__(self, project, base, parent=None):
        QtGui.QWidget.__init__(self, parent)

        self._project = project
        self._base = base
        self._cur_eff = 0
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        load = QtGui.QPushButton('Load')

        remove = QtGui.QPushButton('Remove')

        layout.addWidget(load, 0)
        QtCore.QObject.connect(load,
                               QtCore.SIGNAL('clicked()'),
                               self.load)
        layout.addWidget(remove, 0)
        QtCore.QObject.connect(remove,
                               QtCore.SIGNAL('clicked()'),
                               self.remove)

    def eff_changed(self, num):
        self._cur_eff = num

    def load(self):
        slot = self._cur_eff
        fname = QtGui.QFileDialog.getOpenFileName(
                caption='Load Kunquat effect (to index {0})'.format(slot),
                filter='Kunquat effects (*.kqte *.kqte.gz *.kqte.bz2)')
        if fname:
            self._project.import_kqte(self._base, slot, str(fname))

    def remove(self):
        self._project.remove_dir('{0}eff_{1:02x}'.format(self._base,
                                                         self._cur_eff))

    def set_base(self, base):
        self._base = base

    def sync(self):
        self.set_base(self._base)


