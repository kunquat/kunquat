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


_BANK_SIZE = 32


def _get_update_signal_type(au_id):
    return 'signal_hit_{}'.format(au_id)


class Hits(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None

        self._hit_bank_selector = HitBankSelector()
        self._hit_selector = HitSelector()
        self._hit_editor = HitEditor()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(HeaderLine('Hit selector'))
        v.addWidget(self._hit_bank_selector)
        v.addWidget(self._hit_selector)
        v.addWidget(HeaderLine('Hit editor'))
        v.addWidget(self._hit_editor, 1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._hit_bank_selector.set_au_id(self._au_id)
        self._hit_selector.set_au_id(self._au_id)
        self._hit_editor.set_au_id(self._au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        if au.is_instrument():
            self._hit_bank_selector.set_ui_model(ui_model)
            self._hit_selector.set_ui_model(ui_model)
            self._hit_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        if au.is_instrument():
            self._hit_editor.unregister_updaters()
            self._hit_selector.unregister_updaters()
            self._hit_bank_selector.unregister_updaters()


class HitBankSelector(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        bank_count = int(math.ceil(HITS_MAX / float(_BANK_SIZE)))

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(QLabel('Hit banks:'))
        for i in xrange(bank_count):
            button = HitBankButton(i)
            button.set_au_id(self._au_id)
            button.set_ui_model(self._ui_model)
            h.addWidget(button)
        h.addStretch(1)
        self.setLayout(h)

        self._updater.register_updater(self._perform_updates)

        self._update_buttons()

    def unregister_updaters(self):
        for i in xrange(1, self.layout().count() - 1):
            button = self.layout().itemAt(i).widget()
            button.unregister_updaters()

        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if _get_update_signal_type(self._au_id) in signals:
            self._update_buttons()

    def _update_buttons(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        hit_base, _ = au.get_edit_selected_hit_info()

        for i in xrange(1, self.layout().count() - 1):
            button_index = i - 1
            button = self.layout().itemAt(i).widget()
            button.set_pressed((button_index * _BANK_SIZE) == hit_base)


class HitBankButton(QPushButton):

    def __init__(self, index):
        QPushButton.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._index = index

        self.setCheckable(True)
        self.setText(str(self._index)) # TODO: maybe something more descriptive?

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._select_bank)

    def unregister_updaters(self):
        pass

    def set_pressed(self, pressed):
        old_block = self.blockSignals(True)
        self.setChecked(pressed)
        self.blockSignals(old_block)

    def _select_bank(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        hit_base, hit_offset = au.get_edit_selected_hit_info()
        hit_base = self._index * _BANK_SIZE
        au.set_edit_selected_hit_info(hit_base, hit_offset)

        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class HitSelector(QWidget):

    _PAD = 35
    _ROW_LENGTHS = [9, 9, 7, 7]

    def __init__(self):
        QWidget.__init__(self)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def _get_row_index_offset_base(self, row_index):
        return sum(self._ROW_LENGTHS[row_index + 1:])

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        # Layout
        typewriter_manager = ui_model.get_typewriter_manager()
        assert typewriter_manager.get_row_count() >= 4

        assert sum(self._ROW_LENGTHS) == _BANK_SIZE

        rows = QVBoxLayout()
        rows.setMargin(0)
        rows.setSpacing(2)
        self.setLayout(rows)
        for row_index in xrange(min(4, typewriter_manager.get_row_count())):
            row = QHBoxLayout()
            row.setMargin(0)
            row.setSpacing(4)

            # Initial padding
            pad_px = self._PAD * typewriter_manager.get_pad_factor_at_row(row_index)
            pad = QWidget()
            pad.setFixedWidth(pad_px)
            row.addWidget(pad)

            # Buttons
            row_length = self._ROW_LENGTHS[row_index]
            row_index_offset_base = self._get_row_index_offset_base(row_index)
            for i in xrange(row_length):
                button = HitButton(row_index_offset_base + i)
                button.set_au_id(self._au_id)
                button.set_ui_model(self._ui_model)
                row.addWidget(button)

            row.addStretch(1)
            rows.addLayout(row)

        self._update_buttons()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if _get_update_signal_type(self._au_id) in signals:
            self._update_buttons()

    def _update_buttons(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        hit_base, hit_offset = au.get_edit_selected_hit_info()

        for row_index in xrange(self.layout().count()):
            row_index_offset_base = self._get_row_index_offset_base(row_index)
            row_layout = self.layout().itemAt(row_index).layout()
            for widget_index in xrange(1, row_layout.count() - 1):
                button_index = widget_index - 1
                button = row_layout.itemAt(widget_index).widget()
                button.set_index_base(hit_base)
                is_selected = (hit_offset == row_index_offset_base + button_index)
                button.set_pressed(is_selected)


class HitButton(QPushButton):

    def __init__(self, index_offset):
        QPushButton.__init__(self)
        self._au_id = None
        self._ui_model = None

        self.setFixedSize(QSize(60, 60))
        self.setCheckable(True)

        self._index_base = 0
        self._index_offset = index_offset

        self._update_text()

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._select_hit)

    def unregister_updaters(self):
        pass

    def _update_text(self):
        self.setText(str(self._index_base + self._index_offset))

    def set_index_base(self, index_base):
        self._index_base = index_base
        self._update_text()

    def set_pressed(self, pressed):
        old_block = self.blockSignals(True)
        self.setChecked(pressed)
        self.blockSignals(old_block)

    def _select_hit(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_edit_selected_hit_info(self._index_base, self._index_offset)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class HitEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._enabled = HitEnabled()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(self._enabled)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._enabled.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._enabled.set_ui_model(ui_model)

    def unregister_updaters(self):
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
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


