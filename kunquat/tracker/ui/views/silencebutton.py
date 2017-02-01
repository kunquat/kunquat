# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
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

from .updatingview import UpdatingView


class SilenceButton(QToolButton, UpdatingView):

    def __init__(self):
        super().__init__()
        self._playback_manager = None

        self.setText('Silence')
        self.setToolTip('Silence (Period)')
        self.setAutoRaise(True)

    def _on_setup(self):
        self._playback_manager = self._ui_model.get_playback_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('silence')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _clicked(self):
        self._playback_manager.stop_recording()
        self._ui_model.silence()


