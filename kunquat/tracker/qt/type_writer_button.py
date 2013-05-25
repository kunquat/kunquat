# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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

class TypeWriterButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._instrument = None

        self.setText('100c')
        self.setAutoRaise(True)

    def _play_sound(self):
        self._instrument.set_active_note(0, 100)

    def set_ui_model(self, ui_model):
        module = ui_model.get_module()
        self._instrument = module.get_instrument(0)
        QObject.connect(self, SIGNAL('clicked()'),
                        self._play_sound)

    def _update(self):
        pass

