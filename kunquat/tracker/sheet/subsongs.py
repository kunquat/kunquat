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

from __future__ import division
from __future__ import print_function

from PyQt4 import QtGui, QtCore

import kunquat.tracker.kqt_limits as lim


class Subsongs(QtGui.QTreeView):

    def __init__(self, p):
        QtGui.QWidget.__init__(self)
        self.p = p
        song_list = QtGui.QTreeView
        self.setHeaderHidden(True)
        #self.setDragDropMode(QtGui.QAbstractItemView.InternalMove)


    def init(self):
        self.update()

    def update(self):
        pass



