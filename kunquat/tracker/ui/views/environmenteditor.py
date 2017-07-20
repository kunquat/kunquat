# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.kunquat.limits import *
import kunquat.tracker.ui.model.tstamp as tstamp
from .editorlist import EditorList
from .headerline import HeaderLine
from .kqtcombobox import KqtComboBox
from .updater import Updater
from .varnamevalidator import VarNameValidator
from .varvalidators import *


class EnvironmentEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._vars = VariableList()

        self.add_to_updaters(self._vars)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Initial environment state'))
        v.addWidget(self._vars)
        self.setLayout(v)


class VariableList(EditorList, Updater):

    def __init__(self):
        super().__init__()
        self._var_names = None
        self._var_names_set = None

    def _on_setup(self):
        self.register_action('signal_environment', self._update_var_names)
        self._update_var_names()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _update_var_names(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        self._var_names = env.get_var_names()
        self._var_names_set = set(self._var_names)

        self.update_list()

    def _make_adder_widget(self):
        adder = VariableAdder()
        self.add_to_updaters(adder)
        return adder

    def _get_updated_editor_count(self):
        var_count = len(self._var_names)
        return var_count

    def _make_editor_widget(self, index):
        editor = VariableEditor()
        self.add_to_updaters(editor)
        return editor

    def _update_editor(self, index, editor):
        var_name = self._var_names[index]

        editor.set_var_name(var_name)
        editor.set_used_names(self._var_names_set)

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)


class VariableEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._var_name = None

        self._name_editor = VarNameEditor()
        self._type_editor = VarTypeEditor()
        self._value_editor = VarValueEditor()
        self._remove_button = VarRemoveButton()

        self.add_to_updaters(
                self._name_editor,
                self._type_editor,
                self._value_editor,
                self._remove_button)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._name_editor)
        h.addWidget(self._type_editor)
        h.addWidget(self._value_editor)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_var_name(self, name):
        self._var_name = name

        self._name_editor.set_var_name(name)
        self._type_editor.set_var_name(name)
        self._value_editor.set_var_name(name)
        self._remove_button.set_var_name(name)

    def set_used_names(self, used_names):
        self._name_editor.set_used_names(used_names)


class VarNameEditor(QLineEdit, Updater):

    def __init__(self):
        super().__init__()
        self._validator = None

        self.set_used_names(set())

        self._var_name = None

    def _on_setup(self):
        self.editingFinished.connect(self._change_name)

    def set_var_name(self, name):
        self._var_name = name

        old_block = self.blockSignals(True)
        self.setText(self._var_name)
        self.blockSignals(old_block)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)

    def _change_name(self):
        new_name = str(self.text())
        if new_name == self._var_name:
            return

        module = self._ui_model.get_module()
        env = module.get_environment()
        env.change_var_name(self._var_name, new_name)
        self._updater.signal_update('signal_environment')

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            event.accept()
            self.set_var_name(self._var_name)
        else:
            return super().keyPressEvent(event)


class VarTypeEditor(KqtComboBox, Updater):

    def __init__(self):
        super().__init__()
        self._var_name = None

    def _on_setup(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()

        var_type_names = {
                bool: 'Boolean',
                int: 'Integer',
                float: 'Floating point',
                tstamp.Tstamp: 'Timestamp',
            }

        for t in var_types:
            type_name = var_type_names[t]
            self.addItem(type_name)

        self.currentIndexChanged.connect(self._change_type)

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()
        var_type = env.get_var_type(self._var_name)
        var_type_index = var_types.index(var_type)

        old_block = self.blockSignals(True)
        self.setCurrentIndex(var_type_index)
        self.blockSignals(old_block)

    def _change_type(self, index):
        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()
        env.change_var_type(self._var_name, var_types[index])
        self._updater.signal_update('signal_environment')


class VarValueEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._var_name = None

        self._editors = {
            bool:           QCheckBox(),
            int:            QLineEdit(),
            float:          QLineEdit(),
            tstamp.Tstamp:  QLineEdit(),
        }

        self._editors[bool].setText(' ') # work around broken clickable region

        self._editors[int].setValidator(IntValidator())
        self._editors[float].setValidator(FloatValidator())
        self._editors[tstamp.Tstamp].setValidator(FloatValidator())

    def _on_setup(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()

        s = QStackedLayout()
        for t in var_types:
            s.addWidget(self._editors[t])
        self.setLayout(s)

        self._editors[bool].stateChanged.connect(self._change_bool_value)
        self._editors[int].editingFinished.connect(self._change_int_value)
        self._editors[float].editingFinished.connect(self._change_float_value)
        self._editors[tstamp.Tstamp].editingFinished.connect(self._change_tstamp_value)

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        env = module.get_environment()

        var_types = env.get_var_types()
        var_type = env.get_var_type(self._var_name)
        var_type_index = var_types.index(var_type)
        self.layout().setCurrentIndex(var_type_index)

        var_value = env.get_var_init_value(self._var_name)

        editor = self._editors[var_type]
        old_block = editor.blockSignals(True)
        if var_type == bool:
            editor.setCheckState(Qt.Checked if var_value else Qt.Unchecked)
        elif var_type == int:
            editor.setText(str(var_value))
        elif var_type == float:
            editor.setText(str(var_value))
        elif var_type == tstamp.Tstamp:
            editor.setText(str(float(var_value)))
        else:
            assert False
        editor.blockSignals(old_block)

    def _change_value(self, new_value):
        module = self._ui_model.get_module()
        env = module.get_environment()
        env.change_var_init_value(self._var_name, new_value)
        self._updater.signal_update('signal_environment')

    def _change_bool_value(self, new_state):
        new_value = (new_state == Qt.Checked)
        self._change_value(new_value)

    def _change_int_value(self):
        new_str = self._editors[int].text()
        new_value = int(new_str)
        self._change_value(new_value)

    def _change_float_value(self):
        new_str = self._editors[float].text()
        new_value = float(new_str)
        self._change_value(new_value)

    def _change_tstamp_value(self):
        new_str = self._editors[tstamp.Tstamp].text()
        new_value = tstamp.Tstamp(float(new_str))
        self._change_value(new_value)


class VarRemoveButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self._var_name = None

        self.setToolTip('Remove')

        self.setStyleSheet('padding: 0 -2px;')

    def _on_setup(self):
        icon_bank = self._ui_model.get_icon_bank()
        self.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))

        self.clicked.connect(self._remove)

    def set_var_name(self, name):
        self._var_name = name

    def _remove(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        env.remove_var(self._var_name)
        self._updater.signal_update('signal_environment')


class VariableAdder(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._var_name = NewVarNameEditor()

        self._var_add_button = QPushButton()
        self._var_add_button.setText('Add new variable')
        self._var_add_button.setEnabled(False)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._var_name)
        h.addWidget(self._var_add_button)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_environment', self._update_used_names)

        self._update_used_names()

        self._var_name.textChanged.connect(self._text_changed)
        self._var_name.returnPressed.connect(self._add_new_var)
        self._var_add_button.clicked.connect(self._add_new_var)

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
        text = str(text)
        used_names = self._get_used_names()
        self._var_add_button.setEnabled(bool(text) and (text not in used_names))

    def _add_new_var(self):
        text = str(self._var_name.text())
        assert text and (text not in self._get_used_names())

        module = self._ui_model.get_module()
        env = module.get_environment()
        env.add_var(text, float, 0.0)

        self._var_name.setText('')

        self._updater.signal_update('signal_environment')


class NewVarNameEditor(QLineEdit):

    def __init__(self):
        super().__init__()
        self.setMaxLength(VAR_NAME_MAX - 1)
        self._validator = VarNameValidator(set())
        self.setValidator(self._validator)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)


