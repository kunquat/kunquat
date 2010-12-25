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

import kqt_limits as lim


class InstList(QtGui.QTableWidget):

    def __init__(self, project, parent=None):
        QtGui.QTableWidget.__init__(self, lim.INSTRUMENTS_MAX, 1, parent)
        self.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)

        self._project = project
        self.setVerticalHeaderLabels([str(num) for num in
                                      xrange(lim.INSTRUMENTS_MAX)])
        self.setCornerButtonEnabled(False)
        self.setTabKeyNavigation(False)
        self.horizontalHeader().setStretchLastSection(True)
        self.horizontalHeader().hide()


