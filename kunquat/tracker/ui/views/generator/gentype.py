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


STD_TYPES = [
    { 'type': u'add', 'name': 'Additive' },
    { 'type': u'pcm', 'name': 'Sample' },
]


class GenType(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._types = list(STD_TYPES)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_type_list()
        self._update_type()
        QObject.connect(self, SIGNAL('currentIndexChanged(int)'), self._select_type)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return '_'.join(('signal_gen_type', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_type()

    def _update_type_list(self):
        self.clear()
        for typeinfo in self._types:
            self.addItem(typeinfo['name'])

    def _get_type_index_by_type(self, gentype):
        for i, typeinfo in enumerate(self._types):
            if typeinfo['type'] == gentype:
                return i
        return -1

    def _update_type(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        gentype = generator.get_type()
        index = self._get_type_index_by_type(gentype)

        old_block = self.blockSignals(True)
        if index >= 0:
            self.setCurrentIndex(index)
        else:
            typeinfo = { 'type': gentype, 'name': '<unsupported>' }
            self._types.append(typeinfo)
            self.setCurrentIndex(len(self._types) - 1)
        self.blockSignals(old_block)

    def _select_type(self, index):
        typeinfo = self._types[index]
        gentype = typeinfo['type']
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        generator.reset_impl()
        generator.reset_conf()
        generator.set_type(gentype)
        self._updater.signal_update(set([self._get_update_signal_type()]))


