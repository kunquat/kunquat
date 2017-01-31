# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
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

from .iacontrols import IAControls
from .updatingview import UpdatingView


class IAWindow(QWidget, UpdatingView):

    def __init__(self):
        super().__init__()
        self._ia_controls = IAControls()

        self.add_updating_child(self._ia_controls)

        self.setWindowTitle('Interactivity controls')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._ia_controls)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_interactivity_controls()

    def sizeHint(self):
        return QSize(320, 360)


