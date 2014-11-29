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

from instrument.editor import Editor


class InstrumentWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._updater = None
        self._editor = Editor()

        v = QVBoxLayout()
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._editor.set_ins_id(ins_id)

    def set_ui_model(self, ui_model):
        assert self._ins_id != None
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._editor.set_ui_model(ui_model)
        self._update_title()

    def unregister_updaters(self):
        self._editor.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_controls' in signals:
            self._update_title()

    def _update_title(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        name = instrument.get_name()
        if name:
            title = u'{} – Kunquat Tracker'.format(name)
        else:
            title = u'Kunquat Tracker'
        self.setWindowTitle(title)

    def closeEvent(self, ev):
        ev.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_instrument(self._ins_id)

    def sizeHint(self):
        return QSize(1024, 768)


