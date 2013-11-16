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
        updater = ui_model.get_updater()
        updater.register_updater(self.perform_updates)
        self._ui_manager = ui_model.get_ui_manager()
        self._module = ui_model.get_module()

    def _select_instrument(self, instrument_number):
        instrument = self._instrument_catalog[instrument_number]
        instrument_id = instrument.get_id()
        self._ui_manager.set_selected_instrument_id(instrument_id)

    def update_texts(self):
        for i, instrument in self._instrument_catalog.items():
            instrument_name = instrument.get_name() or '-'
            play = '' if len(instrument.get_active_notes()) < 1 else u'*'
            text = '%s %s' % (instrument_name, play)
            self.setItemText(i, text)

    def update_instruments(self):
        instruments = self._module.get_instruments()
        selected_instrument_id = self._ui_manager.get_selected_instrument_id()
        old_block = self.blockSignals(True)
        self.clear()
        self._instrument_catalog = dict(enumerate(instruments))
        invalid_selection = True
        for i, instrument in self._instrument_catalog.items():
            self.addItem('')
            if selected_instrument_id:
                instrument_id = instrument.get_id()
                if instrument_id == selected_instrument_id:
                    self.setCurrentIndex(i)
                    invalid_selection = False
        self.update_texts()
        self.blockSignals(old_block)
        if invalid_selection and len(instruments) > 0:
            top_instrument = instruments[0]
            top_instrument_id = top_instrument.get_id()
            self._ui_manager.set_selected_instrument_id(top_instrument_id)

    def perform_updates(self, signals):
        self.update_instruments()
        self.update_texts()

