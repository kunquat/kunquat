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

from name import Name
from gentype import GenType
from addgen import AddGen
from samplegen import SampleGen
from unsupportedgen import UnsupportedGen


_gen_classes = {
    'add': AddGen,
    'pcm': SampleGen,
}


class Editor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._name = Name()
        self._gen_type = GenType()
        self._gen_editor = UnsupportedGen()

        v = QVBoxLayout()
        v.setMargin(0)
        v.addWidget(self._name)
        v.addWidget(self._gen_type)
        v.addWidget(self._gen_editor)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._name.set_ins_id(ins_id)
        self._gen_type.set_ins_id(ins_id)
        self._gen_editor.set_ins_id(ins_id)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id
        self._name.set_gen_id(gen_id)
        self._gen_type.set_gen_id(gen_id)
        self._gen_editor.set_gen_id(gen_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._name.set_ui_model(ui_model)
        self._gen_type.set_ui_model(ui_model)
        self._gen_editor.set_ui_model(ui_model)
        self._updater.register_updater(self._perform_updates)
        self._update_type()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._gen_editor.unregister_updaters()
        self._gen_type.unregister_updaters()
        self._name.unregister_updaters()

    def _get_type_update_signal_type(self):
        return '_'.join(('signal_gen_type', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        if self._get_type_update_signal_type() in signals:
            self._update_type()

    def _update_type(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        gentype = generator.get_type()

        self.layout().removeWidget(self._gen_editor)
        self._gen_editor.unregister_updaters()
        self._gen_editor.deleteLater()
        self._gen_editor = None

        cons = _gen_classes.get(gentype, UnsupportedGen)
        self._gen_editor = cons()
        self._gen_editor.set_ins_id(self._ins_id)
        self._gen_editor.set_gen_id(self._gen_id)
        self._gen_editor.set_ui_model(self._ui_model)
        self.layout().addWidget(self._gen_editor)


