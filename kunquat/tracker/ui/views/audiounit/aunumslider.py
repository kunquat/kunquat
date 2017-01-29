# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.views.numberslider import NumberSlider
from .updatingauview import UpdatingAUView


class AuNumSlider(NumberSlider, UpdatingAUView):

    def __init__(self, decimal_count, min_val, max_val, title='', width_txt=''):
        super().__init__(decimal_count, min_val, max_val, title, width_txt)

    def _on_setup(self):
        self.register_action('signal_au', self._update_value)
        self.register_action(self._get_update_signal_type(), self._update_value)
        QObject.connect(self, SIGNAL('numberChanged(float)'), self._value_changed)
        self._update_value()

    # Protected interface

    def _get_update_signal_type(self):
        raise NotImplementedError

    def _update_value(self):
        raise NotImplementedError

    def _value_changed(self, new_value):
        raise NotImplementedError


