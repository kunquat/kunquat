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

from globalforce import GlobalForce
from forcevariation import ForceVariation
from defaultforce import DefaultForce
from force_env import ForceEnvelope
from forcerel_env import ForceReleaseEnvelope


class ForceEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._au_id = None
        self._global_force = GlobalForce()
        self._force_var = ForceVariation()
        self._default_force = DefaultForce()
        self._force_env = ForceEnvelope()
        self._force_rel_env = ForceReleaseEnvelope()

        sliders = QGridLayout()
        sliders.addWidget(QLabel('Global force:'), 0, 0)
        sliders.addWidget(self._global_force, 0, 1)
        sliders.addWidget(QLabel('Force variation:'), 1, 0)
        sliders.addWidget(self._force_var, 1, 1)
        sliders.addWidget(QLabel('Default force:'), 2, 0)
        sliders.addWidget(self._default_force, 2, 1)

        v = QVBoxLayout()
        v.addLayout(sliders)
        v.addWidget(self._force_env)
        v.addSpacing(8)
        v.addWidget(self._force_rel_env)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._global_force.set_au_id(au_id)
        self._force_var.set_au_id(au_id)
        self._default_force.set_au_id(au_id)
        self._force_env.set_au_id(au_id)
        self._force_rel_env.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._global_force.set_ui_model(ui_model)
        self._force_var.set_ui_model(ui_model)
        self._default_force.set_ui_model(ui_model)
        self._force_env.set_ui_model(ui_model)
        self._force_rel_env.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._force_rel_env.unregister_updaters()
        self._force_env.unregister_updaters()
        self._default_force.unregister_updaters()
        self._force_var.unregister_updaters()
        self._global_force.unregister_updaters()


