# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2014-2016
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


class PlayButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self.setText('Play')
        self.setToolTip('Play (Comma)')
        self.setAutoRaise(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('play')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        QObject.connect(self, SIGNAL('clicked()'),
                        self._ui_model.play)

    def unregister_updaters(self):
        pass

    def _update(self):
        pass


