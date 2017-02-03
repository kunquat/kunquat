# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2017
#          Toni Ruottu, Finland 2014
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

from .updater import Updater


class AboutButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('About')

    def _on_setup(self):
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_about()


