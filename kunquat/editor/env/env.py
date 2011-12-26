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

        set_map_layout = QtGui.QVBoxLayout()
        set_map_layout.setMargin(0)
        set_map_layout.setSpacing(0)
        set_map_layout.addWidget(QtGui.QLabel('[Set map]'))

        layout.addLayout(vars_layout)
        layout.addLayout(set_map_layout)

    def sync(self):
        self._vars.sync()


