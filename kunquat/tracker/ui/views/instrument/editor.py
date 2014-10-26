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

from name import Name
from globalforce import GlobalForce
from force_env import ForceEnvelope


class Editor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._name = Name()
        self._global_force = GlobalForce()
        self._force_env = ForceEnvelope()

        v = QVBoxLayout()
        v.addWidget(self._name)
        v.addWidget(self._global_force)
        v.addWidget(self._force_env)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._name.set_ins_id(ins_id)
        self._global_force.set_ins_id(ins_id)
        self._force_env.set_ins_id(ins_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._name.set_ui_model(ui_model)
        self._global_force.set_ui_model(ui_model)
        self._force_env.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._force_env.unregister_updaters()
        self._global_force.unregister_updaters()
        self._name.unregister_updaters()


