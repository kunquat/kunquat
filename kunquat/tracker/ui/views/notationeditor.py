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

import math

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from .headerline import HeaderLine


class NotationEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._notations = Notations()
        self._tuning_tables = TuningTables()
        self._template = Template()
        self._octaves = Octaves()
        self._notes = Notes()
        self._note = Note()
        self._keymap = Keymap()

        ll = QVBoxLayout()
        ll.setMargin(0)
        ll.setSpacing(2)
        ll.addWidget(self._notations)
        ll.addWidget(self._tuning_tables)

        nl = QVBoxLayout()
        nl.setMargin(0)
        nl.setSpacing(2)
        nl.addWidget(self._notes)
        nl.addWidget(self._note)

        nlists = QHBoxLayout()
        nlists.setMargin(0)
        nlists.setSpacing(2)
        nlists.addWidget(self._template)
        nlists.addWidget(self._octaves)
        nlists.addLayout(nl)

        el = QVBoxLayout()
        el.setMargin(0)
        el.setSpacing(2)
        el.addLayout(nlists)
        el.addWidget(self._keymap)

        separator = QFrame()
        separator.setFrameShape(QFrame.VLine)
        separator.setFrameShadow(QFrame.Sunken)
        separator.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.MinimumExpanding)
        separator.setMinimumWidth(2)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addLayout(ll)
        h.addWidget(separator)
        h.addLayout(el, 1)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._notations.set_ui_model(ui_model)
        self._tuning_tables.set_ui_model(ui_model)
        self._template.set_ui_model(ui_model)
        self._octaves.set_ui_model(ui_model)
        self._notes.set_ui_model(ui_model)
        self._note.set_ui_model(ui_model)
        self._keymap.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._keymap.unregister_updaters()
        self._note.unregister_updaters()
        self._notes.unregister_updaters()
        self._octaves.unregister_updaters()
        self._template.unregister_updaters()
        self._tuning_tables.unregister_updaters()
        self._notations.unregister_updaters()


class NotationListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add notation')
        self._add_button.setToolTip('Add notation')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove notation')
        self._remove_button.setToolTip('Remove notation')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._add_button.setIcon(QIcon(icon_bank.get_icon_path('add')))
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('remove')))

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
        notation_manager.set_editor_selected_octave_id(None)
        notation_manager.set_editor_selected_note_index(None)
        notation_manager.set_editor_selected_key_index(None)
        notation_manager.set_editor_selected_template_note(None)
        self._updater.signal_update(set([
            'signal_notation',
            'signal_notation_list',
            'signal_notation_editor_selection']))


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
                    vis_name = name or '-'
                    return vis_name
                elif role == Qt.EditRole:
                    return name
                else:
                    assert False

        return None

    def headerData(self, section, orientation, role):
        return None

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
                new_name = value
                notation_manager = self._ui_model.get_notation_manager()
                notation = notation_manager.get_editor_selected_notation()
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

        self.setMinimumWidth(100)

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
            notation_manager.set_editor_selected_octave_id(None)
            notation_manager.set_editor_selected_note_index(None)
            notation_manager.set_editor_selected_key_index(None)
            notation_manager.set_editor_selected_template_note(None)
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


class TuningTableListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add tuning table')
        self._add_button.setToolTip('Add tuning table')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove tuning table')
        self._remove_button.setToolTip('Remove tuning table')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._add_button.setIcon(QIcon(icon_bank.get_icon_path('add')))
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('remove')))

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_tuning_table)
        QObject.connect(
                self._remove_button, SIGNAL('clicked()'), self._remove_tuning_table)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_tuning_tables', 'signal_tuning_table_selection'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        module = self._ui_model.get_module()
        self._add_button.setEnabled(module.get_free_tuning_table_id() != None)

        notation_manager = self._ui_model.get_notation_manager()
        selected_table_id = notation_manager.get_editor_selected_tuning_table_id()
        self._remove_button.setEnabled(selected_table_id != None)

    def _add_tuning_table(self):
        module = self._ui_model.get_module()
        module.add_tuning_table(module.get_free_tuning_table_id())
        self._updater.signal_update(set(['signal_tuning_tables']))

    def _remove_tuning_table(self):
        notation_manager = self._ui_model.get_notation_manager()
        selected_table_id = notation_manager.get_editor_selected_tuning_table_id()
        module = self._ui_model.get_module()
        table = module.get_tuning_table(selected_table_id)
        table.remove()
        notation_manager.set_editor_selected_tuning_table_id(None)
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_tuning_table_editor(selected_table_id)
        self._updater.signal_update(
                set(['signal_tuning_tables', 'signal_tuning_table_selection']))


class TuningTableListModel(QAbstractTableModel):

    def __init__(self):
        QAbstractTableModel.__init__(self)
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
            return self._items[row]
        return None

    def get_index(self, table_id):
        for i, item in enumerate(self._items):
            cur_id, _ = item
            if cur_id == table_id:
                return self.createIndex(i, 0, item)
        return QModelIndex()

    def _make_items(self):
        module = self._ui_model.get_module()
        table_ids = module.get_tuning_table_ids()

        self._items = []
        for table_id in sorted(list(table_ids)):
            table = module.get_tuning_table(table_id)
            self._items.append((table_id, table.get_name()))

    # Qt interface

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return 1

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._items)

    def data(self, index, role):
        if role in (Qt.DisplayRole, Qt.EditRole):
            row = index.row()
            column = index.column()
            if 0 <= row < len(self._items) and column == 0:
                _, name = self._items[row]
                if role == Qt.DisplayRole:
                    vis_name = name or '-'
                    return vis_name
                elif role == Qt.EditRole:
                    return name
                else:
                    assert False

        return None

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole and orientation == Qt.Vertical:
            if 0 <= section < len(self._items):
                table_id, _ = self._items[section]
                num = int(table_id.split('_')[1], 16)
                return str(num)
        return None

    def flags(self, index):
        default_flags = QAbstractItemModel.flags(self, index)
        if not index.isValid():
            return default_flags
        if index.column() != 0 or not 0 <= index.row() < len(self._items):
            return default_flags

        return default_flags | Qt.ItemIsEditable

    def setData(self, index, value, role):
        if role == Qt.EditRole:
            row = index.row()
            if index.column() == 0 and 0 <= row < len(self._items):
                new_name = value
                table_id, _ = self._items[row]
                module = self._ui_model.get_module()
                table = module.get_tuning_table(table_id)
                table.set_name(new_name)
                self._updater.signal_update(set(['signal_tuning_tables']))
                return True

        return False


class TuningTableListView(QTableView):

    def __init__(self):
        QTableView.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

        self.setMinimumWidth(100)

        hheader = self.horizontalHeader()
        hheader.setStretchLastSection(True)
        hheader.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item:
            table_id, _ = item
            notation_manager = self._ui_model.get_notation_manager()
            notation_manager.set_editor_selected_tuning_table_id(table_id)
            self._updater.signal_update(set(['signal_tuning_table_selection']))

    def setModel(self, model):
        QTableView.setModel(self, model)

        selection_model = self.selectionModel()

        notation_manager = self._ui_model.get_notation_manager()
        selected_table_id = notation_manager.get_editor_selected_tuning_table_id()
        if selected_table_id:
            selection_model.select(
                    model.get_index(selected_table_id), QItemSelectionModel.Select)

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class TuningTables(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._toolbar = TuningTableListToolBar()

        self._list_model = None
        self._list_view = TuningTableListView()

        self._edit_button = QPushButton('Edit tuning table')
        self._edit_button.setEnabled(False)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Tuning tables'))
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        v.addWidget(self._edit_button)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._toolbar.set_ui_model(ui_model)
        self._list_view.set_ui_model(ui_model)

        QObject.connect(self._edit_button, SIGNAL('clicked()'), self._open_editor)

        self._update_model()
        self._update_selection()

    def unregister_updaters(self):
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_tuning_tables' in signals:
            self._update_model()
        if 'signal_tuning_table_selection' in signals:
            self._update_selection()

    def _update_model(self):
        self._list_model = TuningTableListModel()
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)

    def _update_selection(self):
        notation_manager = self._ui_model.get_notation_manager()
        selected_table_id = notation_manager.get_editor_selected_tuning_table_id()
        self._edit_button.setEnabled(selected_table_id != None)

    def _open_editor(self):
        notation_manager = self._ui_model.get_notation_manager()
        selected_table_id = notation_manager.get_editor_selected_tuning_table_id()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_tuning_table_editor(selected_table_id)


class Template(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._center_pitch = CenterPitch()
        self._octave_ratio = OctaveRatio()
        self._octaves = TemplateOctaves()
        self._notes = TemplateNotes()
        self._create_button = QPushButton('Create notation and keymap')
        self._create_tt_button = QPushButton('Create tuning table')

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Template'))
        v.addWidget(self._center_pitch)
        v.addWidget(self._octave_ratio)
        v.addWidget(self._octaves)
        v.addWidget(self._notes)
        v.addWidget(self._create_button)
        v.addWidget(self._create_tt_button)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._center_pitch.set_ui_model(ui_model)
        self._octave_ratio.set_ui_model(ui_model)
        self._octaves.set_ui_model(ui_model)
        self._notes.set_ui_model(ui_model)

        QObject.connect(self._create_button, SIGNAL('clicked()'), self._create)
        QObject.connect(
                self._create_tt_button, SIGNAL('clicked()'), self._create_tuning_table)

        self._update_enabled()

    def unregister_updaters(self):
        self._notes.unregister_updaters()
        self._octaves.unregister_updaters()
        self._octave_ratio.unregister_updaters()
        self._center_pitch.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_list',
            'signal_notation_editor_selection',
            'signal_notation_template_notes',
            'signal_tuning_tables'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        if not notation:
            self.setEnabled(False)
            return

        self.setEnabled(True)

        note_count = notation.get_template().get_note_count()
        self._create_button.setEnabled(note_count > 0)

        module = self._ui_model.get_module()
        self._create_tt_button.setEnabled(
                note_count > 0 and module.get_free_tuning_table_id() != None)

    def _create(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        notation.apply_template_settings()
        notation_manager.set_editor_selected_octave_id(None)
        notation_manager.set_editor_selected_note_index(None)
        notation_manager.set_editor_selected_key_index(None)

        typewriter_manager = self._ui_model.get_typewriter_manager()
        if ((notation_manager.get_editor_selected_notation_id() ==
                    notation_manager.get_selected_notation_id()) and
                typewriter_manager.get_octave() >= notation.get_octave_count()):
            typewriter_manager.set_octave(notation.get_octave_count() - 1)

        self._updater.signal_update(set([
            'signal_notation',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection',
            'signal_notation_editor_notes',
            'signal_notation_editor_note_selection',
            'signal_notation_editor_key_count',
            'signal_notation_editor_key_selection']))

    def _create_tuning_table(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        name = notation.get_name()
        template = notation.get_template()
        module = self._ui_model.get_module()
        table_id = module.get_free_tuning_table_id()
        assert table_id != None
        module.create_tuning_table_from_notation_template(table_id, name, template)
        self._updater.signal_update(set(['signal_tuning_tables']))


class CenterPitch(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._value = QDoubleSpinBox()
        self._value.setDecimals(2)
        self._value.setRange(-9999, 9999)
        self._value.setValue(0)

        self._units = QComboBox()
        self._units.addItem('cents')
        self._units.addItem('Hz')
        self._units.setCurrentIndex(self._units.findText('cents'))

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(QLabel('Center pitch:'), 0)
        h.addWidget(self._value, 1)
        h.addWidget(self._units, 1)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._value, SIGNAL('valueChanged(double)'), self._change_center)
        QObject.connect(
                self._units, SIGNAL('currentIndexChanged(int)'), self._change_units)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_list',
            'signal_notation_editor_selection',
            'signal_notation_template_center_pitch'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        if not notation:
            return

        value, units = notation.get_template().get_center_pitch()

        old_block = self._value.blockSignals(True)
        if units == 'cents':
            self._value.setRange(-9999, 9999)
        elif units == 'Hz':
            self._value.setRange(1, 20000)
        else:
            assert False
        if self._value.value() != value:
            self._value.setValue(value)
        self._value.blockSignals(old_block)

        old_block = self._units.blockSignals(True)
        self._units.setCurrentIndex(self._units.findText(units))
        self._units.blockSignals(old_block)

    def _change_center(self, new_center):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()

        _, units = template.get_center_pitch()
        template.set_center_pitch(new_center, units)
        self._updater.signal_update(set(['signal_notation_template_center_pitch']))

    def _get_cents(self, hz):
        return math.log(hz / 440.0, 2) * 1200

    def _get_hz(self, cents):
        return 2**(cents / 1200.0) * 440

    def _change_units(self, item_index):
        new_units = str(self._units.itemText(item_index))

        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()

        value, units = template.get_center_pitch()
        if units == new_units:
            return

        if new_units == 'cents':
            new_value = self._get_cents(value)
        elif new_units == 'Hz':
            new_value = self._get_hz(value)

        template.set_center_pitch(new_value, new_units)
        self._updater.signal_update(set(['signal_notation_template_center_pitch']))


class RatioValidator(QValidator):

    def __init__(self):
        QValidator.__init__(self)

    def validate(self, contents, pos):
        in_str = str(contents)
        if not in_str:
            return (QValidator.Intermediate, contents, pos)

        if '/' in in_str:
            parts = in_str.split('/')
            if len(parts) != 2:
                return (QValidator.Invalid, contents, pos)
            nums = []
            for part in parts:
                if not part.strip():
                    continue
                try:
                    nums.append(int(part))
                except ValueError:
                    return (QValidator.Invalid, contents, pos)
            if any(num < 0 for num in nums):
                return (QValidator.Invalid, contents, pos)
            if len(nums) < 2:
                return (QValidator.Intermediate, contents, pos)
            if all(num > 0 for num in nums) and nums[0] > nums[1]:
                return (QValidator.Acceptable, contents, pos)
            return (QValidator.Intermediate, contents, pos)
        else:
            try:
                value = float(in_str)
            except ValueError:
                return (QValidator.Invalid, contents, pos)
            if value < 0:
                return (QValidator.Invalid, contents, pos)
            if value == 0:
                return (QValidator.Intermediate, contents, pos)

        return (QValidator.Acceptable, contents, pos)


class OctaveRatio(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._ratio = QLineEdit()
        self._ratio.setValidator(RatioValidator())

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(QLabel('Octave ratio:'))
        h.addWidget(self._ratio)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._ratio, SIGNAL('editingFinished()'), self._change_ratio)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_list',
            'signal_notation_editor_selection',
            'signal_notation_template_octave_ratio'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        if not notation:
            return

        ratio = notation.get_template().get_octave_ratio()

        old_block = self._ratio.blockSignals(True)
        if isinstance(ratio, list):
            self._ratio.setText('{}/{}'.format(*ratio))
        else:
            self._ratio.setText(str(ratio))
        self._ratio.blockSignals(old_block)

    def _change_ratio(self):
        text = str(self._ratio.text())
        if '/' in text:
            parts = text.split('/')
            value = [int(part) for part in parts]
        else:
            value = float(text)

        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()

        template.set_octave_ratio(value)
        self._updater.signal_update(set(['signal_notation_template_octave_ratio']))


class TemplateOctaves(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._lowest = QSpinBox()
        self._lowest.setRange(0, 15)
        self._center = QSpinBox()
        self._center.setRange(0, 15)
        self._highest = QSpinBox()
        self._highest.setRange(0, 15)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(QLabel('Lowest octave:'))
        h.addWidget(self._lowest)
        h.addWidget(QLabel('Center:'))
        h.addWidget(self._center)
        h.addWidget(QLabel('Highest:'))
        h.addWidget(self._highest)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._lowest, SIGNAL('valueChanged(int)'), self._change_lowest)
        QObject.connect(self._center, SIGNAL('valueChanged(int)'), self._change_center)
        QObject.connect(self._highest, SIGNAL('valueChanged(int)'), self._change_highest)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_list',
            'signal_notation_editor_selection',
            'signal_notation_template_octaves'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        if not notation:
            return

        lowest, center, highest = notation.get_template().get_octaves()

        old_block = self._lowest.blockSignals(True)
        if self._lowest.value() != lowest:
            self._lowest.setValue(lowest)
        self._lowest.blockSignals(old_block)

        old_block = self._center.blockSignals(True)
        self._center.setRange(lowest, highest)
        if self._center.value() != center:
            self._center.setValue(center)
        self._center.blockSignals(old_block)

        old_block = self._highest.blockSignals(True)
        if self._highest.value() != highest:
            self._highest.setValue(highest)
        self._highest.blockSignals(old_block)

    def _change_lowest(self, value):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()

        octaves = template.get_octaves()
        octaves[0] = value
        octaves[1] = max(octaves[0], octaves[1])
        octaves[2] = max(octaves[0], octaves[2])

        template.set_octaves(*octaves)
        self._updater.signal_update(set(['signal_notation_template_octaves']))

    def _change_center(self, value):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()

        octaves = template.get_octaves()
        octaves[1] = value
        assert octaves[0] <= octaves[1] <= octaves[2]

        template.set_octaves(*octaves)
        self._updater.signal_update(set(['signal_notation_template_octaves']))

    def _change_highest(self, value):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()

        octaves = template.get_octaves()
        octaves[2] = value
        octaves[1] = min(octaves[1], octaves[2])
        octaves[0] = min(octaves[0], octaves[2])

        template.set_octaves(*octaves)
        self._updater.signal_update(set(['signal_notation_template_octaves']))


class TemplateNotesToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add note')
        self._add_button.setToolTip('Add note')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove note')
        self._remove_button.setToolTip('Remove note')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._add_button.setIcon(QIcon(icon_bank.get_icon_path('add')))
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('remove')))

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_note)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_note)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_list',
            'signal_notation_editor_selection',
            'signal_notation_template_note_selection',
            'signal_notation_template_notes'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        selected_note_coords = notation_manager.get_editor_selected_template_note()
        notation = notation_manager.get_editor_selected_notation()
        self._remove_button.setEnabled(bool(notation and selected_note_coords))

    def _add_note(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()
        template.add_note()
        self._updater.signal_update(set(['signal_notation_template_notes']))

    def _remove_note(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        template = notation.get_template()
        index, column = notation_manager.get_editor_selected_template_note()
        template.remove_note(index)
        note_count = template.get_note_count()
        if note_count > 0:
            notation_manager.set_editor_selected_template_note(
                    (min(index, template.get_note_count() - 1), column))
        else:
            notation_manager.set_editor_selected_template_note(None)
        self._updater.signal_update(set(['signal_notation_template_notes']))


class TemplateNoteTableModel(QAbstractTableModel):

    def __init__(self):
        QAbstractTableModel.__init__(self)
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._make_items()

    def unregister_updaters(self):
        pass

    def get_index(self, row, column):
        return self.createIndex(row, column, None)

    def _make_items(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()

        self._items = []
        if notation:
            template = notation.get_template()
            for i in range(template.get_note_count()):
                name = template.get_note_name(i)
                ratio = template.get_note_ratio(i)
                self._items.append((name, ratio))

    # Qt interface

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return 2

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._items)

    def data(self, index, role):
        if role in (Qt.DisplayRole, Qt.EditRole):
            row = index.row()
            column = index.column()
            if 0 <= row < len(self._items):
                if column == 0:
                    name, _ = self._items[row]
                    return name
                elif column == 1:
                    _, ratio = self._items[row]
                    if isinstance(ratio, list):
                        return '{}/{}'.format(*ratio)
                    else:
                        return str(ratio)

        return None

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            if section == 0:
                return 'Name'
            elif section == 1:
                return 'Ratio'
        return None

    def flags(self, index):
        default_flags = QAbstractTableModel.flags(self, index)
        if not index.isValid():
            return default_flags
        if not 0 <= index.row() < len(self._items):
            return default_flags

        return default_flags | Qt.ItemIsEditable

    def _get_validated_ratio(self, text):
        if '/' in text:
            parts = str(text).split('/')
            if len(parts) != 2:
                return None
            nums = []
            for part in parts:
                try:
                    nums.append(int(part))
                except ValueError:
                    return None
            if nums[0] <= 0:
                return None
            if nums[1] <= 0:
                return None
            return nums
        else:
            try:
                value = float(text)
            except ValueError:
                return None
            return value

    def setData(self, index, value, role):
        if role == Qt.EditRole:
            row = index.row()
            column = index.column()
            if 0 <= row < len(self._items):
                name, ratio = self._items[row]
                if column == 0:
                    new_name = value
                    notation_manager = self._ui_model.get_notation_manager()
                    notation = notation_manager.get_editor_selected_notation()
                    template = notation.get_template()
                    template.set_note_name(row, new_name)
                    self._updater.signal_update(set(['signal_notation_template_notes']))
                    return True
                elif column == 1:
                    new_ratio = self._get_validated_ratio(value)
                    if new_ratio == None:
                        return False
                    notation_manager = self._ui_model.get_notation_manager()
                    notation = notation_manager.get_editor_selected_notation()
                    template = notation.get_template()
                    template.set_note_ratio(row, new_ratio)
                    self._updater.signal_update(set(['signal_notation_template_notes']))
                    return True

        return False


class TemplateNoteTableView(QTableView):

    def __init__(self):
        QTableView.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

        self.setMinimumWidth(100)

        header = self.horizontalHeader()
        header.setStretchLastSection(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass

    def _select_entry(self, cur_index, prev_index):
        if not cur_index.isValid():
            return
        row, column = cur_index.row(), cur_index.column()

        notation_manager = self._ui_model.get_notation_manager()
        notation_manager.set_editor_selected_template_note((row, column))
        self._updater.signal_update(set(['signal_notation_template_note_selection']))

    def setModel(self, model):
        QTableView.setModel(self, model)

        selection_model = self.selectionModel()

        notation_manager = self._ui_model.get_notation_manager()
        coords = notation_manager.get_editor_selected_template_note()
        if coords:
            row, column = coords
            selection_model.select(
                    model.get_index(row, column), QItemSelectionModel.Select)

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class TemplateNotes(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._toolbar = TemplateNotesToolBar()

        self._table_model = None
        self._table_view = TemplateNoteTableView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(self._toolbar)
        v.addWidget(self._table_view)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._toolbar.set_ui_model(ui_model)
        self._table_view.set_ui_model(ui_model)

        self._update_model()

    def unregister_updaters(self):
        self._table_view.unregister_updaters()
        self._toolbar.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_list',
            'signal_notation_editor_selection',
            'signal_notation_template_notes'])
        if not signals.isdisjoint(update_signals):
            self._update_model()

    def _update_model(self):
        self._table_model = TemplateNoteTableModel()
        self._table_model.set_ui_model(self._ui_model)
        self._table_view.setModel(self._table_model)


class OctaveListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add octave')
        self._add_button.setToolTip('Add octave')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove octave')
        self._remove_button.setToolTip('Remove octave')
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

        icon_bank = self._ui_model.get_icon_bank()
        self._add_button.setIcon(QIcon(icon_bank.get_icon_path('add')))
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('remove')))

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_octave)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_octave)
        QObject.connect(
                self._set_base_button, SIGNAL('clicked()'), self._set_base_octave)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        if not notation:
            self.setEnabled(False)
            return

        self.setEnabled(True)
        has_selected_octave = notation_manager.get_editor_selected_octave_id() != None
        self._remove_button.setEnabled(
                has_selected_octave and notation.get_octave_count() > 1)
        self._set_base_button.setEnabled(has_selected_octave)

    def _add_octave(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        notation.add_octave()
        self._updater.signal_update(set([
            'signal_notation',
            'signal_notation_editor_octaves']))

    def _remove_octave(self):
        notation_manager = self._ui_model.get_notation_manager()
        selected_octave_id = notation_manager.get_editor_selected_octave_id()
        notation = notation_manager.get_editor_selected_notation()
        notation.remove_octave(selected_octave_id)
        base_octave_id = notation.get_base_octave_id()
        if base_octave_id > 0 and base_octave_id >= selected_octave_id:
            notation.set_base_octave_id(base_octave_id - 1)
        notation_manager.set_editor_selected_octave_id(max(0, selected_octave_id - 1))
        notation_manager.set_editor_selected_key_index(None)
        self._updater.signal_update(set([
            'signal_notation',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection']))

    def _set_base_octave(self):
        notation_manager = self._ui_model.get_notation_manager()
        selected_octave_id = notation_manager.get_editor_selected_octave_id()
        notation = notation_manager.get_editor_selected_notation()
        notation.set_base_octave_id(selected_octave_id)
        self._updater.signal_update(set(['signal_notation_editor_octaves']))


class OctaveListModel(QAbstractListModel):

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
        notation = notation_manager.get_editor_selected_notation()

        if notation:
            octave_count = notation.get_octave_count()
            self._items = [notation.get_octave_name(i) for i in range(octave_count)]
        else:
            self._items = []

    def get_index(self, octave_index):
        return self.createIndex(octave_index, 0, self._items[octave_index])

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role in (Qt.DisplayRole, Qt.EditRole):
            row = index.row()
            if 0 <= row < len(self._items):
                name = self._items[row]
                if role == Qt.DisplayRole:
                    vis_name = name or '-'
                    notation_manager = self._ui_model.get_notation_manager()
                    notation = notation_manager.get_editor_selected_notation()
                    if row == notation.get_base_octave_id():
                        vis_name += ' *'
                    return vis_name
                elif role == Qt.EditRole:
                    return name
                else:
                    assert False

        return None

    def headerData(self, section, orientation, role):
        return None

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
                new_name = value
                notation_manager = self._ui_model.get_notation_manager()
                notation = notation_manager.get_editor_selected_notation()
                notation.set_octave_name(index.row(), new_name)
                self._updater.signal_update(set([
                    'signal_notation',
                    'signal_notation_editor_octaves']))
                return True

        return False


class OctaveListView(QListView):

    def __init__(self):
        QListView.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

        self.setMinimumWidth(100)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item != None:
            notation_manager = self._ui_model.get_notation_manager()
            notation_manager.set_editor_selected_octave_id(cur_index.row())
            notation_manager.set_editor_selected_key_index(None)
            self._updater.signal_update(set(['signal_notation_editor_octave_selection']))

    def setModel(self, model):
        QListView.setModel(self, model)

        selection_model = self.selectionModel()

        notation_manager = self._ui_model.get_notation_manager()
        selected_octave_id = notation_manager.get_editor_selected_octave_id()
        if selected_octave_id != None:
            selection_model.select(
                    model.get_index(selected_octave_id), QItemSelectionModel.Select)

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class Octaves(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

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
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._toolbar.set_ui_model(ui_model)
        self._list_view.set_ui_model(ui_model)

        self._update_model()
        self._update_enabled()

    def unregister_updaters(self):
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        model_update_signals = set([
            'signal_notation_editor_selection', 'signal_notation_editor_octaves'])
        if not signals.isdisjoint(model_update_signals):
            self._update_model()
            self._update_enabled()
        if 'signal_notation_editor_octave_selection' in signals:
            self._update_enabled()

    def _update_model(self):
        self._list_model = OctaveListModel()
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        self.setEnabled(notation_manager.get_editor_selected_notation_id() != None)


class NoteListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add note')
        self._add_button.setToolTip('Add note')
        self._add_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove note')
        self._remove_button.setToolTip('Remove note')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._add_button.setIcon(QIcon(icon_bank.get_icon_path('add')))
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('remove')))

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_note)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_note)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_notes',
            'signal_notation_editor_note_selection'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        if not notation:
            self.setEnabled(False)
            return

        self.setEnabled(True)
        has_selected_note = notation_manager.get_editor_selected_note_index() != None
        has_notes = bool(notation.get_notes())
        self._remove_button.setEnabled(has_selected_note and has_notes)

    def _add_note(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        notation.add_note()
        self._updater.signal_update(set([
            'signal_notation', 'signal_notation_editor_notes']))

    def _remove_note(self):
        notation_manager = self._ui_model.get_notation_manager()
        note_index = notation_manager.get_editor_selected_note_index()
        notation = notation_manager.get_editor_selected_notation()
        notation.remove_note(note_index)
        notation_manager.set_editor_selected_note_index(None)
        self._updater.signal_update(set([
            'signal_notation', 'signal_notation_editor_notes']))


class NoteListModel(QAbstractListModel):

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
        notation = notation_manager.get_editor_selected_notation()

        if notation:
            self._items = list(notation.get_notes())
        else:
            self._items = []

    def get_index(self, list_index):
        return self.createIndex(list_index, 0, self._items[list_index])

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                _, name = self._items[row]
                vis_name = name or '-'
                return vis_name

        return None

    def headerData(self, section, orientation, role):
        return None


class NoteListView(QListView):

    def __init__(self):
        QListView.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

        self.setMinimumWidth(100)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item != None:
            notation_manager = self._ui_model.get_notation_manager()
            notation_manager.set_editor_selected_note_index(cur_index.row())
            self._updater.signal_update(set(['signal_notation_editor_note_selection']))

    def setModel(self, model):
        QListView.setModel(self, model)

        selection_model = self.selectionModel()

        notation_manager = self._ui_model.get_notation_manager()
        selected_note_index = notation_manager.get_editor_selected_note_index()
        if selected_note_index != None:
            selection_model.select(
                    model.get_index(selected_note_index), QItemSelectionModel.Select)

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
        self._update_enabled()

    def unregister_updaters(self):
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        model_update_signals = set([
            'signal_notation_editor_selection', 'signal_notation_editor_notes'])
        if not signals.isdisjoint(model_update_signals):
            self._update_model()
            self._update_enabled()
        if 'signal_notation_editor_note_selection' in signals:
            self._update_enabled()

    def _update_model(self):
        self._list_model = NoteListModel()
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        self.setEnabled(notation_manager.get_editor_selected_notation_id() != None)


class Note(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._cents = QDoubleSpinBox()
        self._cents.setDecimals(0)
        self._cents.setRange(-9999, 9999)

        self._name = QLineEdit()

        el = QHBoxLayout()
        el.setMargin(0)
        el.setSpacing(4)
        el.addWidget(QLabel('Name:'))
        el.addWidget(self._name)
        el.addWidget(QLabel('Cents:'))
        el.addWidget(self._cents)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Current note'))
        v.addLayout(el)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._cents, SIGNAL('valueChanged(double)'), self._change_cents)
        QObject.connect(
                self._name, SIGNAL('textChanged(const QString&)'), self._change_name)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_notes',
            'signal_notation_editor_note_selection'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        note_index = notation_manager.get_editor_selected_note_index()
        if not notation or note_index == None:
            self.setEnabled(False)
            return

        self.setEnabled(True)

        cents, name = notation.get_note(note_index)

        old_block = self._cents.blockSignals(True)
        if self._cents.value() != cents:
            self._cents.setValue(cents)
        self._cents.blockSignals(old_block)

        old_block = self._name.blockSignals(True)
        if self._name.text() != name:
            self._name.setText(name)
        self._name.blockSignals(old_block)

    def _change_cents(self, value):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        notation.set_note_cents(notation_manager.get_editor_selected_note_index(), value)
        self._updater.signal_update(set([
            'signal_notation', 'signal_notation_editor_notes']))

    def _change_name(self, name):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        notation.set_note_name(notation_manager.get_editor_selected_note_index(), name)
        self._updater.signal_update(set([
            'signal_notation', 'signal_notation_editor_notes']))


class Keymap(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._key_count = KeyCount()
        self._key_selector = KeySelector()
        self._key_editor = KeyEditor()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Keymap'))
        v.addWidget(self._key_count)
        v.addWidget(self._key_selector)
        v.addWidget(self._key_editor)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._key_count.set_ui_model(ui_model)
        self._key_selector.set_ui_model(ui_model)
        self._key_editor.set_ui_model(ui_model)

        self._update_enabled()

    def unregister_updaters(self):
        self._key_editor.unregister_updaters()
        self._key_selector.unregister_updaters()
        self._key_count.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()
        if not notation or octave_id == None:
            self.setEnabled(False)
            return

        self.setEnabled(True)


_KEYS_MAX = 33


class KeyCount(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._count = QSpinBox()
        self._count.setRange(0, _KEYS_MAX)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(QLabel('Number of keys:'))
        h.addWidget(self._count)
        h.addStretch(1)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._count, SIGNAL('valueChanged(int)'), self._change_key_count)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()

        if notation and (octave_id != None):
            self.setEnabled(True)
            key_count = notation.get_key_count_in_octave(octave_id)
        else:
            self.setEnabled(False)
            key_count = 0

        old_block = self._count.blockSignals(True)
        if self._count.value() != key_count:
            self._count.setValue(key_count)
        self._count.blockSignals(old_block)

    def _change_key_count(self, new_count):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()

        notation.set_key_count_in_octave(octave_id, new_count)
        self._updater.signal_update(set([
            'signal_notation',
            'signal_notation_editor_key_count']))


class KeyButton(QPushButton):

    def __init__(self, index):
        QPushButton.__init__(self)
        self._ui_model = None

        self._index = index

        self.setFixedSize(QSize(60, 60))
        self.setCheckable(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        QObject.connect(self, SIGNAL('clicked()'), self._select_key)

    def unregister_updaters(self):
        pass

    def set_pressed(self, pressed):
        old_block = self.blockSignals(True)
        self.setChecked(pressed)
        self.blockSignals(old_block)

    def _select_key(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation_manager.set_editor_selected_key_index(self._index)
        self._updater.signal_update(set(['signal_notation_editor_key_selection']))


class KeySelector(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._keys = [KeyButton(i) for i in range(_KEYS_MAX)]

        top_row = QHBoxLayout()
        top_row.setMargin(0)
        top_row.setSpacing(2)
        top_row.addSpacing(31)
        for key in self._keys[1::2]:
            top_row.addWidget(key)
        top_row.addStretch(1)

        bottom_row = QHBoxLayout()
        bottom_row.setMargin(0)
        bottom_row.setSpacing(2)
        for key in self._keys[0::2]:
            bottom_row.addWidget(key)
        bottom_row.addStretch(1)

        rows = QVBoxLayout()
        rows.setMargin(0)
        rows.setSpacing(2)
        rows.addLayout(top_row)
        rows.addLayout(bottom_row)
        self.setLayout(rows)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        for key in self._keys:
            key.set_ui_model(self._ui_model)

    def unregister_updaters(self):
        for key in self._keys:
            key.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_octave_selection',
            'signal_notation_editor_notes',
            'signal_notation_editor_key_count',
            'signal_notation_editor_key_selection',
            'signal_notation_editor_key'])
        if not signals.isdisjoint(update_signals):
            self._update_keys()

    def _update_keys(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()

        texts = [''] * _KEYS_MAX
        if notation and (octave_id != None):
            count = notation.get_key_count_in_octave(octave_id)
            for i in range(count):
                text = '(none)'
                cents = notation.get_key_cents(octave_id, i)
                if cents != None:
                    text = notation.get_full_name(cents)
                texts[i] = text

        selected_index = notation_manager.get_editor_selected_key_index()

        for i, text in enumerate(texts):
            key = self._keys[i]
            key.setText(text)
            key.setEnabled(bool(text))
            key.set_pressed(i == selected_index)


class KeyEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._enabled = QCheckBox()
        self._enabled.setText('Enabled')

        self._cents_label = QLabel('Cents:')

        self._cents = QDoubleSpinBox()
        self._cents.setDecimals(0)
        self._cents.setRange(-9999, 9999)

        self._note_selector_label = QLabel('Select note:')

        self._note_selector = KeyNoteSelector()

        el = QHBoxLayout()
        el.setMargin(0)
        el.setSpacing(4)
        el.addWidget(self._enabled)
        el.addWidget(self._cents_label)
        el.addWidget(self._cents)
        el.addWidget(self._note_selector_label)
        el.addWidget(self._note_selector)
        el.addStretch(1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Key'))
        v.addLayout(el)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._note_selector.set_ui_model(ui_model)

        QObject.connect(self._enabled, SIGNAL('stateChanged(int)'), self._set_enabled)
        QObject.connect(self._cents, SIGNAL('valueChanged(double)'), self._set_cents)

        self._update_all()

    def unregister_updaters(self):
        self._note_selector.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection',
            'signal_notation_editor_key_selection',
            'signal_notation_editor_key'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()
        key_index = notation_manager.get_editor_selected_key_index()

        if not notation or octave_id == None or key_index == None:
            self.setEnabled(False)
            return

        self.setEnabled(True)

        new_cents = notation.get_key_cents(octave_id, key_index)

        old_block = self._enabled.blockSignals(True)
        self._enabled.setChecked(new_cents != None)
        self._enabled.blockSignals(old_block)

        old_block = self._cents.blockSignals(True)
        self._cents.setEnabled(True)
        if new_cents != None:
            self._cents_label.setEnabled(True)
            self._cents.setEnabled(True)
            self._note_selector_label.setEnabled(True)
            self._note_selector.setEnabled(True)
            if self._cents.value() != new_cents:
                self._cents.setValue(new_cents)
        else:
            self._cents_label.setEnabled(False)
            self._cents.setEnabled(False)
            self._note_selector_label.setEnabled(False)
            self._note_selector.setEnabled(False)
        self._cents.blockSignals(old_block)

    def _set_enabled(self, state):
        enabled = (state == Qt.Checked)

        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()
        key_index = notation_manager.get_editor_selected_key_index()

        if enabled:
            new_cents = self._cents.value() or 0
            notation.set_key_cents(octave_id, key_index, new_cents)
        else:
            notation.set_key_cents(octave_id, key_index, None)

        self._updater.signal_update(set([
            'signal_notation', 'signal_notation_editor_key']))

    def _set_cents(self, cents):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()
        key_index = notation_manager.get_editor_selected_key_index()

        notation.set_key_cents(octave_id, key_index, cents)
        self._updater.signal_update(set([
            'signal_notation', 'signal_notation_editor_key']))


class KeyNoteSelector(QComboBox):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('activated(int)'), self._select_note)

        self._update_notes()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        notes_update_signals = set([
            'signal_notation_editor_selection', 'signal_notation_editor_notes'])
        if not signals.isdisjoint(notes_update_signals):
            self._update_notes()
            return

        selection_update_signals = set([
            'signal_notation_editor_selection',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection',
            'signal_notation_editor_key_selection',
            'signal_notation_editor_key'])
        if not signals.isdisjoint(selection_update_signals):
            self._update_selection()

    def _update_notes(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()

        old_block = self.blockSignals(True)
        self.clear()
        if notation:
            for note in notation.get_notes():
                cents, name = note
                self.addItem(name, cents)
            self._update_selection()
        self.blockSignals(old_block)

    def _update_selection(self):
        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()
        key_index = notation_manager.get_editor_selected_key_index()

        if not notation or octave_id == None or key_index == None:
            return

        cents = notation.get_key_cents(octave_id, key_index)
        if cents == None:
            return

        name, _ = notation.get_note_name_and_offset(cents)
        index = self.findText(name)
        if index != -1:
            old_block = self.blockSignals(True)
            self.setCurrentIndex(index)
            self.blockSignals(old_block)

    def _select_note(self, index):
        if index == -1:
            return

        notation_manager = self._ui_model.get_notation_manager()
        notation = notation_manager.get_editor_selected_notation()
        octave_id = notation_manager.get_editor_selected_octave_id()
        key_index = notation_manager.get_editor_selected_key_index()

        cents = self.itemData(index)
        notation.set_key_cents(octave_id, key_index, cents)
        self._updater.signal_update(set([
            'signal_notation', 'signal_notation_editor_key']))


