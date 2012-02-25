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


class Env(QtGui.QWidget):

    def __init__(self,
                 project,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        vars_layout = QtGui.QVBoxLayout()
        vars_layout.setMargin(0)
        vars_layout.setSpacing(0)
        vars_layout.addWidget(QtGui.QLabel('Variables'))
        self._vars = Variables(project, 'p_environment.json')
        vars_layout.addWidget(self._vars)

        map_layout = QtGui.QVBoxLayout()
        map_layout.setMargin(0)
        map_layout.setSpacing(0)
        map_layout.addWidget(QtGui.QLabel('Bind'))
        self._bind = Bind(project)
        map_layout.addWidget(self._bind, 1)

        layout.addLayout(vars_layout, 1)
        layout.addLayout(map_layout, 2)

    def sync(self):
        self._vars.sync()
        self._bind.sync()


