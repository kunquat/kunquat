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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from .name import Name


class InfoEditor(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._au_id = None
        self._name = Name()

        v = QVBoxLayout()
        v.addWidget(self._name, 0, Qt.AlignTop)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._name.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._name.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._name.unregister_updaters()


