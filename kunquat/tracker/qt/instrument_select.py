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


class InstrumentSelect(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._ui_manager = None
        self._module = None
        self._instrument_catalog = {}
        QObject.connect(self, SIGNAL("currentIndexChanged(int)"), self._select_instrument)

    def set_ui_model(self, ui_model):
        self._ui_manager = ui_model.get_ui_manager()
        self._ui_manager.register_updater(self.update_instruments)
        self._module = ui_model.get_module()
        self._module.register_updater(self.update_instruments)

    def _select_instrument(self, i):
        if i < 0:
            return
        instrument = self._instrument_catalog[i]
        self._ui_manager.set_selected_instrument(instrument)

    def update_instruments(self):
        instruments = self._module.get_instruments()
        selected = self._ui_manager.get_selected_instrument()
        old_block = self.blockSignals(True)
        self.clear()
        self._instrument_catalog = dict(enumerate(instruments))
        invalid_selection = True
        for i, instrument in self._instrument_catalog.items():
            instrument_number = instrument.get_instrument_number()
            instrument_name = instrument.get_name() or '-'
            text = 'instrument %s: %s' % (instrument_number, instrument_name)
            self.addItem(text)
            if selected:
                current = selected.get_instrument_number()
                instrument_number = instrument.get_instrument_number()
                if instrument_number == current:
                    self.setCurrentIndex(i)
                    invalid_selection = False
        self.blockSignals(old_block)
        if invalid_selection and len(instruments) > 0:
            self._ui_manager.set_selected_instrument(instruments[0])

