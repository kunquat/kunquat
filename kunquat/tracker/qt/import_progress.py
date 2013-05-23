# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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
        QProgressBar.__init__(self)
        self._stat_manager = None

        self.setGeometry(QRect(30, 70, 481, 23))
        self.setObjectName("progressBar")
        self.setValue(1)
        self.setMaximum(1)
        self.setMinimum(0)

    def set_stat_manager(self, stat_manager):
        self._stat_manager = stat_manager
        self._stat_manager.register_updater(self.update_import_progress)

    def update_import_progress(self):
        position = self._stat_manager.get_import_progress_position()
        steps = self._stat_manager.get_import_progress_steps()
        self.setMaximum(steps)
        self.setValue(position)

