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


class TestButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._au_id = None
        self._control_manager = None
        self._typewriter_manager = None
        self._button_model = None

        self.setText('Test')

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._control_manager = ui_model.get_control_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()
        QObject.connect(self, SIGNAL('pressed()'), self._pressed)
        QObject.connect(self, SIGNAL('released()'), self._released)

    def unregister_updaters(self):
        pass

    def _pressed(self):
        module = self._ui_model.get_module()
        control_id = module.get_control_id_by_au_id(self._au_id)
        if not control_id:
            self._button_model = None
            return

        au = module.get_audio_unit(self._au_id)

        self._control_manager.set_control_id_override(control_id)
        au.set_test_expressions_enabled(True)
        self._button_model = self._typewriter_manager.get_random_button_model()
        self._button_model.start_tracked_note()
        self._control_manager.set_control_id_override(None)
        au.set_test_expressions_enabled(False)

    def _released(self):
        if self._button_model:
            self._button_model.stop_tracked_note()


