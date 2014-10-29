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

from insnumslider import InsNumSlider


class ForceVariation(InsNumSlider):

    def __init__(self):
        InsNumSlider.__init__(self, 1, 0.0, 32.0, width_txt='-00.0')
        self.set_number(0)

    def _update_value(self):
        old_block = self.blockSignals(True)
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        self.set_number(instrument.get_force_variation())
        self.blockSignals(old_block)

    def _value_changed(self, force_var):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        instrument.set_force_variation(force_var)
        self._updater.signal_update(set(['signal_instrument']))


