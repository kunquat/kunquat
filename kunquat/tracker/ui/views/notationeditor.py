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

from headerline import HeaderLine


class NotationEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._notations = Notations()
        self._octaves = Octaves()
        self._notes = Notes()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._notations)
        h.addWidget(self._octaves)
        h.addWidget(self._notes)
        h.addStretch(1)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._notations.set_ui_model(ui_model)
        self._octaves.set_ui_model(ui_model)
        self._notes.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._notes.unregister_updaters()
        self._octaves.unregister_updaters()
        self._notations.unregister_updaters()


class NotationListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add notation')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove notation')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_notation)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_notation)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_list', 'signal_notation_editor_selection'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        has_custom_notations = len(notation_manager.get_custom_notation_ids()) > 0
        has_selected_notation = bool(notation_manager.get_editor_selected_notation_id())
        self._remove_button.setEnabled(has_custom_notations and has_selected_notation)

    def _add_notation(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation_manager.add_custom_notation()
        self._updater.signal_update(set(['signal_notation_list']))

    def _remove_notation(self):
        notation_manager = self._ui_model.get_notation_manager()
        selected_notation_id = notation_manager.get_editor_selected_notation_id()
        notation_manager.remove_custom_notation(selected_notation_id)
        notation_manager.set_editor_selected_notation_id(None)
        self._updater.signal_update(set(['signal_notation_list']))


class NotationListModel(QAbstractListModel):

    def __init__(self):
        QAbstractListModel.__init__(self)
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
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
        notation_manager = self._ui_model.get_notation_manager()
        notation_ids = notation_manager.get_custom_notation_ids()

        self._items = list((nid, notation_manager.get_notation(nid).get_name())
                for nid in notation_ids)

    def get_index(self, notation_id):
        _, list_index = notation_id
        return self.createIndex(list_index, 0, self._items[list_index])

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role in (Qt.DisplayRole, Qt.EditRole):
            row = index.row()
            if 0 <= row < len(self._items):
                _, name = self._items[row]
                if role == Qt.DisplayRole:
                    vis_name = name or u'-'
                    return QVariant(vis_name)
                elif role == Qt.EditRole:
                    return QVariant(name)

        return QVariant()

    def headerData(self, section, orientation, role):
        return QVariant()

    def flags(self, index):
        default_flags = QAbstractItemModel.flags(self, index)
        if not index.isValid():
            return default_flags
        if not 0 <= index.row() < len(self._items):
            return default_flags

        return default_flags | Qt.ItemIsEditable

    def setData(self, index, value, role):
        if role == Qt.EditRole:
            if 0 <= index.row() < len(self._items):
                new_name = unicode(value.toString())
                notation_manager = self._ui_model.get_notation_manager()
                selected_notation_id = notation_manager.get_editor_selected_notation_id()
                notation = notation_manager.get_notation(selected_notation_id)
                notation.set_name(new_name)
                self._updater.signal_update(set(['signal_notation_list']))
                return True

        return False


class NotationListView(QListView):

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
            notation_id, _ = item
            notation_manager = self._ui_model.get_notation_manager()
            notation_manager.set_editor_selected_notation_id(notation_id)
            self._updater.signal_update(set(['signal_notation_editor_selection']))

    def setModel(self, model):
        QListView.setModel(self, model)

        selection_model = self.selectionModel()

        notation_manager = self._ui_model.get_notation_manager()
        selected_notation_id = notation_manager.get_editor_selected_notation_id()
        if selected_notation_id:
            selection_model.select(
                    model.get_index(selected_notation_id), QItemSelectionModel.Select)

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class Notations(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._toolbar = NotationListToolBar()

        self._list_model = None
        self._list_view = NotationListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Custom notations'))
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
        if 'signal_notation_list' in signals:
            self._update_model()

    def _update_model(self):
        self._list_model = NotationListModel()
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)


class OctaveListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add octave')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove octave')
        self._remove_button.setEnabled(False)

        self._set_base_button = QToolButton()
        self._set_base_button.setText('Set base')
        self._set_base_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)
        self.addWidget(self._set_base_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_octave)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_octave)
        QObject.connect(
                self._set_base_button, SIGNAL('clicked()'), self._set_base_octave)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        pass

    def _update_enabled(self):
        pass

    def _add_octave(self):
        pass

    def _remove_octave(self):
        pass

    def _set_base_octave(self):
        pass


class OctaveListModel(QAbstractListModel):

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

    def _make_items(self):
        pass

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                return QVariant() # TODO

    def headerData(self, section, orientation, role):
        return QVariant()


class OctaveListView(QListView):

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
        pass # TODO

    def setModel(self, model):
        QListView.setModel(self, model)

        selection_model = self.selectionModel()

        # TODO: Refresh selection

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class Octaves(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None

        self._toolbar = OctaveListToolBar()

        self._list_model = None
        self._list_view = OctaveListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Octaves'))
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._toolbar.set_ui_model(ui_model)
        self._list_view.set_ui_model(ui_model)

        self._update_model()

    def unregister_updaters(self):
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()

    def _update_model(self):
        self._list_model = OctaveListModel()
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)


class NoteListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add note')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove note')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_note)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_note)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        pass

    def _update_enabled(self):
        pass

    def _add_note(self):
        pass

    def _remove_note(self):
        pass


class NoteListModel(QAbstractListModel):

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

    def _make_items(self):
        pass

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


class NoteListView(QListView):

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
        pass # TODO

    def setModel(self, model):
        QListView.setModel(self, model)

        selection_model = self.selectionModel()

        # TODO: Refresh selection

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class Notes(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._toolbar = NoteListToolBar()

        self._list_model = None
        self._list_view = NoteListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Notes'))
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
        pass

    def _update_model(self):
        self._list_model = NoteListModel()
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)


