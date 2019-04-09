# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2018-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math
import string

from kunquat.tracker.ui.qt import *


class VarPrecSpinBox(QAbstractSpinBox):

    valueChanged = Signal(float, name='valueChanged')

    def __init__(self, step_decimals=0, max_decimals=6):
        super().__init__()

        assert step_decimals >= 0
        assert max_decimals >= 1

        self._step_decimals = step_decimals
        self._max_decimals = max(step_decimals, max_decimals)

        self._value = 0
        self._min_value = 0
        self._max_value = 100 * 10**self._max_decimals

        self._minimum_width = None

        line_edit = self.lineEdit()
        line_edit.setText(str(self._value))
        line_edit.textChanged.connect(self._change_value)

        self.editingFinished.connect(self._minimise_string)

    def _to_internal(self, value):
        return int(round(value * 10**self._max_decimals))

    def _from_internal(self, value):
        return value / 10**self._max_decimals

    def _clamp_value(self):
        self._value = min(max(self._min_value, self._value), self._max_value)

    def setMinimum(self, minimum):
        self._min_value = self._to_internal(minimum)
        self._max_value = max(self._min_value, self._max_value)
        self._clamp_value()

    def setMaximum(self, maximum):
        self._max_value = self._to_internal(maximum)
        self._min_value = min(self._min_value, self._max_value)
        self._clamp_value()

    def setRange(self, minimum, maximum):
        assert minimum <= maximum
        self._min_value = self._to_internal(minimum)
        self._max_value = self._to_internal(maximum)
        self._clamp_value()

    def setDecimals(self, decimals):
        assert decimals >= 1
        if decimals == self._max_decimals:
            return

        if decimals < self._max_decimals:
            factor = 10**(self._max_decimals - decimals)
            scale = lambda x: int(round(x / factor))
        else:
            factor = 10**(decimals - self._max_decimals)
            scale = lambda x: x * factor

        self._value = scale(self._value)
        self._min_value = scale(self._min_value)
        self._max_value = scale(self._max_value)

        self._max_decimals = decimals

    def _set_value(self, value):
        old_block = self.blockSignals(True)
        self._value = self._to_internal(value)
        self._clamp_value()
        new_text = self.text()
        if new_text != self.lineEdit().text():
            self.lineEdit().setText(new_text)
        self.blockSignals(old_block)
        self.update()

    def setValue(self, value):
        self._set_value(value)
        self.valueChanged.emit(self.value())

    def text(self):
        is_negative = (self._value < 0)
        abs_str = str(abs(self._value))

        digit_count = len(abs_str)
        if digit_count <= self._max_decimals:
            abs_str = '0.' + ('0' * (self._max_decimals - digit_count)) + abs_str
        else:
            whole_count = digit_count - self._max_decimals
            abs_str = abs_str[:whole_count] + '.' + abs_str[whole_count:]

        abs_str = abs_str.rstrip('0').rstrip('.')
        if not abs_str:
            abs_str = '0'

        return '-' + abs_str if is_negative else abs_str

    def _minimise_string(self):
        line_edit = self.lineEdit()
        old_text = line_edit.text()
        new_text = self.text()
        if new_text != old_text:
            old_block = line_edit.blockSignals(True)
            line_edit.setText(self.text())
            line_edit.blockSignals(old_block)

    def fixup(self, in_str):
        if not in_str:
            in_str = self.text()
        return super().fixup(in_str)

    def value(self):
        return self._from_internal(self._value)

    def _str_to_internal(self, in_str):
        if not in_str:
            raise ValueError('No input string')

        is_negative = (in_str[0] == '-')
        abs_str = in_str[1:] if is_negative else in_str
        parts = abs_str.split('.')
        if len(parts) > 2:
            raise ValueError('Illegal number format')
        if not all(all(c in string.digits for c in p) for p in parts):
            raise ValueError('Illegal number format')

        if len(parts) == 1:
            parts.append('0' * self._max_decimals)
        else:
            decimal_count = len(parts[1])
            parts[1] += '0' * (self._max_decimals - decimal_count)
            if len(parts[1]) > self._max_decimals:
                raise ValueError('Too many decimals')

        whole, decimals = (int(p) for p in parts)

        value = whole * 10**self._max_decimals + decimals
        if is_negative:
            value = -value

        if not (self._min_value <= value <= self._max_value):
            raise ValueError('Number outside range')

        return value

    def _change_value(self, value_str):
        try:
            self._value = self._str_to_internal(value_str)
        except ValueError:
            return

        self.valueChanged.emit(self.value())

    def stepEnabled(self):
        if self.wrapping():
            return QAbstractSpinBox.StepUpEnabled | QAbstractSpinBox.StepDownEnabled

        flags = QAbstractSpinBox.StepNone
        if self._value > self._min_value:
            flags |= QAbstractSpinBox.StepDownEnabled
        if self._value < self._max_value:
            flags |= QAbstractSpinBox.StepUpEnabled

        return flags

    def stepBy(self, steps):
        step_size = 10**(self._max_decimals - self._step_decimals)
        self._value += steps * step_size

        interval = self._max_value - self._min_value
        if self.wrapping() and interval > 0:
            if self._value < self._min_value:
                excess = (self._min_value - self._value) % interval
                self._value = self._max_value - excess
            elif self._value > self._max_value:
                excess = (self._value - self._max_value) % interval
                self._value = self._min_value + excess

        self._clamp_value()

        line_edit = self.lineEdit()
        old_block = line_edit.blockSignals(True)
        line_edit.setText(self.text())
        line_edit.blockSignals(old_block)

        self.valueChanged.emit(self.value())
        self.update()

    def _can_be_extended_to_valid(self, test_str):
        if not test_str:
            return True

        if not all(c in (string.digits + '-.') for c in test_str):
            return False
        if '-' in test_str[1:]:
            return False

        test_is_negative = (test_str[0] == '-')
        if test_is_negative and (self._min_value >= 0):
            return False

        def test_with_decimal_point(test_str):
            if self._max_decimals == 0:
                return False

            parts = test_str.split('.')
            if len(parts) > 2:
                return False
            whole_str, dec_str = parts
            if len(dec_str) > self._max_decimals:
                return False

            # See if the extension(s) with smallest absolute value is acceptable
            whole_abs_str = whole_str.lstrip('-')
            whole_abs = int(whole_abs_str) if whole_abs_str else 0
            dec_abs = int(dec_str) if dec_str else 0
            min_result_abs = whole_abs * 10**self._max_decimals + dec_abs
            test_value = -min_result_abs if test_is_negative else min_result_abs

            def test_fixed_sign(test_value):
                if test_value >= 0:
                    if self._max_value < 0:
                        return False
                    bound = self._max_value
                else:
                    if self._min_value >= 0:
                        return False
                    bound = abs(self._min_value)
                    test_value = abs(test_value)
                return (test_value <= bound)

            if not test_fixed_sign(test_value):
                if test_is_negative:
                    return False
                elif not test_fixed_sign(-test_value):
                    return False

            return True

        if '.' in test_str:
            return test_with_decimal_point(test_str)

        # No decimal point so place it as far left as possible
        test_abs_str = test_str.lstrip('-')
        split_point = max(0, len(test_abs_str) - self._max_decimals)
        test_str = test_abs_str[:split_point] + '.' + test_abs_str[split_point:]
        if test_is_negative:
            test_str = '-' + test_str
        return test_with_decimal_point(test_str)

    def validate(self, in_str, pos):
        maybe_intermediate = (
                QValidator.Intermediate
                if self._can_be_extended_to_valid(in_str)
                else QValidator.Invalid)

        if not in_str:
            return (QValidator.Intermediate, in_str, pos)

        try:
            value = self._str_to_internal(in_str)
        except ValueError:
            return (maybe_intermediate, in_str, pos)

        if not (self._min_value <= value <= self._max_value):
            return (maybe_intermediate, in_str, pos)

        return (QValidator.Acceptable, in_str, pos)

    def update_style(self, style_mgr):
        self._minimum_width = None

    def minimumSizeHint(self):
        if not self._minimum_width:
            def get_longest_str(bound):
                return '.' + str(bound)
            min_longest = get_longest_str(self._min_value)
            max_longest = get_longest_str(self._max_value)
            longest = min_longest if len(min_longest) > len(max_longest) else max_longest

            fm = self.fontMetrics()
            width = fm.width(longest) + 2
            height = self.lineEdit().minimumSizeHint().height()

            opt = QStyleOptionSpinBox()
            self.initStyleOption(opt)
            self._minimum_width = self.style().sizeFromContents(
                    QStyle.CT_SpinBox, opt, QSize(width, height), self).expandedTo(
                            QApplication.globalStrut()).width()

        height = super().minimumSizeHint().height()
        return QSize(self._minimum_width, height)


