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

from __future__ import division, print_function

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim
from kunquat.editor.envelope import Envelope


class EnvForceRel(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._cur_inst = 0
        self._project = project
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._env = Envelope(project)
        layout.addWidget(self._env)


