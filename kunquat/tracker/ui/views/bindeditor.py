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

from editorlist import EditorList
from headerline import HeaderLine

import kunquat.kunquat.events as events


class BindEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._bind_list = BindList()
        self._event_selector = EventSelector()
        self._constraints = Constraints()
        self._targets = Targets()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Event bindings'))
        v.addWidget(self._bind_list)
        v.addWidget(self._event_selector)
        v.addWidget(self._constraints)
        v.addWidget(self._targets)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._bind_list.set_ui_model(ui_model)
        self._event_selector.set_ui_model(ui_model)
        self._constraints.set_ui_model(ui_model)
        self._targets.set_ui_model(ui_model)

        self._update_editor_enabled()

    def unregister_updaters(self):
        self._targets.unregister_updaters()
        self._constraints.unregister_updaters()
        self._event_selector.unregister_updaters()
        self._bind_list.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_editor_enabled()

    def _update_editor_enabled(self):
        bindings = self._ui_model.get_module().get_bindings()
        enable_editor = bindings.has_selected_binding()
        self._event_selector.setEnabled(enable_editor)
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


class EventSelector(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._selector = QComboBox()
        self._selector.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        all_events = events.all_events_by_name
        event_names = sorted(list(event['name'] for event in all_events.itervalues()))
        for event_name in event_names:
            self._selector.addItem(event_name)

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

        event_name = bindings.get_selected_binding().get_source_event()

        old_block = self._selector.blockSignals(True)
        self._selector.setCurrentIndex(self._selector.findText(event_name))
        self._selector.blockSignals(old_block)

    def _change_event(self, index):
        new_event = str(self._selector.itemText(index))
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        binding.set_source_event(new_event)
        self._updater.signal_update(set(['signal_bind']))


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

        self._event_selector = QLabel(str(index))

        self._remove_button = QPushButton()
        self._remove_button.setStyleSheet('padding: 0 -2px;')

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._event_selector)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))
        self._remove_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_all()

    def _update_all(self):
        pass # TODO

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

        self._ch_offset = QLabel(str(index))

        self._remove_button = QPushButton()
        self._remove_button.setStyleSheet('padding: 0 -2px;')

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._ch_offset)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))
        self._remove_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_all()

    def _update_all(self):
        pass # TODO

    def _remove(self):
        bindings = self._ui_model.get_module().get_bindings()
        binding = bindings.get_selected_binding()
        binding.get_targets().remove_target(self._index)
        self._updater.signal_update(set(['signal_bind']))


