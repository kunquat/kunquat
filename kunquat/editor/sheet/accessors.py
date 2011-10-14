# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function

from PyQt4 import QtGui, QtCore

import kunquat.editor.timestamp as ts
import trigger


class FuncValidator(QtGui.QValidator):

    def __init__(self, func=lambda x: False):
        super(FuncValidator, self).__init__()
        self.isvalid = func

    def validate(self, field, pos):
        if self.isvalid(field):
            return QtGui.QValidator.Acceptable, pos
        return QtGui.QValidator.Intermediate, pos


class BoolEdit(QtCore.QObject):

    def __init__(self, parent=None):
        super(BoolEdit, self).__init__(parent)
        self.value = False

    def hasFocus(self):
        return True

    def hide(self):
        pass

    def setPalette(self, palette):
        pass

    def show(self):
        pass

    def setFocus(self):
        self.emit(QtCore.SIGNAL('returnPressed()'))

    def setGeometry(self, rect):
        pass

    def set_validator_func(self, func):
        pass

    def set_value(self, value):
        self.value = bool(value)

    def get_value(self):
        return not self.value


class StringEdit(QtGui.QLineEdit):

    def __init__(self, parent=None):
        super(StringEdit, self).__init__(parent)
        self.setValidator(FuncValidator())

    def keyPressEvent(self, ev):
        super(StringEdit, self).keyPressEvent(ev)
        if ev.key() != QtCore.Qt.Key_Escape:
            ev.accept()

    def set_validator_func(self, func):
        assert func
        self.setValidator(FuncValidator(func))

    def set_value(self, value):
        self.setText(value)

    def get_value(self):
        return str(self.text())


class TypeEdit(StringEdit):

    def __init__(self, parent=None):
        super(TypeEdit, self).__init__(parent)

    def get_value(self):
        return trigger.TriggerType(self.text())


class FloatEdit(StringEdit):

    def _float_validate(self, value):
        try:
            return self.isvalid(float(value))
        except ValueError:
            return False

    def set_validator_func(self, func):
        self.isvalid = func
        self.setValidator(FuncValidator(self._float_validate))

    def set_value(self, value):
        self.setText(str(value))

    def get_value(self):
        return float(self.text())


class NoteEdit(FloatEdit):

    def get_value(self):
        return trigger.Note(self.text())


class IntEdit(StringEdit):

    def _int_validate(self, value):
        try:
            return self.isvalid(int(value))
        except ValueError:
            return False

    def set_validator_func(self, func):
        self.isvalid = func
        self.setValidator(FuncValidator(self._int_validate))

    def set_value(self, value):
        self.setText(str(value))

    def get_value(self):
        return int(self.text())


class HitIndexEdit(IntEdit):

    def get_value(self):
        return trigger.HitIndex(self.text())


class TimestampEdit(StringEdit):

    def _ts_validate(self, value):
        try:
            return self.isvalid(ts.Timestamp(float(value)))
        except (TypeError, OverflowError, ValueError):
            return False

    def set_validator_func(self, func):
        self.isvalid = func
        self.setValidator(FuncValidator(self._ts_validate))

    def set_value(self, value):
        try:
            int_val = int(value)
            float_val = float(value)
        except ValueError:
            return
        self.setText(str(int_val if int_val == float_val else float_val))

    def get_value(self):
        return ts.Timestamp(float(self.text()))


