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


'''
Format:
bind = [bind_entry]
bind_entry = [event_name, constraints, target_events]
constraints = [constraint]
constraint = [event_name, expr]
target_events = [target_event]
target_event = [ch_offset, [event_name, maybe_expr]]
'''


class BindEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._bind_list = BindList()
        self._event_name = EventName()
        self._constraints = Constraints()
        self._targets = Targets()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Event bindings'))
        v.addWidget(self._bind_list)
        v.addWidget(self._event_name)
        v.addWidget(self._constraints)
        v.addWidget(self._targets)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._bind_list.set_ui_model(ui_model)
        self._event_name.set_ui_model(ui_model)
        self._constraints.set_ui_model(ui_model)
        self._targets.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._targets.unregister_updaters()
        self._constraints.unregister_updaters()
        self._event_name.unregister_updaters()
        self._bind_list.unregister_updaters()


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
        pass # TODO

    def _add_binding(self):
        pass # TODO

    def _remove_binding(self):
        pass # TODO


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
        pass # TODO

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                return QVariant() # TODO

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
            pass # TODO

    def setModel(self, model):
        QListView.setModel(self, model)

        selection_model = self.selectionModel()
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


class EventName(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._edit = QLineEdit()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(QLabel('Event name:'))
        h.addWidget(self._edit)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_name()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_bind' in signals:
            self._update_name()

    def _update_name(self):
        pass # TODO


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
        return 0 # TODO

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

        # TODO


class ConstraintAdder(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Add constraint')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass


class ConstraintEditor(QWidget):

    def __init__(self, index):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._index = index

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass


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
        return 0 # TODO

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

        # TODO


class TargetAdder(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Add event target')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass


class TargetEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._index = index

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass


