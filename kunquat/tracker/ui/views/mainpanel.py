# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
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
from portal import Portal
from typewriterpanel import TypewriterPanel
from importprogress import ImportProgress
from peakmeter import PeakMeter


class MainPanel(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._portal = Portal()
        self._typewriter_panel = TypewriterPanel()
        self._import_progress = ImportProgress()
        self._peak_meter = PeakMeter()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.addWidget(self._portal)
        v.addWidget(self._typewriter_panel)
        v.addWidget(self._import_progress)
        v.addWidget(self._peak_meter)
        self.setLayout(v)

        self._typewriter_panel.setFocus()

        if not cmdline.get_experimental():
            self._import_progress.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._portal.set_ui_model(ui_model)
        self._typewriter_panel.set_ui_model(ui_model)
        self._import_progress.set_ui_model(ui_model)
        self._peak_meter.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._peak_meter.unregister_updaters()
        self._import_progress.unregister_updaters()
        self._typewriter_panel.unregister_updaters()
        self._portal.unregister_updaters()

