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

from PyQt4 import QtCore, QtGui

from inst_editor import InstEditor
from inst_list import InstList
from itertools import cycle
import kunquat.tracker.kqt_limits as lim

class Instruments(QtGui.QSplitter):

    def __init__(self,
                 tw,
                 piano,
                 project,
                 instrument_spin,
                 playback_manager,
                 note_input,
                 scale,
                 octave_spin,
                 parent=None):
        QtGui.QSplitter.__init__(self, parent)

        self._tw = tw
        self._piano = piano
        self._project = project
        self._inst_list = InstList(project, instrument_spin)
        self._inst_editor = InstEditor(project, instrument_spin)
        self._instrument_spin = instrument_spin
        self._playback_manager = playback_manager
        self._note_input = note_input
        self._scale = scale
        self._octave_spin = octave_spin

        self.addWidget(self._inst_list)
        self.addWidget(self._inst_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([240, 1])

        self._inst_num = 0
        self._channel = cycle(xrange(lim.COLUMNS_MAX))

        QtCore.QObject.connect(instrument_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.inst_changed)
        QtCore.QObject.connect(octave_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.octave_changed)

    def inst_changed(self, num):
        self._piano._inst_num = num
        self._inst_num = num

    def octave_changed(self, num):
        self._note_input.base_octave = num

    def keyPressEvent(self, ev):
        self._tw.keyPressEvent(ev)

    def keyReleaseEvent(self, ev):
        self._tw.keyReleaseEvent(ev)

    def sync(self):
        self._inst_list.sync()
        self._inst_editor.sync()

