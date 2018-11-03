# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from kunquat.tracker.ui.qt import *

from kunquat.kunquat.limits import *
from kunquat.tracker.ui.views.headerline import HeaderLine
from .hitselector import HitSelector
from .audiounitupdater import AudioUnitUpdater


def _get_update_signal_type(au_id):
    return 'signal_hit_{}'.format(au_id)


class Hits(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

        self._hit_selector = AuHitSelector()
        self._hit_editor = HitEditor()

        self._hits_header = HeaderLine('Hits')
        self._properties_header = HeaderLine('Hit properties')

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._hits_header)
        v.addWidget(self._hit_selector)
        v.addWidget(self._properties_header)
        v.addWidget(self._hit_editor)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        if au.is_instrument():
            self.add_to_updaters(self._hit_selector, self._hit_editor)
            self.register_action('signal_style_changed', self._update_style)
            self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._hits_header.update_style(style_mgr)
        self._properties_header.update_style(style_mgr)
        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))


class AuHitSelector(HitSelector, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self.update_contents)
        self.create_layout(self._ui_model.get_typewriter_manager())
        self.update_contents()

        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

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
        self._updater.signal_update(self._get_update_signal_type())

    def _get_hit_name(self, index):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        hit = au.get_hit(index)
        return hit.get_name()

    def _update_style(self):
        self.update_style(self._ui_model.get_style_manager())


class HitEditor(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

        self._enabled = HitEnabled()
        self._name = HitName()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._enabled)
        v.addWidget(self._name)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._enabled, self._name)

        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))


def _get_current_hit(ui_model, au_id):
    module = ui_model.get_module()
    au = module.get_audio_unit(au_id)
    hit_base, hit_offset = au.get_edit_selected_hit_info()
    hit_index = hit_base + hit_offset
    return au.get_hit(hit_index)


class HitEnabled(QCheckBox, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        self.setText('Enabled')

    def _on_setup(self):
        self.register_action(
                _get_update_signal_type(self._au_id), self._update_existence)
        self.stateChanged.connect(self._change_existence)

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
        self._updater.signal_update(
            _get_update_signal_type(self._au_id),
            'signal_au_conns_hit_{}'.format(self._au_id),
            'signal_hits')


class HitName(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        self._edit = QLineEdit()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(QLabel('Name:'))
        h.addWidget(self._edit)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action(_get_update_signal_type(self._au_id), self._update_name)
        self._edit.textEdited.connect(self._change_name)

        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_name()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_name(self):
        hit = _get_current_hit(self._ui_model, self._au_id)

        old_block = self._edit.blockSignals(True)
        name = hit.get_name() or ''
        if name != str(self._edit.text()):
            self._edit.setText(name)
        self._edit.blockSignals(old_block)

    def _change_name(self, name):
        hit = _get_current_hit(self._ui_model, self._au_id)
        hit.set_name(name)
        self._updater.signal_update(
            _get_update_signal_type(self._au_id), 'signal_hits')


