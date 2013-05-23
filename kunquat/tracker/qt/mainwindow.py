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

class MainWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None

        self._driver_select = DriverSelect()

        self._progressBar = QProgressBar(self)
        self._progressBar.setGeometry(QRect(30, 70, 481, 23))
        self._progressBar.setObjectName("progressBar")
        self._progressBar.setValue(1)
        self._progressBar.setMaximum(1)
        self._progressBar.setMinimum(0)

        self._render_stats = RenderStats()

        v = QVBoxLayout()
        v.addWidget(self._driver_select)
        v.addWidget(self._progressBar)
        v.addWidget(self._render_stats)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        driver_manager = self._ui_model.get_driver_manager()
        self._driver_select.set_driver_manager(driver_manager)
        stat_manager = self._ui_model.get_stat_manager()
        self._render_stats.set_stat_manager(stat_manager)
        stat_manager.register_updater(self.update_import_progress)

    def update_import_progress(self):
        stats = self._ui_model.get_stat_manager()
        position = stats.get_import_progress_position()
        steps = stats.get_import_progress_steps()
        self._progressBar.setMaximum(steps)
        self._progressBar.setValue(position)

    def run(self):
        self.update_import_progress()

    def __del__(self):
        pass


