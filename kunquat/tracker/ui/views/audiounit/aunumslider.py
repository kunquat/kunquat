# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.numberslider import NumberSlider
from .audiounitupdater import AudioUnitUpdater


class AuNumSlider(NumberSlider, AudioUnitUpdater):

    def __init__(self, decimal_count, min_val, max_val, title='', width_txt=''):
        super().__init__(decimal_count, min_val, max_val, title, width_txt)

    def _on_setup(self):
        self.register_action('signal_au', self._update_value)
        self.register_action(self._get_update_signal_type(), self._update_value)
        self.register_action('signal_style_changed', self._update_style)
        self.numberChanged.connect(self._value_changed)
        self._update_style()
        self._update_value()

    def _update_style(self):
        self.update_style(self._ui_model.get_style_manager())

    # Protected interface

    def _get_update_signal_type(self):
        raise NotImplementedError

    def _update_value(self):
        raise NotImplementedError

    def _value_changed(self, new_value):
        raise NotImplementedError


