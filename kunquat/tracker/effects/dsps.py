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

import kunquat.tracker.kqt_limits as lim
from dsp_editor import DSPEditor
from dsp_list import DSPList


class DSPs(QtGui.QSplitter):

    def __init__(self, project, base, parent=None):
        QtGui.QSplitter.__init__(self, parent)
        self._project = project
        self._dsp_list = DSPList(project, base)
        self._dsp_editor = DSPEditor(project, base)
        self.addWidget(self._dsp_list)
        self.addWidget(self._dsp_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([160, 1])
        self.set_base(base)
        QtCore.QObject.connect(self._dsp_list,
                               QtCore.SIGNAL('dspChanged(int)'),
                               self._dsp_editor.dsp_changed)

    def set_base(self, base):
        self._base = base
        self._dsp_list.set_base(base)
        self._dsp_editor.set_base(base)

    def sync(self):
        self.set_base(self._base)


