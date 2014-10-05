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


class Name(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._updater = None

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_name()
        QObject.connect(self, SIGNAL('textEdited(QString)'), self._text_edited)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_controls' in signals:
            self._update_name()

    def _update_name(self):
        old_block = self.blockSignals(True)
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        self.setText(instrument.get_name())
        self.blockSignals(old_block)

    def _text_edited(self, text):
        text = unicode(text)
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        instrument.set_name(text)
        self._updater.signal_update(set(['signal_controls']))


