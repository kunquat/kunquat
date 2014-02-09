# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2014
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
        self._updater = None
        self._ui_manager = None
        self._module = None
        self._control_catalog = dict()
        QObject.connect(self, SIGNAL("currentIndexChanged(int)"), self._select_instrument)

    def set_ui_model(self, ui_model):
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._ui_manager = ui_model.get_ui_manager()
        self._module = ui_model.get_module()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_controls' in signals:
            self._update_controls()
        self._update_control_texts()

    def _select_instrument(self, catalog_index):
        control_id = self._control_catalog[catalog_index]
        self._ui_manager.set_selected_control_id(control_id)

    def _update_control_texts(self):
        for i, control_id in self._control_catalog.items():
            parts = control_id.split('_')
            second = parts[1]
            control_number = int(second, 16)
            control = self._module.get_control(control_id)
            instrument = control.get_instrument()
            instrument_name = instrument.get_name() or '-'
            play = '' if len(control.get_active_notes()) < 1 else u'*'
            text = 'instrument %s: %s %s' % (control_number, instrument_name, play)
            self.setItemText(i, text)

    def _update_controls(self):
        control_ids = self._module.get_control_ids()
        self._control_catalog = dict(enumerate(sorted(control_ids)))
        selected_control_id = self._ui_manager.get_selected_control_id()
        old_block = self.blockSignals(True)
        self.clear()
        for i, control_id in self._control_catalog.items():
            self.addItem('')
            if selected_control_id and control_id == selected_control_id:
                self.setCurrentIndex(i)
        self._update_control_texts()
        self.blockSignals(old_block)


