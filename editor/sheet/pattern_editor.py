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

from PyQt4 import QtGui, QtCore

import kqt_limits as lim
from pattern import Pattern


class PatternEditor(QtGui.QWidget):

    def __init__(self,
                 project,
                 playback_manager,
                 section_manager,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        top_control = QtGui.QWidget()
        top_layout = QtGui.QHBoxLayout(top_control)

        name = QtGui.QLabel('[pattern num/name]')

        length = QtGui.QLabel('[length]')

        top_layout.addWidget(name)
        top_layout.addWidget(length)

        pattern = Pattern(project, section_manager, playback_manager)
        layout.addWidget(top_control, 0)
        layout.addWidget(pattern, 1)


