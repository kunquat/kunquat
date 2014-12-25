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
from addgen import AddGen
from samplegen import SampleGen
from unsupportedgen import UnsupportedGen
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper


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
        self._control_manager = None
        self._name = Name()
        self._gen_editor = None

        self._keyboard_mapper = KeyboardMapper()

        v = QVBoxLayout()
        v.setMargin(0)
        v.addWidget(self._name)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._name.set_ins_id(ins_id)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id
        self._name.set_gen_id(gen_id)

    def set_ui_model(self, ui_model):
        assert self._ins_id != None
        assert self._gen_id != None
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._control_manager = ui_model.get_control_manager()
        self._name.set_ui_model(ui_model)
        self._keyboard_mapper.set_ui_model(ui_model)
        self._updater.register_updater(self._perform_updates)
        self._set_type()

    def _set_type(self):
        assert self._gen_editor == None

        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        gentype = generator.get_type()

        cons = _gen_classes.get(gentype, UnsupportedGen)
        self._gen_editor = cons()
        self._gen_editor.set_ins_id(self._ins_id)
        self._gen_editor.set_gen_id(self._gen_id)
        self._gen_editor.set_ui_model(self._ui_model)
        self.layout().addWidget(self._gen_editor)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._keyboard_mapper.unregister_updaters()
        self._gen_editor.unregister_updaters()
        self._name.unregister_updaters()

    def _perform_updates(self, signals):
        pass

    def keyPressEvent(self, event):
        # TODO: This plays the complete instrument,
        #       change after adding generator jamming support
        module = self._ui_model.get_module()
        control_id = module.get_control_id_by_instrument_id(self._ins_id)
        if not control_id:
            return

        self._control_manager.set_control_id_override(control_id)
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()
        self._control_manager.set_control_id_override(None)

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()


