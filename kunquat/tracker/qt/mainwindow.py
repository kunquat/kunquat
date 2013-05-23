# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
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

from driver_select import DriverSelect
from render_stats import RenderStats
from import_progress import ImportProgress

class MainWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None

        self._driver_select = DriverSelect()
        self._import_progress = ImportProgress()
        self._render_stats = RenderStats()

        v = QVBoxLayout()
        v.addWidget(self._driver_select)
        v.addWidget(self._import_progress)
        v.addWidget(self._render_stats)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        driver_manager = self._ui_model.get_driver_manager()
        stat_manager = self._ui_model.get_stat_manager()
        self._driver_select.set_driver_manager(driver_manager)
        self._render_stats.set_stat_manager(stat_manager)
        self._import_progress.set_stat_manager(stat_manager)

