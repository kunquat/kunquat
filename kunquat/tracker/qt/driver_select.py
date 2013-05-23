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


class DriverSelect(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._driver_manager = None
        self._driver_catalog = {}
        QObject.connect(self, SIGNAL("currentIndexChanged(int)"), self._select_driver)

    def set_driver_manager(self, driver_manager):
        self._driver_manager = driver_manager
        driver_manager.register_updater(self.update_drivers)

    def _select_driver(self, i):
        if i < 0:
            return
        driver = self._driver_catalog[i]
        self._driver_manager.set_selected_driver(driver)

    def update_drivers(self):
        drivers = self._driver_manager.get_drivers()
        selected = self._driver_manager.get_selected_driver()
        old_block = self.blockSignals(True)
        self.clear()
        self._driver_catalog = dict(enumerate(drivers))
        for i, driver in self._driver_catalog.items():
            name = driver.get_id()
            self.addItem(name)
            if selected:
                current = selected.get_id()
                name = driver.get_id()
                if name == current:
                    self.setCurrentIndex(i)
        self.blockSignals(old_block)

