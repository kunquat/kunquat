# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .utils import get_scaled_font


class NumberSlider(QWidget):

    numberChanged = Signal(float, name='numberChanged')

    def __init__(self, decimal_count, min_val, max_val, title='', width_txt=''):
        super().__init__()

        assert decimal_count >= 0

        self._min_val = min_val
        self._max_val = max_val
        self._width_txt = width_txt

        self._decimal_count = decimal_count
        self._scale = 10**decimal_count

        self._slider = QSlider()
        self._slider.setOrientation(Qt.Horizontal)
        self.set_range(min_val, max_val)

        self._value = QLabel()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(5)
        if title:
            h.addWidget(QLabel(title))
        h.addWidget(self._slider)
        h.addWidget(self._value)
        self.setLayout(h)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)

        self._slider.valueChanged.connect(self._number_changed)

        self.set_number(min_val)

    def update_style(self, style_mgr):
        fm = QFontMetrics(get_scaled_font(style_mgr, 1))
        if self._width_txt:
            width = fm.boundingRect(self._width_txt).width()
        else:
            val_fmt = self._get_val_fmt()
            width = max(fm.boundingRect(val_fmt.format(val)).width()
                    for val in (self._min_val, self._max_val))
        width += style_mgr.get_scaled_size(2)
        self._value.setFixedWidth(width)

        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def set_number(self, num):
        old_block = self._slider.blockSignals(True)
        int_val = int(round(num * self._scale))
        self._slider.setValue(int_val)
        self._slider.blockSignals(old_block)

        val_fmt = self._get_val_fmt()
        self._value.setText(val_fmt.format(num))

    def set_range(self, min_val, max_val):
        self._min_val = min_val
        self._max_val = max_val

        old_block = self._slider.blockSignals(True)
        self._slider.setMinimum(int(min_val * self._scale))
        self._slider.setMaximum(int(max_val * self._scale))
        self._slider.blockSignals(old_block)

    def _get_val_fmt(self):
        return '{{:.{}f}}'.format(self._decimal_count)

    def _number_changed(self, int_val):
        val = int_val / float(self._scale)
        self.numberChanged.emit(val)

    def sizeHint(self):
        return self._slider.sizeHint()


