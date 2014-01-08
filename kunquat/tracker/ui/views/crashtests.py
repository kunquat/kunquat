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

import threading

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class ViewFailure(QPushButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Trigger view failure')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _clicked(self):
        return self.undefined


class CrashTests(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._view_failure = ViewFailure()

        v = QHBoxLayout()
        v.addWidget(self._view_failure)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._view_failure.set_ui_model(ui_model)


