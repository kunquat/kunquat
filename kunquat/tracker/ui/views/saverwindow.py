# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .saving import try_save_module
from .updater import Updater


class SaverWindow(QWidget):

    def __init__(self):
        super().__init__()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def keyPressEvent(self, event):
        if (event.modifiers() == Qt.ControlModifier) and (event.key() == Qt.Key_S):
            if self._ui_model.get_module().is_modified():
                try_save_module(self._ui_model)
        else:
            event.ignore()


