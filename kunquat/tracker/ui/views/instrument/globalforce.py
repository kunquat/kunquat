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

from forceslider import ForceSlider


class GlobalForce(ForceSlider):

    def __init__(self):
        ForceSlider.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._updater = None

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_value()
        QObject.connect(self, SIGNAL('forceChanged(float)'), self._value_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_instrument' in signals:
            self._update_value()

    def _update_value(self):
        old_block = self.blockSignals(True)
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        self.set_force(instrument.get_global_force())
        self.blockSignals(old_block)

    def _value_changed(self, global_force):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        instrument.set_global_force(global_force)
        self._updater.signal_update(set(['signal_instrument']))


