# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtCore, QtGui


class EmptyRange(QtGui.QLabel):

    def __init__(self, index, allow_degen=True, parent=None):
        QtGui.QLabel.__init__(self, parent=parent)
        self.index = index

    @property
    def range(self):
        return []

    @range.setter
    def range(self, r):
        return


