# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
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

from aunumslider import AuNumSlider


class GlobalForce(AuNumSlider):

    def __init__(self):
        AuNumSlider.__init__(self, 1, -64.0, 18.0, width_txt='-00.0')
        self.set_number(0)

    def _update_value(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        self.set_number(au.get_global_force())

    def _value_changed(self, global_force):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_global_force(global_force)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return ''.join(('signal_global_force_', self._au_id))


