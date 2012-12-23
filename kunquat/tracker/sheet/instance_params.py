# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtGui, QtCore

class InstanceParams(QtGui.QGroupBox):

    def __init__(self, project):
        QtGui.QWidget.__init__(self, 'instance')
        self._project = project
        self._layout = QtGui.QVBoxLayout(self)


    def init(self):
        pass

