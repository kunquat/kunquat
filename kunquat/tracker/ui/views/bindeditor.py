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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.kunquat.events as events
from kunquat.kunquat.limits import *
from .editorlist import EditorList
from .headerline import HeaderLine


class BindEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._bind_list = BindList()
        self._source_event = SourceEventSelector()
        self._constraints = Constraints()
        self._targets = Targets()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Event bindings'))
        v.addWidget(self._bind_list)
        v.addWidget(self._source_event)
        v.addWidget(self._constraints)
        v.addWidget(self._targets)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._bind_list.set_ui_model(ui_model)
        self._source_event.set_ui_model(ui_model)
        self._constraints.set_ui_model(ui_model)
        self._targets.set_ui_model(ui_model)

        self._update_editor_enabled()

    def unregister_updaters(self):
        self._targets.unregister_updaters()
        self._constraints.unregister_updaters()
        self._source_event.unregister_updaters()
        self._bind_list.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_editor_enabled()

    def _update_editor_enabled(self):
        bindings = self._ui_model.get_module().get_bindings()
        enable_editor = bindings.has_selected_binding()
        self._source_event.setEnabled(enable_editor)
        self._constraints.setEnabled(enable_editor)
        self._targets.setEnabled(enable_editor)


class BindListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add binding')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove binding')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_binding)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_binding)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_enabled()

    def _update_enabled(self):
        bindings = self._ui_model.get_module().get_bindings()
        remove_enabled = ((bindings.get_count() > 0) and
                (bindings.get_selected_binding_index() != None))
        self._remove_button.setEnabled(remove_enabled)

    def _add_binding(self):
        bindings = self._ui_model.get_module().get_bindings()
        bindings.add_binding()
        self._updater.signal_update(set(['signal_bind']))

    def _remove_binding(self):
        bindings = self._ui_model.get_module().get_bindings()
        selected_index = bindings.get_selected_binding_index()
        if selected_index != None:
            bindings.remove_binding(selected_index)
            bindings.set_selected_binding_index(None)
            self._updater.signal_update(set(['signal_bind']))


class BindListModel(QAbstractListModel):

    def __init__(self):
        QAbstractListModel.__init__(self)
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._make_items()

    def unregister_updaters(self):
        pass

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        bindings = self._ui_model.get_module().get_bindings()
        bs = (bindings.get_binding(i) for i in xrange(bindings.get_count()))
        self._items = [(i, b.get_source_event()) for (i, b) in enumerate(bs)]

    def get_index(self, list_index):
        return self.createIndex(list_index, 0, self._items[list_index])

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                _, event_name = self._items[row]
                return QVariant(event_name)

        return QVariant()

    def headerData(self, section, orientation, role):
        return QVariant()


class BindListView(QListView):

    def __init__(self):
        QListView.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item:
            index, _ = item
            bindings = self._ui_model.get_module().get_bindings()
            bindings.set_selected_binding_index(index)
            self._updater.signal_update(set(['signal_bind']))

    def setModel(self, model):
        QListView.setModel(self, model)

        selection_model = self.selectionModel()

        bindings = self._ui_model.get_module().get_bindings()
        selected_index = bindings.get_selected_binding_index()
        if selected_index != None:
            selection_model.select(
                    model.get_index(selected_index), QItemSelectionModel.Select)

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class BindList(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._toolbar = BindListToolBar()

        self._list_model = None
        self._list_view = BindListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._toolbar.set_ui_model(ui_model)
        self._list_view.set_ui_model(ui_model)

        self._update_model()

    def unregister_updaters(self):
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_model()

    def _update_model(self):
        self._list_model = BindListModel()
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)


class EventBox(QComboBox):

    def __init__(self, excluded=set()):
        QComboBox.__init__(self)
        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)
        self.update_names(excluded)

    def update_names(self, excluded=set()):
        selected = None
        selected_index = self.currentIndex()
        if selected_index != -1:
            selected = unicode(self.itemText(selected_index))

        all_events = events.all_events_by_name
        event_names = sorted(list(
            event['name'] for event in all_events.itervalues()
            if event['name'] not in excluded),
            key=lambda x: x.lstrip('/=.->+<') or x)

        old_block = self.blockSignals(True)
        self.clear()
        for event_name in event_names:
            self.addItem(event_name)
        self.blockSignals(old_block)

        if selected:
            self.try_select_event(selected)

    def try_select_event(self, event_name):
        old_block = self.blockSignals(True)
        index = self.findText(event_name)
        if index != -1:
            self.setCurrentIndex(index)
        self.blockSignals(old_block)

    def get_selected_event(self):
        index = self.currentIndex()
        if index == -1:
            return None
        return unicode(self.itemText(index))


class SourceEventSelector(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._selector = EventBox()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(QLabel('Event:'))
        h.addWidget(self._selector)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._selector, SIGNAL('currentIndexChanged(int)'), self._change_event)

        self._update_event()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_event()

    def _update_event(self):
        bindings = self._ui_model.get_module().get_bindings()
        if not bindings.has_selected_binding():
            return

        binding = bindings.get_selected_binding()

        excluded = binding.get_excluded_source_events()
        self._selector.update_names(excluded)

        event_name = binding.get_source_event()
        self._selector.try_select_event(event_name)

    def _change_event(self, index):
        new_event = self._selector.get_selected_event()
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        binding.set_source_event(new_event)
        self._updater.signal_update(set(['signal_bind']))


class TightLabel(QLabel):

    def __init__(self, text):
        QLabel.__init__(self, text)
        self.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)


class Constraints(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._cblist = ConstraintList()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Binding constraints'))
        v.addWidget(self._cblist)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._cblist.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._cblist.unregister_updaters()


class ConstraintList(EditorList):

    def __init__(self):
        EditorList.__init__(self)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _make_adder_widget(self):
        adder = ConstraintAdder()
        adder.set_ui_model(self._ui_model)
        return adder

    def _get_updated_editor_count(self):
        bindings = self._ui_model.get_module().get_bindings()
        if not bindings.has_selected_binding():
            return 0

        constraints = bindings.get_selected_binding().get_constraints()
        return constraints.get_count()

    def _make_editor_widget(self, index):
        editor = ConstraintEditor(index)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_all()

    def _update_all(self):
        self.update_list()


class ConstraintAdder(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Add constraint')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._add_constraint)

    def unregister_updaters(self):
        pass

    def _add_constraint(self):
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        binding.get_constraints().add_constraint()
        self._updater.signal_update(set(['signal_bind']))


class ConstraintEditor(QWidget):

    def __init__(self, index):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._index = index

        self._event = EventBox()

        self._expression = QLineEdit()

        self._remove_button = QPushButton()
        self._remove_button.setStyleSheet('padding: 0 -2px;')

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._event)
        h.addWidget(TightLabel('Test expression:'))
        h.addWidget(self._expression)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))
        self._remove_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        QObject.connect(
                self._event, SIGNAL('currentIndexChanged(int)'), self._change_event)

        QObject.connect(
                self._expression, SIGNAL('editingFinished()'), self._change_expression)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_all()

    def _update_all(self):
        bindings = self._ui_model.get_module().get_bindings()
        if not bindings.has_selected_binding():
            return

        binding = bindings.get_selected_binding()
        constraints = binding.get_constraints()
        if self._index >= constraints.get_count():
            return

        constraint = constraints.get_constraint(self._index)

        event_name = constraint.get_event_name()
        self._event.try_select_event(event_name)

        old_block = self._expression.blockSignals(True)
        new_expression = constraint.get_expression()
        if self._expression.text() != new_expression:
            self._expression.setText(new_expression)
        self._expression.blockSignals(old_block)

    def _get_constraint(self):
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        constraint = binding.get_constraints().get_constraint(self._index)
        return constraint

    def _change_event(self, index):
        event_name = self._event.get_selected_event()
        constraint = self._get_constraint()
        constraint.set_event_name(event_name)
        self._updater.signal_update(set(['signal_bind']))

    def _change_expression(self):
        expression = unicode(self._expression.text())
        constraint = self._get_constraint()
        constraint.set_expression(expression)
        self._updater.signal_update(set(['signal_bind']))

    def _remove(self):
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        binding.get_constraints().remove_constraint(self._index)
        self._updater.signal_update(set(['signal_bind']))


class Targets(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._target_list = TargetList()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Event targets'))
        v.addWidget(self._target_list)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._target_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._target_list.unregister_updaters()


class TargetList(EditorList):

    def __init__(self):
        EditorList.__init__(self)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _make_adder_widget(self):
        adder = TargetAdder()
        adder.set_ui_model(self._ui_model)
        return adder

    def _get_updated_editor_count(self):
        bindings = self._ui_model.get_module().get_bindings()
        if not bindings.has_selected_binding():
            return 0

        targets = bindings.get_selected_binding().get_targets()
        return targets.get_count()

    def _make_editor_widget(self, index):
        editor = TargetEditor(index)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_all()

    def _update_all(self):
        self.update_list()


class TargetAdder(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Add event target')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._add_target)

    def unregister_updaters(self):
        pass

    def _add_target(self):
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        binding.get_targets().add_target()
        self._updater.signal_update(set(['signal_bind']))


class TargetEditor(QWidget):

    def __init__(self, index):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._index = index

        self._ch_offset = QSpinBox()
        self._ch_offset.setRange(-CHANNELS_MAX + 1, CHANNELS_MAX - 1)

        self._event = EventBox()

        self._expression = QLineEdit()

        self._remove_button = QPushButton()
        self._remove_button.setStyleSheet('padding: 0 -2px;')

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(TightLabel('Channel offset:'))
        h.addWidget(self._ch_offset)
        h.addWidget(self._event)
        h.addWidget(TightLabel('Expression:'))
        h.addWidget(self._expression)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))
        self._remove_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        QObject.connect(
                self._ch_offset, SIGNAL('valueChanged(int)'), self._change_ch_offset)

        QObject.connect(
                self._event, SIGNAL('currentIndexChanged(int)'), self._change_event)

        QObject.connect(
                self._expression, SIGNAL('editingFinished()'), self._change_expression)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_all()

    def _update_all(self):
        bindings = self._ui_model.get_module().get_bindings()
        if not bindings.has_selected_binding():
            return

        binding = bindings.get_selected_binding()
        targets = binding.get_targets()
        if self._index >= targets.get_count():
            return

        target = targets.get_target(self._index)

        old_block = self._ch_offset.blockSignals(True)
        new_ch_offset = target.get_channel_offset()
        if self._ch_offset.value() != new_ch_offset:
            self._ch_offset.setValue(new_ch_offset)
        self._ch_offset.blockSignals(old_block)

        excluded = target.get_excluded_events()
        self._event.update_names(excluded)

        event_name = target.get_event_name()
        self._event.try_select_event(event_name)

        all_events = events.all_events_by_name

        old_block = self._expression.blockSignals(True)
        if all_events[event_name]['arg_type'] != None:
            new_expression = target.get_expression()
            if self._expression.text() != new_expression:
                self._expression.setText(new_expression)
            self._expression.setEnabled(True)
        else:
            self._expression.setEnabled(False)
            self._expression.setText('')
        self._expression.blockSignals(old_block)

    def _get_target(self):
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        target = binding.get_targets().get_target(self._index)
        return target

    def _change_ch_offset(self, new_offset):
        target = self._get_target()
        target.set_channel_offset(new_offset)
        self._updater.signal_update(set(['signal_bind']))

    def _change_event(self, index):
        event_name = self._event.get_selected_event()
        target = self._get_target()

        all_events = events.all_events_by_name
        if all_events[event_name]['arg_type'] != None:
            expression = unicode(self._expression.text())
        else:
            expression = None

        target.set_event_info(event_name, expression)
        self._updater.signal_update(set(['signal_bind']))

    def _change_expression(self):
        expression = unicode(self._expression.text())
        target = self._get_target()
        target.set_event_info(self._event.get_selected_event(), expression)
        self._updater.signal_update(set(['signal_bind']))

    def _remove(self):
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        binding.get_targets().remove_target(self._index)
        self._updater.signal_update(set(['signal_bind']))


