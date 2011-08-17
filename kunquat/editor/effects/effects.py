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

from eff_editor import EffEditor
from eff_list import EffList


class Effects(QtGui.QSplitter):

    def __init__(self, project, base, parent=None):
        QtGui.QSplitter.__init__(self, parent)

        self._project = project
        self._base = base

        self._eff_list = EffList(project, base)
        self._eff_editor = EffEditor(project, base)

        QtCore.QObject.connect(self._eff_list,
                               QtCore.SIGNAL('effectChanged(int)'),
                               self._eff_editor.eff_changed)

        self.addWidget(self._eff_list)
        self.addWidget(self._eff_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([240, 1])

    def set_base(self, base):
        self._base = base
        self._eff_list.set_base(base)
        self._eff_editor.set_base(base)

    def sync(self):
        self.set_base(self._base)


