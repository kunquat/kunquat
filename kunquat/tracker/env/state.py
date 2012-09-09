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

from bind import Bind
from variables import Variables


class State(QtGui.QWidget):

    def __init__(self,
                 project,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project

        self._vars = Variables(project, 'p_environment.json')

        #self.setCentralWidget(self._vars)

        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        layout.addWidget(self._vars)

    def init(self):
        self._vars.init()

    def sync(self):
        self._vars.sync()


