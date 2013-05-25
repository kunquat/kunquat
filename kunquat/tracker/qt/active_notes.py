# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
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


class ActiveNotes(QLabel):

    def __init__(self, instrument_number):
        QLabel.__init__(self)
        self._instrument_number = instrument_number
        self._instrument = None

    def set_ui_model(self, ui_model):
        module = ui_model.get_module()
        self._instrument = module.get_instrument(self._instrument_number)
        self._instrument.register_updater(self.update)

    def update(self):
        notes = self._instrument.get_active_notes()
        text = 'instrument{}: {}'.format(self._instrument_number, str(notes))
        self.setText(text)

