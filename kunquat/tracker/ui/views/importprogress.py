# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2014-2016
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


class ImportProgress(QProgressBar):

    def __init__(self):
        super().__init__()
        self._updater = None
        self._stat_manager = None

        self.setGeometry(QRect(30, 70, 481, 23))
        self.setObjectName("progressBar")
        self.setValue(1)
        self.setMaximum(1)
        self.setMinimum(0)

    def set_ui_model(self, ui_model):
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._stat_manager = ui_model.get_stat_manager()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        self._update_import_progress()

    def _update_import_progress(self):
        position = self._stat_manager.get_import_progress_position()
        steps = self._stat_manager.get_import_progress_steps()
        self.setMaximum(steps)
        self.setValue(position)


