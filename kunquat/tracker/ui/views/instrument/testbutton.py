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


class TestButton(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._control_manager = None
        self._typewriter_manager = None
        self._button_model = None

        self.setText('Test')

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

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
        control_id = module.get_control_id_by_instrument_id(self._ins_id)
        if not control_id:
            return

        self._control_manager.set_control_id_override(control_id)
        self._button_model = self._typewriter_manager.get_random_button_model()
        self._button_model.start_tracked_note()
        self._control_manager.set_control_id_override(None)

    def _released(self):
        self._button_model.stop_tracked_note()


