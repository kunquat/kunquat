# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.kunquat.limits import *
from kunquat.tracker.ui.views.headerline import HeaderLine
from .hitselector import HitSelector


def _get_update_signal_type(au_id):
    return 'signal_hit_{}'.format(au_id)


class Hits(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None

        self._hit_selector = AuHitSelector()
        self._hit_editor = HitEditor()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(HeaderLine('Hits'))
        v.addWidget(self._hit_selector)
        v.addWidget(HeaderLine('Hit properties'))
        v.addWidget(self._hit_editor)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._hit_selector.set_au_id(self._au_id)
        self._hit_editor.set_au_id(self._au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        if au.is_instrument():
            self._hit_selector.set_ui_model(ui_model)
            self._hit_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        if au.is_instrument():
            self._hit_editor.unregister_updaters()
            self._hit_selector.unregister_updaters()


class AuHitSelector(HitSelector):

    def __init__(self):
        HitSelector.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self.create_layout(self._ui_model.get_typewriter_manager())
        self.update_contents()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self.update_contents()

    def _get_update_signal_type(self):
        return _get_update_signal_type(self._au_id)

    def _get_selected_hit_info(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au.get_edit_selected_hit_info()

    def _set_selected_hit_info(self, hit_info):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        hit_base, hit_offset = hit_info
        au.set_edit_selected_hit_info(hit_base, hit_offset)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class HitEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._enabled = HitEnabled()
        self._name = HitName()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(self._enabled)
        v.addWidget(self._name)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._enabled.set_au_id(au_id)
        self._name.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._enabled.set_ui_model(ui_model)
        self._name.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._name.unregister_updaters()
        self._enabled.unregister_updaters()


def _get_current_hit(ui_model, au_id):
    module = ui_model.get_module()
    au = module.get_audio_unit(au_id)
    hit_base, hit_offset = au.get_edit_selected_hit_info()
    hit_index = hit_base + hit_offset
    return au.get_hit(hit_index)


class HitEnabled(QCheckBox):

    def __init__(self):
        QCheckBox.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self.setText('Enabled')

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('stateChanged(int)'), self._change_existence)

        self._update_existence()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if _get_update_signal_type(self._au_id) in signals:
            self._update_existence()

    def _update_existence(self):
        hit = _get_current_hit(self._ui_model, self._au_id)

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if hit.get_existence() else Qt.Unchecked)
        self.blockSignals(old_block)

    def _change_existence(self, state):
        existence = (state == Qt.Checked)
        hit = _get_current_hit(self._ui_model, self._au_id)
        hit.set_existence(existence)
        self._updater.signal_update(set([
            _get_update_signal_type(self._au_id),
            'signal_au_conns_hit_{}'.format(self._au_id),
            'signal_hits']))


class HitName(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._edit = QLineEdit()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(QLabel('Name:'))
        h.addWidget(self._edit)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._edit, SIGNAL('textEdited(QString)'), self._change_name)

        self._update_name()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if _get_update_signal_type(self._au_id) in signals:
            self._update_name()

    def _update_name(self):
        hit = _get_current_hit(self._ui_model, self._au_id)

        old_block = self._edit.blockSignals(True)
        name = hit.get_name() or u''
        if name != unicode(self._edit.text()):
            self._edit.setText(name)
        self._edit.blockSignals(old_block)

    def _change_name(self, text_qstring):
        name = unicode(text_qstring)

        hit = _get_current_hit(self._ui_model, self._au_id)
        hit.set_name(name)
        self._updater.signal_update(set([
            _get_update_signal_type(self._au_id), 'signal_hits']))


