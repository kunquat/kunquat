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
from gen_list import GenList
from gen_editor import GenEditor


class Generators(QtGui.QSplitter):

    def __init__(self, project, parent=None):
        QtGui.QSplitter.__init__(self, parent)
        self._project = project
        self._gen_list = GenList(project)
        self._gen_editor = GenEditor(project)
        self.addWidget(self._gen_list)
        self.addWidget(self._gen_editor)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([160, 1])

    def inst_changed(self, num):
        self._gen_list.inst_changed(num)
        self._gen_editor.inst_changed(num)

    def sync(self):
        self._gen_list.sync()
        self._gen_editor.sync()


