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

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.kunquat.limits import *


_BANK_SIZE = 32


class HitSelector(QWidget):

    def __init__(self):
        super().__init__()

        cb_info = {
            'get_hit_info': self._get_selected_hit_info,
            'set_hit_info': self._set_selected_hit_info,
            'get_hit_name': self._get_hit_name,
        }

        self._hit_bank_selector = HitBankSelector(cb_info)
        self._hit_keyboard_layout = HitKeyboardLayout(cb_info)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(4)
        v.addWidget(self._hit_bank_selector)
        v.addWidget(self._hit_keyboard_layout)
        self.setLayout(v)

    def create_layout(self, typewriter_manager):
        self._hit_bank_selector.create_layout()
        self._hit_keyboard_layout.create_layout(typewriter_manager)

    def update_contents(self):
        self._hit_bank_selector.update_contents()
        self._hit_keyboard_layout.update_contents()

    # Protected callbacks

    def _get_selected_hit_info(self):
        raise NotImplementedError

    def _set_selected_hit_info(self, hit_info):
        raise NotImplementedError

    def _get_hit_name(self, index):
        raise NotImplementedError


class HitKeyboardLayout(QWidget):

    _PAD = 35
    _ROW_LENGTHS = [9, 9, 7, 7]

    def __init__(self, cb_info):
        super().__init__()

        self._cb_info = cb_info
        self._get_selected_hit_info = cb_info['get_hit_info']
        self._set_selected_hit_info = cb_info['set_hit_info']

    def _get_row_index_offset_base(self, row_index):
        return sum(self._ROW_LENGTHS[row_index + 1:])

    def create_layout(self, typewriter_manager):
        assert typewriter_manager.get_row_count() >= 4

        assert sum(self._ROW_LENGTHS) == _BANK_SIZE

        rows = QVBoxLayout()
        rows.setContentsMargins(0, 0, 0, 0)
        rows.setSpacing(2)
        self.setLayout(rows)
        for row_index in range(min(4, typewriter_manager.get_row_count())):
            row = QHBoxLayout()
            row.setContentsMargins(0, 0, 0, 0)
            row.setSpacing(4)

            # Initial padding
            pad_px = self._PAD * typewriter_manager.get_pad_factor_at_row(row_index)
            pad = QWidget()
            pad.setFixedWidth(pad_px)
            row.addWidget(pad)

            # Buttons
            row_length = self._ROW_LENGTHS[row_index]
            row_index_offset_base = self._get_row_index_offset_base(row_index)
            for i in range(row_length):
                button = HitButton(self._cb_info, row_index_offset_base + i)
                row.addWidget(button)

            row.addStretch(1)
            rows.addLayout(row)

    def update_contents(self):
        hit_base, hit_offset = self._get_selected_hit_info()

        for row_index in range(self.layout().count()):
            row_index_offset_base = self._get_row_index_offset_base(row_index)
            row_layout = self.layout().itemAt(row_index).layout()
            for widget_index in range(1, row_layout.count() - 1):
                button_index = widget_index - 1
                button = row_layout.itemAt(widget_index).widget()
                button.set_index_base(hit_base)
                is_selected = (hit_offset == row_index_offset_base + button_index)
                button.set_pressed(is_selected)


class HitBankSelector(QWidget):

    def __init__(self, cb_info):
        super().__init__()

        self._cb_info = cb_info
        self._get_selected_hit_info = cb_info['get_hit_info']
        self._set_selected_hit_info = cb_info['set_hit_info']

    def create_layout(self):
        bank_count = int(math.ceil(HITS_MAX / float(_BANK_SIZE)))

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(QLabel('Hit banks:'))
        for i in range(bank_count):
            button = HitBankButton(self._cb_info, i)
            h.addWidget(button)
        h.addStretch(1)
        self.setLayout(h)

    def update_contents(self):
        hit_base, _ = self._get_selected_hit_info()

        for i in range(1, self.layout().count() - 1):
            button_index = i - 1
            button = self.layout().itemAt(i).widget()
            button.set_pressed((button_index * _BANK_SIZE) == hit_base)


class HitBankButton(QPushButton):

    def __init__(self, cb_info, index):
        super().__init__()

        self._get_selected_hit_info = cb_info['get_hit_info']
        self._set_selected_hit_info = cb_info['set_hit_info']

        self._index = index

        self.setCheckable(True)
        self.setText(str(self._index)) # TODO: maybe something more descriptive?

        QObject.connect(self, SIGNAL('clicked()'), self._select_bank)

    def set_pressed(self, pressed):
        old_block = self.blockSignals(True)
        self.setChecked(pressed)
        self.blockSignals(old_block)

    def _select_bank(self):
        hit_base, hit_offset = self._get_selected_hit_info()
        hit_base = self._index * _BANK_SIZE
        self._set_selected_hit_info((hit_base, hit_offset))


class HitButton(QPushButton):

    def __init__(self, cb_info, index_offset):
        super().__init__()

        self.setFixedSize(QSize(60, 60))
        self.setCheckable(True)

        self._get_selected_hit_info = cb_info['get_hit_info']
        self._set_selected_hit_info = cb_info['set_hit_info']
        self._get_hit_name = cb_info['get_hit_name']

        self._index_base = 0
        self._index_offset = index_offset

        self._index = QLabel()
        self._name = QLabel()

        self._index.setAlignment(Qt.AlignCenter)
        self._name.setAlignment(Qt.AlignCenter)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.setAlignment(Qt.AlignCenter)
        v.addWidget(self._index)
        v.addWidget(self._name)
        self.setLayout(v)

        self._update_text()

        QObject.connect(self, SIGNAL('clicked()'), self._select_hit)

    def _update_text(self):
        index = self._index_base + self._index_offset
        self._index.setText(str(index))
        self._name.setText(self._get_hit_name(index))

    def set_index_base(self, index_base):
        self._index_base = index_base
        self._update_text()

    def set_pressed(self, pressed):
        old_block = self.blockSignals(True)
        self.setChecked(pressed)
        self.blockSignals(old_block)

    def _select_hit(self):
        self._set_selected_hit_info((self._index_base, self._index_offset))


