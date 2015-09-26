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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.kunquat.limits import *
import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.views.connectionseditor import ConnectionsEditor
from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.varnamevalidator import VarNameValidator
from kunquat.tracker.ui.views.varvalidators import *


def _get_update_signal_type(au_id):
    return 'signal_au_control_vars_{}'.format(au_id)


class Subdevices(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._conns_editor = ConnectionsEditor()
        self._control_vars = ControlVariables()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(self._conns_editor, 4)
        v.addWidget(self._control_vars, 1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._conns_editor.set_au_id(au_id)
        self._control_vars.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._conns_editor.set_ui_model(ui_model)
        self._control_vars.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._control_vars.unregister_updaters()
        self._conns_editor.unregister_updaters()


class ControlVariables(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._var_list = ControlVariableList()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Control variables'))
        v.addWidget(self._var_list)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._var_list.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._var_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._var_list.unregister_updaters()


class ControlVariableList(EditorList):

    def __init__(self):
        EditorList.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._var_names = None
        self._var_names_set = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_var_names()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if _get_update_signal_type(self._au_id) in signals:
            self._update_var_names()

    def _update_var_names(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        self._var_names = au.get_control_var_names()
        self._var_names_set = set(self._var_names)

        self.update_list()

    def _make_adder_widget(self):
        adder = ControlVariableAdder()
        adder.set_au_id(self._au_id)
        adder.set_ui_model(self._ui_model)
        return adder

    def _get_updated_editor_count(self):
        var_count = len(self._var_names)
        return var_count

    def _make_editor_widget(self, index):
        editor = ControlVariableEditor()
        editor.set_au_id(self._au_id)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        var_name = self._var_names[index]

        editor.set_var_name(var_name)
        editor.set_used_names(self._var_names_set)

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()


class ControlVariableEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._name_editor = ControlVariableNameEditor()
        self._type_editor = ControlVariableTypeEditor()
        self._init_value_editor = ControlVariableInitValueEditor()
        self._ext_editor = ControlVariableExtEditor()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._name_editor)
        #h.addWidget(self._type_editor)
        h.addWidget(self._init_value_editor)
        h.addWidget(self._ext_editor)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._name_editor.set_au_id(au_id)
        self._type_editor.set_au_id(au_id)
        self._init_value_editor.set_au_id(au_id)
        self._ext_editor.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._name_editor.set_ui_model(ui_model)
        self._type_editor.set_ui_model(ui_model)
        self._init_value_editor.set_ui_model(ui_model)
        self._ext_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ext_editor.unregister_updaters()
        self._init_value_editor.unregister_updaters()
        self._type_editor.unregister_updaters()
        self._name_editor.unregister_updaters()

    def set_var_name(self, name):
        self._name_editor.set_var_name(name)
        self._type_editor.set_var_name(name)
        self._init_value_editor.set_var_name(name)
        self._ext_editor.set_var_name(name)

    def set_used_names(self, used_names):
        self._name_editor.set_used_names(used_names)


class ControlVariableNameEditor(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None
        self._validator = None

        self.set_used_names(set())

        self._var_name = None

    def set_au_id(self, au_id):
        self._au_id = au_id

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

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)

    def _change_name(self):
        new_name = unicode(self.text())
        if new_name == self._var_name:
            return

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.change_control_var_name(self._var_name, new_name)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            event.accept()
            self.set_var_name(self._var_name)
        else:
            return QLineEdit.keyPressEvent(self, event)


class ControlVariableTypeEditor(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._var_name = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()

        var_type_names = {
            bool: 'Boolean',
            int: 'Integer',
            float: 'Floating point',
            tstamp.Tstamp: 'Timestamp',
        }

        for t in var_types:
            type_name = var_type_names[t]
            self.addItem(type_name)

        QObject.connect(self, SIGNAL('currentIndexChanged(int)'), self._change_type)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()
        var_type = au.get_control_var_type(self._var_name)
        var_type_index = var_types.index(var_type)

        old_block = self.blockSignals(True)
        self.setCurrentIndex(var_type_index)
        self.blockSignals(old_block)

    def _change_type(self, index):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()
        au.change_control_var_type(self._var_name, var_types[index])
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class ControlVariableValueEditor(QWidget):

    def __init__(self, label):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._var_name = None

        self._editor_layout = None

        self._editors = {
            float:  QLineEdit(),
        }

        self._editors[float].setValidator(FloatValidator())

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(QLabel(label))
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()

        self._editor_layout = QStackedLayout()
        for t in var_types:
            self._editor_layout.addWidget(self._editors[t])
        self.layout().addLayout(self._editor_layout)

        QObject.connect(
                self._editors[float],
                SIGNAL('editingFinished()'),
                self._change_float_value)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_types = au.get_control_var_types()
        var_type = au.get_control_var_type(self._var_name)
        var_type_index = var_types.index(var_type)
        self._editor_layout.setCurrentIndex(var_type_index)

        var_value = self._get_value(au)

        editor = self._editors[var_type]
        old_block = editor.blockSignals(True)
        if var_type == float:
            editor.setText(unicode(var_value))
        else:
            assert False
        editor.blockSignals(old_block)

    def _get_value(self, au):
        raise NotImplementedError

    def _set_value(self, au, new_value):
        raise NotImplementedError

    def _change_value(self, new_value):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        self._set_value(au, new_value)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))

    def _change_float_value(self):
        new_qstring = self._editors[float].text()
        new_value = float(unicode(new_qstring))
        self._change_value(new_value)


class ControlVariableInitValueEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Initial value:')

    def _get_value(self, au):
        return au.get_control_var_init_value(self._var_name)

    def _set_value(self, au, new_value):
        au.change_control_var_init_value(self._var_name, new_value)


class ControlVariableMinValueEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Minimum value:')

    def _get_value(self, au):
        return au.get_control_var_min_value(self._var_name)

    def _set_value(self, au, new_value):
        au.change_control_var_min_value(self._var_name, new_value)


class ControlVariableMaxValueEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Maximum value:')

    def _get_value(self, au):
        return au.get_control_var_max_value(self._var_name)

    def _set_value(self, au, new_value):
        au.change_control_var_max_value(self._var_name, new_value)


class ControlVariableExtEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._var_name = None

        self._editors = {
            float: ControlVariableFloatExtEditor(),
        }

    def set_au_id(self, au_id):
        self._au_id = au_id

        for editor in self._editors.itervalues():
            editor.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        for editor in self._editors.itervalues():
            editor.set_ui_model(ui_model)

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()

        s = QStackedLayout()
        for t in var_types:
            s.addWidget(self._editors[t])
        self.setLayout(s)

    def unregister_updaters(self):
        for editor in self._editors.itervalues():
            editor.unregister_updaters()

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_types = au.get_control_var_types()
        var_type = au.get_control_var_type(self._var_name)
        var_type_index = var_types.index(var_type)
        self.layout().setCurrentIndex(var_type_index)

        editor = self._editors[var_type]
        editor.set_var_name(name)


class ControlVariableFloatExtEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._var_name = None

        self._min_value_editor = ControlVariableMinValueEditor()
        self._max_value_editor = ControlVariableMaxValueEditor()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._min_value_editor)
        h.addWidget(self._max_value_editor)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._min_value_editor.set_au_id(au_id)
        self._max_value_editor.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._min_value_editor.set_ui_model(ui_model)
        self._max_value_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._max_value_editor.unregister_updaters()
        self._min_value_editor.unregister_updaters()

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_type = au.get_control_var_type(self._var_name)
        if var_type != float:
            self.setEnabled(False)
            return

        self.setEnabled(True)

        self._min_value_editor.set_var_name(name)
        self._max_value_editor.set_var_name(name)


class ControlVariableAdder(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._var_name = NewControlVarNameEditor()

        self._var_add_button = QPushButton()
        self._var_add_button.setText('Add new variable')
        self._var_add_button.setEnabled(False)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._var_name)
        h.addWidget(self._var_add_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

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
        if _get_update_signal_type(self._au_id) in signals:
            self._update_used_names()

    def _get_used_names(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        used_names = set(au.get_control_var_names())
        return used_names

    def _update_used_names(self):
        used_names = self._get_used_names()
        self._var_name.set_used_names(used_names)

        is_add_allowed = bool(self._var_name.text())
        self._var_add_button.setEnabled(is_add_allowed)

    def _text_changed(self, text_qstring):
        text = unicode(text_qstring)
        used_names = self._get_used_names()

        is_add_allowed = bool(text) and (text not in used_names)
        self._var_add_button.setEnabled(is_add_allowed)

    def _add_new_var(self):
        text = unicode(self._var_name.text())
        assert text and (text not in self._get_used_names())

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.add_control_var_float(text, 0.0, 0.0, 1.0)

        self._var_name.setText('')

        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class NewControlVarNameEditor(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self.setMaxLength(ENV_VAR_NAME_MAX - 1)
        self._validator = VarNameValidator(set())
        self.setValidator(self._validator)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)


class ControlVariableBindings(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._bind_list = ControlVariableBindingList()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(4)
        v.addWidget(HeaderLine('Bindings'))
        v.addWidget(self._bind_list)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def unregister_updaters(self):
        pass


class ControlVariableBindingList(EditorList):

    def __init__(self):
        EditorList.__init__(self)


