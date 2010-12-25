# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtCore, QtGui

from inst_editor import InstEditor
from inst_list import InstList


class Instruments(QtGui.QSplitter):

    def __init__(self, project, instrument_spin, parent=None):
        QtGui.QSplitter.__init__(self, parent)

        self._project = project

        self._inst_list = InstList(project, instrument_spin)

        self._inst_editor = InstEditor(project, instrument_spin)

        self.addWidget(self._inst_list)
        self.addWidget(self._inst_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([240, 1])


