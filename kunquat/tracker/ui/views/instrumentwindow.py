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


class InstrumentWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._ins_id = None

        v = QVBoxLayout()
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_ui_model(self, ui_model):
        assert self._ins_id != None
        self._ui_model = ui_model

    def unregister_updaters(self):
        pass

    def closeEvent(self, ev):
        ev.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_instrument(self._ins_id)


