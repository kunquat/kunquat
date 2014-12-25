# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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


class AddGen(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None

        self._phase_mod_enabled_toggle = QCheckBox('Phase modulation')

        v = QVBoxLayout()
        v.addWidget(self._phase_mod_enabled_toggle)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_gen()

        QObject.connect(
                self._phase_mod_enabled_toggle,
                SIGNAL('stateChanged(int)'),
                self._phase_mod_enabled_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_gen()

    def _get_add_params(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        add_params = generator.get_type_params()
        return add_params

    def _update_gen(self):
        add_params = self._get_add_params()

        old_block = self._phase_mod_enabled_toggle.blockSignals(True)
        self._phase_mod_enabled_toggle.setCheckState(
                Qt.Checked if add_params.get_phase_mod_enabled() else Qt.Unchecked)
        self._phase_mod_enabled_toggle.blockSignals(old_block)

    def _phase_mod_enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        add_params = self._get_add_params()
        add_params.set_phase_mod_enabled(new_enabled)


