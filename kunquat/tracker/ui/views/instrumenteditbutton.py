# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
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


class InstrumentEditButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self.setText('Edit')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        module = self._ui_model.get_module()
        control_manager = self._ui_model.get_control_manager()
        control_id = control_manager.get_selected_control_id()
        control = module.get_control(control_id)
        au = control.get_audio_unit()

        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_audio_unit(au.get_id())


