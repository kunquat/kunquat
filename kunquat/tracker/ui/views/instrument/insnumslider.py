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

from numberslider import NumberSlider


class InsNumSlider(NumberSlider):

    def __init__(self, decimal_count, min_val, max_val, title=''):
        NumberSlider.__init__(self, decimal_count, min_val, max_val, title)
        self._ins_id = None
        self._ui_model = None
        self._updater = None

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_value()
        QObject.connect(self, SIGNAL('numberChanged(float)'), self._value_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_instrument' in signals:
            self._update_value()

    # Protected interface

    def _update_value(self):
        raise NotImplementedError

    def _value_changed(self, new_value):
        raise NotImplementedError


