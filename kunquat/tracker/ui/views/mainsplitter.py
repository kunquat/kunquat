# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
#          Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.tracker.cmdline as cmdline
from .composition import Composition
from .typewriterpanel import TypewriterPanel


class MainSplitter(QSplitter):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self.setOrientation(Qt.Vertical)
        self._composition = Composition()
        self._typewriter_panel = TypewriterPanel()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._composition)
        v.addWidget(self._typewriter_panel)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._composition.set_ui_model(ui_model)
        self._typewriter_panel.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._typewriter_panel.unregister_updaters()
        self._composition.unregister_updaters()

