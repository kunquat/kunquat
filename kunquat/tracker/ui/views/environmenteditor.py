# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import string

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.kunquat.limits import *


class EnvironmentEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._table = EnvEditorTable()
        self._var_adder = VariableAdder()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(self._table)
        v.addWidget(self._var_adder)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._table.set_ui_model(ui_model)
        self._var_adder.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._var_adder.unregister_updaters()
        self._table.unregister_updaters()


class EnvModel(QAbstractTableModel):

    def __init__(self):
        QAbstractTableModel.__init__(self)
        self._ui_model = None
        self._updater = None

        self._entries = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        module = self._ui_model.get_module()

        self._make_table(module)

    def _make_table(self, module):
        env = module.get_environment()
        names = env.get_var_names()

        entries = []
        for name in names:
            var_type = env.get_var_type(name)
            init_value = env.get_var_init_value(name)
            entry = (name, var_type, init_value)
            entries.append(entry)

        self._entries = entries

    # Qt interface

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return 4

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._entries)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            col = index.column()
            if (0 <= row < len(self._entries)) and (0 <= col < 3):
                if col == 0:
                    return QVariant(self._entries[row][col])

        return QVariant()

    def headerData(self, section, orientation, role):
        if (orientation == Qt.Horizontal) and (role == Qt.DisplayRole):
            headers = ['Name', 'Type', 'Initial value']
            if 0 <= section < len(headers):
                return QVariant(headers[section])

        return QVariant()

    def setData(self, index, value, role):
        if not index.isValid():
            return False

        module = self._ui_model.get_module()
        env = module.get_environment()

        if role == Qt.EditRole:
            row = index.row()
            col = index.column()
            if 0 <= row < len(self._entries):
                orig_entry = self._entries[row]
                if col == 0:
                    orig_name = orig_entry[0]
                    new_name = unicode(value.toString())
                    env.change_var_name(orig_name, new_name)
                    self._updater.signal_update(set(['signal_environment']))
                    return True

        return False

    def flags(self, index):
        default_flags = QAbstractItemModel.flags(self, index)
        if not index.isValid():
            return default_flags
        if 0 <= index.column() < 3:
            return Qt.ItemIsEditable | default_flags
        return default_flags


class EnvEditorTable(QTableView):

    def __init__(self):
        QTableView.__init__(self)
        self._ui_model = None
        self._updater = None
        self._env_model = None

        horiz_header = self.horizontalHeader()
        horiz_header.setVisible(True)
        horiz_header.setStretchLastSection(True)

        self.verticalHeader().setVisible(False)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_model()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_environment' in signals:
            self._update_model()

    def _update_model(self):
        self._env_model = EnvModel()
        self._env_model.set_ui_model(self._ui_model)
        self.setModel(self._env_model)


class VariableAdder(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_name = NewVarNameEditor()

        self._var_add_button = QPushButton()
        self._var_add_button.setText('Add new variable')
        self._var_add_button.setEnabled(False)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._var_name)
        h.addWidget(self._var_add_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_used_names()

        QObject.connect(
                self._var_name, SIGNAL('textChanged(QString)'), self._text_changed)
        QObject.connect(
                self._var_name, SIGNAL('returnPressed()'), self._add_new_var)
        QObject.connect(
                self._var_add_button, SIGNAL('clicked()'), self._add_new_var)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_environment' in signals:
            self._update_used_names()

    def _get_used_names(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        used_names = set(env.get_var_names())
        return used_names

    def _update_used_names(self):
        used_names = self._get_used_names()
        self._var_name.set_used_names(used_names)
        self._var_add_button.setEnabled(bool(self._var_name.text()))

    def _text_changed(self, text):
        text = unicode(text)
        used_names = self._get_used_names()
        self._var_add_button.setEnabled(bool(text) and (text not in used_names))

    def _add_new_var(self):
        text = unicode(self._var_name.text())
        assert text and (text not in self._get_used_names())

        module = self._ui_model.get_module()
        env = module.get_environment()
        env.add_var(text, float, 0.0)

        self._var_name.setText('')

        self._updater.signal_update(set(['signal_environment']))


class VarNameValidator(QValidator):

    def __init__(self, used_names):
        QValidator.__init__(self)
        self._used_names = used_names

    def validate(self, contents, pos):
        in_str = unicode(contents)
        if not in_str:
            return (QValidator.Intermediate, pos)

        allowed_init_chars = '_' + string.ascii_lowercase
        allowed_chars = allowed_init_chars + string.digits

        if in_str[0] not in allowed_init_chars:
            return (QValidator.Invalid, pos)

        if all(ch in allowed_chars for ch in in_str):
            if in_str not in self._used_names:
                return (QValidator.Acceptable, pos)
            else:
                return (QValidator.Intermediate, pos)

        return (QValidator.Invalid, pos)


class NewVarNameEditor(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self.setMaxLength(ENV_VAR_NAME_MAX - 1)
        self._validator = VarNameValidator(set())
        self.setValidator(self._validator)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)


