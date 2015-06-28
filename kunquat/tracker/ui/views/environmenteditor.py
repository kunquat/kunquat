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
        self._vars = VariableList()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(self._vars)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._vars.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._vars.unregister_updaters()


class VariableListContainer(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.setSizeConstraint(QLayout.SetMinimumSize)
        self.setLayout(v)


class VariableListArea(QScrollArea):

    def __init__(self):
        QScrollArea.__init__(self)

    def do_width_hack(self):
        widget = self.widget()
        if widget:
            widget.setFixedWidth(self.width() - self.verticalScrollBar().width() - 5)

    def resizeEvent(self, event):
        self.do_width_hack()


class VariableList(QWidget):

    def __init__(self):
        QTableView.__init__(self)
        self._ui_model = None
        self._updater = None

        self._area = VariableListArea()

        self._area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self._area.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._area)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self._disconnect_editors()
        self._updater.unregister_updater(self._perform_updates)

    def _init_container(self):
        self._area.setWidget(VariableListContainer())
        adder = VariableAdder()
        adder.set_ui_model(self._ui_model)
        self._area.widget().layout().addWidget(adder)

    def _disconnect_editors(self):
        layout = self._area.widget().layout()
        for i in xrange(layout.count()):
            widget = layout.itemAt(i).widget()
            widget.unregister_updaters()

    def _perform_updates(self, signals):
        if 'signal_environment' in signals:
            self._update_all()

    def _update_all(self):
        module = self._ui_model.get_module()
        env = module.get_environment()

        var_names = env.get_var_names()
        var_count = len(var_names)

        if not self._area.widget():
            self._init_container()

        layout = self._area.widget().layout()

        if var_count < layout.count() - 1:
            self._disconnect_editors()
            self._init_container()
            layout = self._area.widget().layout()

        # Create new variable editors
        for i in xrange(layout.count() - 1, var_count):
            editor = VariableEditor()
            editor.set_ui_model(self._ui_model)
            layout.insertWidget(i, editor)

        # Update editor contents
        for i, name in enumerate(var_names):
            editor = layout.itemAt(i).widget()
            editor.set_var_name(name)

        self._area.do_width_hack()


class VariableEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._var_name = None

        self._name_editor = VarNameEditor()
        self._remove_button = VarRemoveButton()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._name_editor)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        self._name_editor.set_ui_model(ui_model)
        self._remove_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._remove_button.unregister_updaters()
        self._name_editor.unregister_updaters()

    def set_var_name(self, name):
        self._var_name = name

        self._name_editor.set_var_name(name)
        self._remove_button.set_var_name(name)


class VarNameEditor(QLineEdit):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_name = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('editingFinished()'), self._change_name)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

        old_block = self.blockSignals(True)
        self.setText(self._var_name)
        self.blockSignals(old_block)

    def _change_name(self):
        new_name = unicode(self.text())
        if new_name == self._var_name:
            return

        module = self._ui_model.get_module()
        env = module.get_environment()
        env.change_var_name(self._var_name, new_name)
        self._updater.signal_update(set(['signal_environment']))

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            event.accept()
            self.set_var_name(self._var_name)
        else:
            return QLineEdit.keyPressEvent(self, event)


class VarRemoveButton(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_name = None

        self.setStyleSheet('padding: 0 -2px;')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        icon_bank = ui_model.get_icon_bank()
        self.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))

        QObject.connect(self, SIGNAL('clicked()'), self._remove)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

    def _remove(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        env.remove_var(self._var_name)
        self._updater.signal_update(set(['signal_environment']))


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


