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

from kunquat.tracker.ui.model.trigger import Trigger


class RestButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None
        self._sheet_manager = None

        self.setText(u'══')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._sheet_manager = ui_model.get_sheet_manager()

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        trigger = Trigger('n-', None)
        self._sheet_manager.insert_trigger(trigger)


