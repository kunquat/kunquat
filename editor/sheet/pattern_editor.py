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
import timestamp as ts
from timestamp_spin import TimestampSpin


class PatternEditor(QtGui.QWidget):

    def __init__(self,
                 project,
                 playback_manager,
                 section_manager,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        section_manager.connect(self.section_changed)
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        top_control = QtGui.QWidget()
        top_layout = QtGui.QHBoxLayout(top_control)

        name = QtGui.QLabel('[pattern num/name]')

        autoinst = QtGui.QCheckBox('Explicit inst.')
        autoinst.setChecked(True)

        self._length = TimestampSpin(project,
                                     'Length:',
                                     (ts.Timestamp(0), ts.Timestamp(1024, 0)),
                                     ts.Timestamp(16),
                                     'pat_000/p_pattern.json',
                                     'length',
                                     2)

        top_layout.addWidget(name, 1)
        top_layout.addWidget(autoinst, 0)
        top_layout.addWidget(self._length, 0)

        pattern = Pattern(project, section_manager, playback_manager)
        layout.addWidget(top_control, 0)
        layout.addWidget(pattern, 1)
        QtCore.QObject.connect(self._length,
                               QtCore.SIGNAL('tsChanged(int, int)'),
                               pattern.length_changed)
        QtCore.QObject.connect(autoinst,
                               QtCore.SIGNAL('stateChanged(int)'),
                               pattern.autoinst_changed)

    def section_changed(self, subsong, section):
        pattern = self._project.get_pattern(subsong, section)
        if pattern != None:
            key = 'pat_{0:03d}/p_pattern.json'.format(pattern)
            self._length.set_key(key)


