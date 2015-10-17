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

from itertools import izip

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

def _get_rebuild_signal_type(au_id):
    return 'signal_au_control_vars_rebuild_{}'.format(au_id)


class Subdevices(QSplitter):

    def __init__(self):
        QSplitter.__init__(self, Qt.Vertical)

        self._conns_editor = ConnectionsEditor()
        self._control_vars = ControlVariables()

        self.addWidget(self._conns_editor)
        self.addWidget(self._control_vars)

        self.setStretchFactor(0, 4)

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
        force_rebuild = _get_rebuild_signal_type(self._au_id) in signals
        if _get_update_signal_type(self._au_id) in signals:
            self._update_var_names(force_rebuild)

    def _update_var_names(self, force_rebuild=False):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        self._var_names = au.get_control_var_names()
        self._var_names_set = set(self._var_names)

        self.update_list(force_rebuild)

    def _make_adder_widget(self):
        adder = ControlVariableAdder()
        adder.set_au_id(self._au_id)
        adder.set_ui_model(self._ui_model)
        return adder

    def _get_updated_editor_count(self):
        var_count = len(self._var_names)
        return var_count

    def _make_editor_widget(self, index):
        var_name = self._var_names[index]

        editor = ControlVariableEditor()
        editor.set_au_id(self._au_id)
        editor.set_context(var_name)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        var_name = self._var_names[index]

        editor.set_context(var_name)
        editor.set_used_names(self._var_names_set)

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()


class ControlVariableEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

        self._expander = ControlVariableTypeExpander()
        self._name_editor = ControlVariableNameEditor()
        self._type_editor = ControlVariableTypeEditor()
        self._init_value_editor = ControlVariableInitValueEditor()
        self._ext_editor = ControlVariableExtEditor()
        self._remove_button = ControlVariableRemoveButton()
        self._bindings = ControlVariableBindings()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._name_editor)
        #h.addWidget(self._type_editor)
        h.addWidget(self._init_value_editor)
        h.addWidget(self._ext_editor)
        h.addWidget(self._remove_button)

        g = QGridLayout()
        g.setMargin(0)
        g.setSpacing(0)
        g.addWidget(self._expander, 0, 0)
        g.addLayout(h, 0, 1)
        g.addWidget(self._bindings, 1, 1)
        self.setLayout(g)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._name_editor.set_au_id(au_id)
        self._type_editor.set_au_id(au_id)
        self._init_value_editor.set_au_id(au_id)
        self._ext_editor.set_au_id(au_id)
        self._remove_button.set_au_id(au_id)
        self._bindings.set_au_id(au_id)

    def set_context(self, context):
        self._context = context
        self._name_editor.set_context(context)
        self._type_editor.set_context(context)
        self._init_value_editor.set_context(context)
        self._ext_editor.set_context(context)
        self._remove_button.set_context(context)
        self._bindings.set_context(context)

        if self._ui_model:
            self._update_contents()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._name_editor.set_ui_model(ui_model)
        self._type_editor.set_ui_model(ui_model)
        self._init_value_editor.set_ui_model(ui_model)
        self._ext_editor.set_ui_model(ui_model)
        self._remove_button.set_ui_model(ui_model)
        self._bindings.set_ui_model(ui_model)

        QObject.connect(self._expander, SIGNAL('clicked(bool)'), self._toggle_expand)

        self._update_contents()

    def unregister_updaters(self):
        self._bindings.unregister_updaters()
        self._remove_button.unregister_updaters()
        self._ext_editor.unregister_updaters()
        self._init_value_editor.unregister_updaters()
        self._type_editor.unregister_updaters()
        self._name_editor.unregister_updaters()

    def set_used_names(self, used_names):
        self._name_editor.set_used_names(used_names)

    def _update_contents(self):
        var_name = self._context

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        is_expanded = au.is_control_var_expanded(var_name)

        old_block = self._expander.blockSignals(True)
        self._expander.setChecked(is_expanded)
        self._expander.blockSignals(old_block)

    def _toggle_expand(self, expand):
        var_name = self._context

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_control_var_expanded(var_name, expand)

        if expand:
            self._bindings.setVisible(True)
        else:
            self._updater.signal_update(set([
                _get_update_signal_type(self._au_id),
                _get_rebuild_signal_type(self._au_id)]))


class ControlVariableTypeExpander(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self.setCheckable(True)

    def sizeHint(self):
        sh = QPushButton.sizeHint(self)
        return QSize(sh.height(), sh.height())

    def paintEvent(self, event):
        painter = QPainter(self)

        colour = self.palette().color(QPalette.Text)
        painter.setPen(colour)
        painter.setBrush(colour)

        center_x = self.width() // 2
        center_y = self.height() // 2

        triangle_extent = 5

        painter.translate(QPoint(center_x, center_y))
        if self.isChecked():
            painter.drawPolygon(
                    QPoint(-triangle_extent, 0),
                    QPoint(0, triangle_extent),
                    QPoint(triangle_extent, 0))
        else:
            painter.drawPolygon(
                    QPoint(0, -triangle_extent),
                    QPoint(triangle_extent, 0),
                    QPoint(0, triangle_extent))


class NameEditor(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None
        self._validator = None

        self.set_used_names(set())

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_context(self, context):
        self._context = context

        if self._ui_model:
            self._update_contents()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('editingFinished()'), self._change_name_handler)

        self._update_contents()

    def unregister_updaters(self):
        pass

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)

    def _change_name_handler(self):
        new_name = unicode(self.text())
        self._change_name(new_name)

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            event.accept()
            self.set_context(self._context)
        else:
            return QLineEdit.keyPressEvent(self, event)

    # Protected interface

    def _update_contents(self):
        raise NotImplementedError

    def _change_name(self, new_name):
        raise NotImplementedError


class ControlVariableNameEditor(NameEditor):

    def __init__(self):
        NameEditor.__init__(self)

    def _update_contents(self):
        old_block = self.blockSignals(True)
        self.setText(self._context)
        self.blockSignals(old_block)

    def _change_name(self, new_name):
        if new_name == self._context:
            return

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.change_control_var_name(self._context, new_name)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class ControlVariableTypeEditor(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_context(self, context):
        self._context = context

        if self._ui_model:
            self._update_contents()

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

        self._update_contents()

    def unregister_updaters(self):
        pass

    def _update_contents(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()
        var_type = au.get_control_var_type(self._context)
        var_type_index = var_types.index(var_type)

        old_block = self.blockSignals(True)
        self.setCurrentIndex(var_type_index)
        self.blockSignals(old_block)

    def _change_type(self, index):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()
        au.change_control_var_type(self._context, var_types[index])
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class ControlVariableValueEditor(QWidget):

    def __init__(self, label):
        QWidget.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

        # TODO: The QStackedLayout causes flickering during update for some reason, fix

        '''
        self._editor_layout = None
        '''

        self._editors = None

        self._editors = {
            float:  QLineEdit(),
        }

        self._editors[float].setValidator(FloatValidator())

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(QLabel(label))
        h.addWidget(self._editors[float])
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_context(self, context):
        self._context = context

        if self._ui_model:
            self._update_contents()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()

        '''
        self._editor_layout = QStackedLayout()
        for t in var_types:
            self._editor_layout.addWidget(self._editors[t])
        self.layout().addLayout(self._editor_layout)
        '''

        QObject.connect(
                self._editors[float],
                SIGNAL('editingFinished()'),
                self._change_float_value)

        self._update_contents()

    def unregister_updaters(self):
        pass

    def _change_value(self, new_value):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        self._set_value(au, new_value)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))

    def _change_float_value(self):
        new_qstring = self._editors[float].text()
        new_value = float(unicode(new_qstring))
        self._change_value(new_value)

    # Protected interface

    def _update_contents(self):
        # TODO: this callback contains stuff that doesn't belong here, fix
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_types = au.get_control_var_types()
        var_type = au.get_control_var_type(self._context)
        var_type_index = var_types.index(var_type)
        '''
        self._editor_layout.setCurrentIndex(var_type_index)
        '''

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


class ControlVariableInitValueEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Initial value:')

    def _get_value(self, au):
        return au.get_control_var_init_value(self._context)

    def _set_value(self, au, new_value):
        au.change_control_var_init_value(self._context, new_value)


class ControlVariableMinValueEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Minimum value:')

    def _get_value(self, au):
        return au.get_control_var_min_value(self._context)

    def _set_value(self, au, new_value):
        au.change_control_var_min_value(self._context, new_value)


class ControlVariableMaxValueEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Maximum value:')

    def _get_value(self, au):
        return au.get_control_var_max_value(self._context)

    def _set_value(self, au, new_value):
        au.change_control_var_max_value(self._context, new_value)


class ControlVariableExtEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

        self._editors = {
            float: ControlVariableFloatExtEditor(),
        }

        # TODO: The QStackedLayout causes flickering during update for some reason, fix

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._editors[float])
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

        for editor in self._editors.itervalues():
            editor.set_au_id(au_id)

    def set_context(self, context):
        self._context = context

        if self._ui_model:
            self._update_contents()
        else:
            for editor in self._editors.itervalues():
                editor.set_context(self._context)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        for editor in self._editors.itervalues():
            editor.set_ui_model(ui_model)

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        var_types = au.get_control_var_types()

        '''
        s = QStackedLayout()
        for t in var_types:
            s.addWidget(self._editors[t])
        self.setLayout(s)
        '''

        self._update_contents()

    def unregister_updaters(self):
        for editor in self._editors.itervalues():
            editor.unregister_updaters()

    def _update_contents(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_types = au.get_control_var_types()
        var_type = au.get_control_var_type(self._context)
        var_type_index = var_types.index(var_type)
        '''
        self.layout().setCurrentIndex(var_type_index)
        '''

        editor = self._editors[var_type]
        editor.set_context(self._context)


class ControlVariableFloatExtEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

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

    def set_context(self, context):
        self._context = context

        if self._ui_model:
            self._update_contents()
        else:
            self._min_value_editor.set_context(self._context)
            self._max_value_editor.set_context(self._context)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._min_value_editor.set_ui_model(ui_model)
        self._max_value_editor.set_ui_model(ui_model)

        self._update_contents()

    def unregister_updaters(self):
        self._max_value_editor.unregister_updaters()
        self._min_value_editor.unregister_updaters()

    def _update_contents(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_type = au.get_control_var_type(self._context)
        if var_type != float:
            self.setEnabled(False)
            return

        self.setEnabled(True)

        self._min_value_editor.set_context(self._context)
        self._max_value_editor.set_context(self._context)


class Adder(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._name_editor = NewNameEditor()

        self._add_button = QPushButton()
        self._add_button.setText(self._get_add_text())
        self._add_button.setEnabled(False)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._name_editor)
        h.addWidget(self._add_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_used_names()

        QObject.connect(
                self._name_editor, SIGNAL('textChanged(QString)'), self._text_changed)
        QObject.connect(
                self._name_editor, SIGNAL('returnPressed()'), self._add_new)
        QObject.connect(
                self._add_button, SIGNAL('clicked()'), self._add_new)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if _get_update_signal_type(self._au_id) in signals:
            self._update_used_names()

    def _update_used_names(self):
        used_names = self._get_used_names()
        self._name_editor.set_used_names(used_names)

        is_add_allowed = bool(self._name_editor.text())
        self._add_button.setEnabled(is_add_allowed)

    def _text_changed(self, text_qstring):
        text = unicode(text_qstring)
        used_names = self._get_used_names()

        is_add_allowed = bool(text) and (text not in used_names)
        self._add_button.setEnabled(is_add_allowed)

    def _add_new(self):
        text = unicode(self._name_editor.text())
        assert text and (text not in self._get_used_names())

        self._add_new_entry(text)

        self._name_editor.setText('')

    # Protected interface

    def _get_add_text(self):
        raise NotImplementedError

    def _get_used_names(self):
        raise NotImplementedError

    def _add_new_entry(self, name):
        raise NotImplementedError


class NewNameEditor(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self.setMaxLength(ENV_VAR_NAME_MAX - 1)
        self._validator = VarNameValidator(set())
        self.setValidator(self._validator)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)


class ControlVariableAdder(Adder):

    def __init__(self):
        Adder.__init__(self)

    def _get_add_text(self):
        return 'Add new variable'

    def _get_used_names(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        used_names = set(au.get_control_var_names())
        return used_names

    def _add_new_entry(self, name):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        au.add_control_var_float(name, 0.0, 0.0, 1.0)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class RemoveButton(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

        self.setToolTip('Remove')

        self.setStyleSheet('padding: 0 -2px;')

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_context(self, context):
        self._context = context

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        icon_bank = ui_model.get_icon_bank()
        self.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))

        QObject.connect(self, SIGNAL('clicked()'), self._remove)

    def unregister_updaters(self):
        pass

    # Protected interface

    def _get_button_text(self):
        raise NotImplementedError

    def _remove(self):
        raise NotImplementedError


class ControlVariableRemoveButton(RemoveButton):

    def __init__(self):
        RemoveButton.__init__(self)

    def _remove(self):
        var_name = self._context

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.remove_control_var(var_name)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class ControlVariableBindings(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

        self._adder = BindTargetAdder()

        self.setVisible(False)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(self._adder)
        self.setLayout(v)

    def _get_adder(self):
        layout = self.layout()
        return layout.itemAt(layout.count() - 1).widget()

    def _get_widgets(self):
        for i in xrange(self.layout().count()):
            yield self.layout().itemAt(i).widget()

    def _get_editors(self):
        for i in xrange(self.layout().count() - 1):
            yield self.layout().itemAt(i).widget()

    def set_au_id(self, au_id):
        self._au_id = au_id
        for widget in self._get_widgets():
            widget.set_au_id(au_id)

    def set_context(self, context):
        self._context = context

        if self._ui_model:
            self._update_contents()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        for widget in self._get_widgets():
            widget.set_ui_model(ui_model)

        self._update_contents()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        for widget in self._get_widgets():
            widget.unregister_updaters()

    def _update_contents(self):
        var_name = self._context

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        self.setVisible(au.is_control_var_expanded(var_name))

        binding_targets = au.get_control_var_binding_targets(var_name)

        cur_binding_count = self.layout().count() - 1

        # Add missing editors
        layout = self.layout()
        for i in xrange(cur_binding_count, len(binding_targets)):
            editor = BindTargetEditor()
            editor.set_au_id(self._au_id)
            editor.set_ui_model(self._ui_model)
            layout.insertWidget(i, editor)

        self._get_adder().set_context(var_name)

        for target_info, editor in izip(binding_targets, self._get_editors()):
            target_dev_id, target_var_name = target_info
            context = (var_name, target_dev_id, target_var_name)
            editor.set_context(context)

    def _perform_updates(self, signals):
        if _get_update_signal_type(self._au_id) in signals:
            pass


class BindTargetEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._target_dev_selector = BindTargetDeviceSelector()
        self._name_editor = BindTargetNameEditor()
        self._map_to_min_editor = BindTargetMapToMinEditor()
        self._map_to_max_editor = BindTargetMapToMaxEditor()
        self._remove_button = BindTargetRemoveButton()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._target_dev_selector)
        h.addWidget(self._name_editor)
        h.addWidget(self._map_to_min_editor)
        h.addWidget(self._map_to_max_editor)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._target_dev_selector.set_au_id(au_id)
        self._name_editor.set_au_id(au_id)
        self._map_to_min_editor.set_au_id(au_id)
        self._map_to_max_editor.set_au_id(au_id)
        self._remove_button.set_au_id(au_id)

    def set_context(self, context):
        self._target_dev_selector.set_context(context)
        self._name_editor.set_context(context)
        self._map_to_min_editor.set_context(context)
        self._map_to_max_editor.set_context(context)
        self._remove_button.set_context(context)

    def set_ui_model(self, ui_model):
        self._target_dev_selector.set_ui_model(ui_model)
        self._name_editor.set_ui_model(ui_model)
        self._map_to_min_editor.set_ui_model(ui_model)
        self._map_to_max_editor.set_ui_model(ui_model)
        self._remove_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._remove_button.unregister_updaters()
        self._map_to_max_editor.unregister_updaters()
        self._map_to_min_editor.unregister_updaters()
        self._name_editor.unregister_updaters()
        self._target_dev_selector.unregister_updaters()

    def set_used_names(self, used_names):
        self._name_editor.set_used_names(used_names)


class BindTargetDeviceSelector(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._au_id = None
        self._context = None
        self._ui_model = None
        self._updater = None

        self.setSizeAdjustPolicy(QComboBox.AdjustToContents)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_context(self, context):
        self._context = context

        if self._ui_model:
            self._update_contents()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self, SIGNAL('currentIndexChanged(int)'), self._change_target_dev_id)

        self._update_contents()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_internal_dev_ids(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        for au_id in au.get_au_ids():
            yield au_id

        for proc_id in au.get_processor_ids():
            yield proc_id

    def _get_internal_dev(self, dev_id):
        module = self._ui_model.get_module()

        if 'proc' in dev_id:
            dev_id_parts = dev_id.split('/')
            internal_au_id_parts = dev_id_parts[:-1]
            internal_au_id = '/'.join(internal_au_id_parts)
            internal_au = module.get_audio_unit(internal_au_id)
            dev = internal_au.get_processor(dev_id)
        else:
            dev = module.get_audio_unit(dev_id)

        return dev

    def _update_contents(self):
        if not self._context:
            return

        target_dev_id = self._context[1]

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        old_block = self.blockSignals(True)

        self.clear()
        for dev_id in self._get_internal_dev_ids():
            dev = self._get_internal_dev(dev_id)

            if dev.get_existence():
                name = dev.get_name() or '-'
                cur_target_dev_id = dev_id.split('/')[-1]
                self.addItem(name, QVariant(cur_target_dev_id))

        selected_index = self.findData(QVariant(target_dev_id))
        if selected_index >= 0:
            self.setCurrentIndex(selected_index)

        self.blockSignals(old_block)

    def _perform_updates(self, signals):
        update_signals = set([
            '_'.join(('signal_connections', self._au_id)),
            'signal_controls'])
        if not signals.isdisjoint(update_signals):
            self._update_contents()

    def _change_target_dev_id(self, index):
        if index < 0:
            return

        new_target_dev_id = str(self.itemData(index).toString())

        var_name, target_dev_id, target_var_name = self._context

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.change_control_var_binding_target_dev(
                var_name, target_dev_id, target_var_name, new_target_dev_id)

        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class BindTargetNameEditor(NameEditor):

    def __init__(self):
        NameEditor.__init__(self)

    def _change_name(self, new_name):
        var_name, target_dev_id, target_var_name = self._context

        if new_name == target_var_name:
            return

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.change_control_var_binding_target_name(
                var_name, target_dev_id, target_var_name, new_name)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))

    def _update_contents(self):
        if not self._context:
            return

        target_name = self._context[2]

        old_block = self.blockSignals(True)
        self.setText(target_name)
        self.blockSignals(old_block)


class BindTargetMapToMinEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Map minimum to:')

    def _get_value(self, au):
        var_name, target_dev_id, target_var_name = self._context
        return au.get_control_var_binding_map_to_min(
                var_name, target_dev_id, target_var_name)

    def _set_value(self, au, new_value):
        var_name, target_dev_id, target_var_name = self._context
        au.change_control_var_binding_map_to_min(
                var_name, target_dev_id, target_var_name, new_value)

    def _update_contents(self):
        if not self._context:
            return

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_value = self._get_value(au)

        editor = self._editors[float]
        old_block = editor.blockSignals(True)
        editor.setText(unicode(var_value))
        editor.blockSignals(old_block)


class BindTargetMapToMaxEditor(ControlVariableValueEditor):

    def __init__(self):
        ControlVariableValueEditor.__init__(self, 'Map maximum to:')

    def _get_value(self, au):
        var_name, target_dev_id, target_var_name = self._context
        return au.get_control_var_binding_map_to_max(
                var_name, target_dev_id, target_var_name)

    def _set_value(self, au, new_value):
        var_name, target_dev_id, target_var_name = self._context
        au.change_control_var_binding_map_to_max(
                var_name, target_dev_id, target_var_name, new_value)

    def _update_contents(self):
        if not self._context:
            return

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_value = self._get_value(au)

        editor = self._editors[float]
        old_block = editor.blockSignals(True)
        editor.setText(unicode(var_value))
        editor.blockSignals(old_block)


class BindTargetRemoveButton(RemoveButton):

    def __init__(self):
        RemoveButton.__init__(self)

    def _remove(self):
        var_name, target_dev_id, target_var_name = self._context
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.remove_control_var_binding(var_name, target_dev_id, target_var_name)
        self._updater.signal_update(set([
            _get_update_signal_type(self._au_id),
            _get_rebuild_signal_type(self._au_id)]))


class BindTargetAdder(Adder):

    def __init__(self):
        Adder.__init__(self)

        self._context = None

    def set_context(self, context):
        self._context = context

    def _get_add_text(self):
        return 'Add new binding'

    def _get_used_names(self):
        if not self._context:
            return set()

        var_name = self._context

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        # Escape if we are being removed
        if var_name not in au.get_control_var_names():
            return set()

        used_names = set(au.get_control_var_binding_targets(var_name))
        return used_names

    def _add_new_entry(self, name):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        var_name = self._context

        au_ids = au.get_au_ids()
        if au_ids:
            dev_id = min(au_ids)
        else:
            proc_ids = au.get_processor_ids()
            if proc_ids:
                dev_id = min(proc_ids)
            else:
                assert False, 'Trying to add binding target without internal devices'

        internal_dev_id = dev_id.split('/')[-1]

        au.add_control_var_binding_float(var_name, internal_dev_id, name, 0.0, 1.0)
        self._updater.signal_update(set([_get_update_signal_type(self._au_id)]))


class NewBindTargetNameEditor(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self.setMaxLength(ENV_VAR_NAME_MAX - 1)
        self._validator = VarNameValidator(set())
        self.setValidator(self._validator)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)


