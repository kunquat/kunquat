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

from generator.editor import Editor


class GeneratorWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._editor = Editor()

        v = QVBoxLayout()
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._editor.set_ins_id(ins_id)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id
        self._editor.set_gen_id(gen_id)

    def set_ui_model(self, ui_model):
        assert self._ins_id != None
        assert self._gen_id != None
        self._ui_model = ui_model
        self._editor.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_title()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._editor.unregister_updaters()

    def _perform_updates(self, signals):
        if 'signal_controls' in signals:
            self._update_title()

    def _update_title(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)

        parts = []
        ins_name = instrument.get_name()
        gen_name = generator.get_name()
        if gen_name:
            parts.append(gen_name)
        if ins_name:
            parts.append('[{}]'.format(ins_name))

        if parts:
            title = u'{} – Kunquat Tracker'.format(u' '.join(parts))
        else:
            title = u'Kunquat Tracker'
        self.setWindowTitle(title)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_generator(self._ins_id, self._gen_id)

    def sizeHint(self):
        return QSize(1024, 768)


