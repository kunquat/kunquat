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


class SilenceButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._playback_manager = None

        self.setText('Silence')
        self.setToolTip('Silence (Period)')
        self.setAutoRaise(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._playback_manager = ui_model.get_playback_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('silence')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        self._playback_manager.stop_recording()
        self._ui_model.silence()


