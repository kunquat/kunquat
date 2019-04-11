# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from kunquat.tracker.ui.qt import *

from .headerline import HeaderLine
from .iconbutton import IconButton
from .kqtcombobox import KqtComboBox
from .updater import Updater
from .varprecspinbox import VarPrecSpinBox


class NotationEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._notations = Notations()
        self._tuning_tables = TuningTables()
        self._template = Template()
        self._octaves = Octaves()
        self._notes = Notes()
        self._note = Note()
        self._keymap = Keymap()

        self.add_to_updaters(
                self._notations,
                self._tuning_tables,
                self._template,
                self._octaves,
                self._notes,
                self._note,
                self._keymap)

        self._tables_layout = QVBoxLayout()
        self._tables_layout.setContentsMargins(0, 0, 0, 0)
        self._tables_layout.setSpacing(2)
        self._tables_layout.addWidget(self._notations)
        self._tables_layout.addWidget(self._tuning_tables)

        self._notes_layout = QVBoxLayout()
        self._notes_layout.setContentsMargins(0, 0, 0, 0)
        self._notes_layout.setSpacing(2)
        self._notes_layout.addWidget(self._notes)
        self._notes_layout.addWidget(self._note)

        self._lists_layout = QHBoxLayout()
        self._lists_layout.setContentsMargins(0, 0, 0, 0)
        self._lists_layout.setSpacing(2)
        self._lists_layout.addWidget(self._template)
        self._lists_layout.addWidget(self._octaves)
        self._lists_layout.addLayout(self._notes_layout)

        self._editor_layout = QVBoxLayout()
        self._editor_layout.setContentsMargins(0, 0, 0, 0)
        self._editor_layout.setSpacing(2)
        self._editor_layout.addLayout(self._lists_layout)
        self._editor_layout.addWidget(self._keymap)

        separator = QFrame()
        separator.setFrameShape(QFrame.VLine)
        separator.setFrameShadow(QFrame.Sunken)
        separator.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.MinimumExpanding)
        separator.setMinimumWidth(2)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addLayout(self._tables_layout)
        h.addWidget(separator)
        h.addLayout(self._editor_layout, 1)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        for layout in (self._tables_layout,
                self._notes_layout,
                self._lists_layout,
                self._editor_layout):
            layout.setSpacing(style_mgr.get_scaled_size_param('small_padding'))

        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


class NotationListToolBar(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._add_button = IconButton(flat=True)
        self._add_button.setToolTip('Add notation')
        self._add_button.setEnabled(True)

        self._remove_button = IconButton(flat=True)
        self._remove_button.setToolTip('Remove notation')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

        self.add_to_updaters(self._add_button, self._remove_button)

    def _on_setup(self):
        self.register_action('signal_notation_list', self._update_enabled)
        self.register_action('signal_notation_editor_selection', self._update_enabled)

        self._add_button.set_icon('add')
        self._remove_button.set_icon('remove')

        self._add_button.clicked.connect(self._add_notation)
        self._remove_button.clicked.connect(self._remove_notation)

        self._update_enabled()

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        has_custom_notations = len(notation_mgr.get_custom_notation_ids()) > 0
        has_selected_notation = bool(notation_mgr.get_editor_selected_notation_id())
        self._remove_button.setEnabled(has_custom_notations and has_selected_notation)

    def _add_notation(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation_mgr.add_custom_notation()
        self._updater.signal_update('signal_notation_list')

    def _remove_notation(self):
        notation_mgr = self._ui_model.get_notation_manager()
        selected_notation_id = notation_mgr.get_editor_selected_notation_id()
        notation_mgr.remove_custom_notation(selected_notation_id)
        notation_mgr.set_editor_selected_notation_id(None)
        notation_mgr.set_editor_selected_octave_id(None)
        notation_mgr.set_editor_selected_note_index(None)
        notation_mgr.set_editor_selected_key_index(None)
        notation_mgr.set_editor_selected_template_note(None)
        self._updater.signal_update(
            'signal_notation',
            'signal_notation_list',
            'signal_notation_editor_selection')


class NotationListModel(QAbstractListModel, Updater):

    def __init__(self):
        super().__init__()
        self._items = []

    def _on_setup(self):
        self._make_items()

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation_ids = notation_mgr.get_custom_notation_ids()

        self._items = list((nid, notation_mgr.get_notation(nid).get_name())
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
                notation_mgr = self._ui_model.get_notation_manager()
                notation = notation_mgr.get_editor_selected_notation()
                notation.set_name(new_name)
                self._updater.signal_update('signal_notation_list')
                return True

        return False


class NotationListView(QListView, Updater):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)
        self.setMinimumWidth(100)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.setMinimumWidth(style_mgr.get_scaled_size(16))

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item:
            notation_id, _ = item
            notation_mgr = self._ui_model.get_notation_manager()
            notation_mgr.set_editor_selected_notation_id(notation_id)
            notation_mgr.set_editor_selected_octave_id(None)
            notation_mgr.set_editor_selected_note_index(None)
            notation_mgr.set_editor_selected_key_index(None)
            notation_mgr.set_editor_selected_template_note(None)
            self._updater.signal_update('signal_notation_editor_selection')

    def setModel(self, model):
        super().setModel(model)

        selection_model = self.selectionModel()

        notation_mgr = self._ui_model.get_notation_manager()
        selected_notation_id = notation_mgr.get_editor_selected_notation_id()
        if selected_notation_id:
            selection_model.select(
                    model.get_index(selected_notation_id), QItemSelectionModel.Select)

        selection_model.currentChanged.connect(
                self._select_entry, type=Qt.QueuedConnection)


class Notations(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._toolbar = NotationListToolBar()

        self._list_model = None
        self._list_view = NotationListView()

        self._header = HeaderLine('Custom notations')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._toolbar, self._list_view)
        self.register_action('signal_notation_list', self._update_model)
        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_model()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_model(self):
        if self._list_model:
            self.remove_from_updaters(self._list_model)
        self._list_model = NotationListModel()
        self.add_to_updaters(self._list_model)
        self._list_view.setModel(self._list_model)


class TuningTableListToolBar(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._add_button = IconButton(flat=True)
        self._add_button.setToolTip('Add tuning table')
        self._add_button.setEnabled(True)

        self._remove_button = IconButton(flat=True)
        self._remove_button.setToolTip('Remove tuning table')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

        self.add_to_updaters(self._add_button, self._remove_button)

    def _on_setup(self):
        self.register_action('signal_tuning_tables', self._update_enabled)
        self.register_action('signal_tuning_table_selection', self._update_enabled)

        self._add_button.set_icon('add')
        self._remove_button.set_icon('remove')

        self._add_button.clicked.connect(self._add_tuning_table)
        self._remove_button.clicked.connect(self._remove_tuning_table)

        self._update_enabled()

    def _update_enabled(self):
        module = self._ui_model.get_module()
        self._add_button.setEnabled(module.get_free_tuning_table_id() != None)

        notation_mgr = self._ui_model.get_notation_manager()
        selected_table_id = notation_mgr.get_editor_selected_tuning_table_id()
        self._remove_button.setEnabled(selected_table_id != None)

    def _add_tuning_table(self):
        module = self._ui_model.get_module()
        module.add_tuning_table(module.get_free_tuning_table_id())
        self._updater.signal_update('signal_tuning_tables')

    def _remove_tuning_table(self):
        notation_mgr = self._ui_model.get_notation_manager()
        selected_table_id = notation_mgr.get_editor_selected_tuning_table_id()
        module = self._ui_model.get_module()
        table = module.get_tuning_table(selected_table_id)
        table.remove()
        notation_mgr.set_editor_selected_tuning_table_id(None)
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.hide_tuning_table_editor(selected_table_id)
        self._updater.signal_update(
                'signal_tuning_tables', 'signal_tuning_table_selection')


class TuningTableListModel(QAbstractTableModel, Updater):

    def __init__(self):
        super().__init__()
        self._items = []

    def _on_setup(self):
        self._make_items()

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
                self._updater.signal_update('signal_tuning_tables')
                return True

        return False


class TuningTableListView(QTableView, Updater):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)

        self.setMinimumWidth(100)

        hheader = self.horizontalHeader()
        hheader.setStretchLastSection(True)
        hheader.hide()

        self.verticalHeader().setSectionResizeMode(QHeaderView.Fixed)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.setMinimumWidth(style_mgr.get_scaled_size(16))

        self.verticalHeader().setDefaultSectionSize(style_mgr.get_scaled_size(2.5))

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item:
            table_id, _ = item
            notation_mgr = self._ui_model.get_notation_manager()
            notation_mgr.set_editor_selected_tuning_table_id(table_id)
            self._updater.signal_update('signal_tuning_table_selection')

    def setModel(self, model):
        super().setModel(model)

        selection_model = self.selectionModel()

        notation_mgr = self._ui_model.get_notation_manager()
        selected_table_id = notation_mgr.get_editor_selected_tuning_table_id()
        if selected_table_id:
            selection_model.select(
                    model.get_index(selected_table_id), QItemSelectionModel.Select)

        selection_model.currentChanged.connect(self._select_entry)


class TuningTables(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._toolbar = TuningTableListToolBar()

        self._list_model = None
        self._list_view = TuningTableListView()

        self._edit_button = QPushButton('Edit tuning table')
        self._edit_button.setEnabled(False)

        self._header = HeaderLine('Tuning tables')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        v.addWidget(self._edit_button)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._toolbar, self._list_view)
        self.register_action('signal_tuning_tables', self._update_model)
        self.register_action('signal_tuning_table_selection', self._update_selection)
        self.register_action('signal_style_changed', self._update_style)

        self._edit_button.clicked.connect(self._open_editor)

        self._update_style()
        self._update_model()
        self._update_selection()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_model(self):
        if self._list_model:
            self.remove_from_updaters(self._list_model)
        self._list_model = TuningTableListModel()
        self.add_to_updaters(self._list_model)
        self._list_view.setModel(self._list_model)

    def _update_selection(self):
        notation_mgr = self._ui_model.get_notation_manager()
        selected_table_id = notation_mgr.get_editor_selected_tuning_table_id()
        self._edit_button.setEnabled(selected_table_id != None)

    def _open_editor(self):
        notation_mgr = self._ui_model.get_notation_manager()
        selected_table_id = notation_mgr.get_editor_selected_tuning_table_id()
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.show_tuning_table_editor(selected_table_id)


class Template(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._centre_pitch = CentrePitch()
        self._octave_ratio = OctaveRatio()
        self._octaves = TemplateOctaves()
        self._notes = TemplateNotes()
        self._create_button = QPushButton('Create notation and keymap')
        self._create_tt_button = QPushButton('Create tuning table')

        self._header = HeaderLine('Template')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._centre_pitch)
        v.addWidget(self._octave_ratio)
        v.addWidget(self._octaves)
        v.addWidget(self._notes)
        v.addWidget(self._create_button)
        v.addWidget(self._create_tt_button)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(
                self._centre_pitch,
                self._octave_ratio,
                self._octaves,
                self._notes)
        self.register_action('signal_notation_list', self._update_enabled)
        self.register_action('signal_notation_editor_selection', self._update_enabled)
        self.register_action('signal_notation_template_notes', self._update_enabled)
        self.register_action('signal_tuning_tables', self._update_enabled)

        self.register_action('signal_style_changed', self._update_style)

        self._create_button.clicked.connect(self._create)
        self._create_tt_button.clicked.connect(self._create_tuning_table)

        self._update_style()
        self._update_enabled()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
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
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        notation.apply_template_settings()
        notation_mgr.set_editor_selected_octave_id(None)
        notation_mgr.set_editor_selected_note_index(None)
        notation_mgr.set_editor_selected_key_index(None)

        typewriter_mgr = self._ui_model.get_typewriter_manager()
        if ((notation_mgr.get_editor_selected_notation_id() ==
                    notation_mgr.get_selected_notation_id()) and
                typewriter_mgr.get_octave() >= notation.get_octave_count()):
            typewriter_mgr.set_octave(notation.get_octave_count() - 1)

        self._updater.signal_update(
            'signal_notation',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection',
            'signal_notation_editor_notes',
            'signal_notation_editor_note_selection',
            'signal_notation_editor_key_count',
            'signal_notation_editor_key_selection')

    def _create_tuning_table(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        name = notation.get_name()
        template = notation.get_template()
        module = self._ui_model.get_module()
        table_id = module.get_free_tuning_table_id()
        assert table_id != None
        module.create_tuning_table_from_notation_template(table_id, name, template)
        self._updater.signal_update('signal_tuning_tables')


class CentrePitch(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._value = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._value.setRange(-9999, 9999)
        self._value.setValue(0)

        self._units = KqtComboBox()
        self._units.addItem('cents')
        self._units.addItem('Hz')
        self._units.setCurrentIndex(self._units.findText('cents'))

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(QLabel('Centre pitch:'), 0)
        h.addWidget(self._value, 1)
        h.addWidget(self._units, 1)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_notation_list', self._update_all)
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_template_centre_pitch', self._update_all)

        self.register_action('signal_style_changed', self._update_style)

        self._value.valueChanged.connect(self._change_centre)
        self._units.currentIndexChanged.connect(self._change_units)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._value.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_all(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        if not notation:
            return

        value, units = notation.get_template().get_centre_pitch()

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

    def _change_centre(self, new_centre):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()

        _, units = template.get_centre_pitch()
        template.set_centre_pitch(new_centre, units)
        self._updater.signal_update('signal_notation_template_centre_pitch')

    def _get_cents(self, hz):
        return math.log(hz / 440.0, 2) * 1200

    def _get_hz(self, cents):
        return 2**(cents / 1200.0) * 440

    def _change_units(self, item_index):
        new_units = str(self._units.itemText(item_index))

        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()

        value, units = template.get_centre_pitch()
        if units == new_units:
            return

        if new_units == 'cents':
            new_value = self._get_cents(value)
        elif new_units == 'Hz':
            new_value = self._get_hz(value)

        template.set_centre_pitch(new_value, new_units)
        self._updater.signal_update('signal_notation_template_centre_pitch')


class RatioValidator(QValidator):

    def __init__(self):
        super().__init__()

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


class OctaveRatio(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._ratio = QLineEdit()
        self._ratio.setValidator(RatioValidator())

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(QLabel('Octave ratio:'))
        h.addWidget(self._ratio)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_notation_list', self._update_all)
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_template_octave_ratio', self._update_all)

        self.register_action('signal_style_changed', self._update_style)

        self._ratio.editingFinished.connect(self._change_ratio)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_all(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
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

        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()

        template.set_octave_ratio(value)
        self._updater.signal_update('signal_notation_template_octave_ratio')


class TemplateOctaves(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._lowest = QSpinBox()
        self._lowest.setRange(0, 15)
        self._centre = QSpinBox()
        self._centre.setRange(0, 15)
        self._highest = QSpinBox()
        self._highest.setRange(0, 15)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(QLabel('Lowest octave:'))
        h.addWidget(self._lowest)
        h.addWidget(QLabel('Centre:'))
        h.addWidget(self._centre)
        h.addWidget(QLabel('Highest:'))
        h.addWidget(self._highest)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_notation_list', self._update_all)
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_template_octaves', self._update_all)

        self.register_action('signal_style_changed', self._update_style)

        self._lowest.valueChanged.connect(self._change_lowest)
        self._centre.valueChanged.connect(self._change_centre)
        self._highest.valueChanged.connect(self._change_highest)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_all(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        if not notation:
            return

        lowest, centre, highest = notation.get_template().get_octaves()

        old_block = self._lowest.blockSignals(True)
        if self._lowest.value() != lowest:
            self._lowest.setValue(lowest)
        self._lowest.blockSignals(old_block)

        old_block = self._centre.blockSignals(True)
        self._centre.setRange(lowest, highest)
        if self._centre.value() != centre:
            self._centre.setValue(centre)
        self._centre.blockSignals(old_block)

        old_block = self._highest.blockSignals(True)
        if self._highest.value() != highest:
            self._highest.setValue(highest)
        self._highest.blockSignals(old_block)

    def _change_lowest(self, value):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()

        octaves = template.get_octaves()
        octaves[0] = value
        octaves[1] = max(octaves[0], octaves[1])
        octaves[2] = max(octaves[0], octaves[2])

        template.set_octaves(*octaves)
        self._updater.signal_update('signal_notation_template_octaves')

    def _change_centre(self, value):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()

        octaves = template.get_octaves()
        octaves[1] = value
        assert octaves[0] <= octaves[1] <= octaves[2]

        template.set_octaves(*octaves)
        self._updater.signal_update('signal_notation_template_octaves')

    def _change_highest(self, value):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()

        octaves = template.get_octaves()
        octaves[2] = value
        octaves[1] = min(octaves[1], octaves[2])
        octaves[0] = min(octaves[0], octaves[2])

        template.set_octaves(*octaves)
        self._updater.signal_update('signal_notation_template_octaves')


class TemplateNotesToolBar(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._add_button = IconButton(flat=True)
        self._add_button.setToolTip('Add note')
        self._add_button.setEnabled(True)

        self._remove_button = IconButton(flat=True)
        self._remove_button.setToolTip('Remove note')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

        self.add_to_updaters(self._add_button, self._remove_button)

    def _on_setup(self):
        self.register_action('signal_notation_list', self._update_enabled)
        self.register_action('signal_notation_editor_selection', self._update_enabled)
        self.register_action(
                'signal_notation_template_note_selection', self._update_enabled)
        self.register_action('signal_notation_template_notes', self._update_enabled)

        self._add_button.set_icon('add')
        self._remove_button.set_icon('remove')

        self._add_button.clicked.connect(self._add_note)
        self._remove_button.clicked.connect(self._remove_note)

        self._update_enabled()

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        selected_note_coords = notation_mgr.get_editor_selected_template_note()
        notation = notation_mgr.get_editor_selected_notation()
        self._remove_button.setEnabled(bool(notation and selected_note_coords))

    def _add_note(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()
        template.add_note()
        self._updater.signal_update('signal_notation_template_notes')

    def _remove_note(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        template = notation.get_template()
        index, column = notation_mgr.get_editor_selected_template_note()
        template.remove_note(index)
        note_count = template.get_note_count()
        if note_count > 0:
            notation_mgr.set_editor_selected_template_note(
                    (min(index, template.get_note_count() - 1), column))
        else:
            notation_mgr.set_editor_selected_template_note(None)
        self._updater.signal_update('signal_notation_template_notes')


class TemplateNoteTableModel(QAbstractTableModel, Updater):

    def __init__(self):
        super().__init__()
        self._items = []

    def _on_setup(self):
        self._make_items()

    def get_index(self, row, column):
        return self.createIndex(row, column, None)

    def _make_items(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()

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
        default_flags = super().flags(index)
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
                    notation_mgr = self._ui_model.get_notation_manager()
                    notation = notation_mgr.get_editor_selected_notation()
                    template = notation.get_template()
                    template.set_note_name(row, new_name)
                    self._updater.signal_update('signal_notation_template_notes')
                    return True
                elif column == 1:
                    new_ratio = self._get_validated_ratio(value)
                    if new_ratio == None:
                        return False
                    notation_mgr = self._ui_model.get_notation_manager()
                    notation = notation_mgr.get_editor_selected_notation()
                    template = notation.get_template()
                    template.set_note_ratio(row, new_ratio)
                    self._updater.signal_update('signal_notation_template_notes')
                    return True

        return False


class TemplateNoteTableView(QTableView, Updater):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)
        self.setMinimumWidth(100)

        header = self.horizontalHeader()
        header.setStretchLastSection(True)

        self.verticalHeader().setSectionResizeMode(QHeaderView.Fixed)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.setMinimumWidth(style_mgr.get_scaled_size(16))

        self.verticalHeader().setDefaultSectionSize(style_mgr.get_scaled_size(2.5))

    def _select_entry(self, cur_index, prev_index):
        if not cur_index.isValid():
            return
        row, column = cur_index.row(), cur_index.column()

        notation_mgr = self._ui_model.get_notation_manager()
        notation_mgr.set_editor_selected_template_note((row, column))
        self._updater.signal_update('signal_notation_template_note_selection')

    def setModel(self, model):
        super().setModel(model)

        selection_model = self.selectionModel()

        notation_mgr = self._ui_model.get_notation_manager()
        coords = notation_mgr.get_editor_selected_template_note()
        if coords:
            row, column = coords
            selection_model.select(
                    model.get_index(row, column), QItemSelectionModel.Select)

        selection_model.currentChanged.connect(self._select_entry)


class TemplateNotes(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._toolbar = TemplateNotesToolBar()

        self._table_model = None
        self._table_view = TemplateNoteTableView()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._toolbar)
        v.addWidget(self._table_view)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._toolbar, self._table_view)
        self.register_action('signal_notation_list', self._update_model)
        self.register_action('signal_notation_editor_selection', self._update_model)
        self.register_action('signal_notation_template_notes', self._update_model)

        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_model()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_model(self):
        if self._table_model:
            self.remove_from_updaters(self._table_model)
        self._table_model = TemplateNoteTableModel()
        self.add_to_updaters(self._table_model)
        self._table_view.setModel(self._table_model)


class OctaveListToolBar(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._add_button = IconButton(flat=True)
        self._add_button.setToolTip('Add octave')
        self._add_button.setEnabled(True)

        self._remove_button = IconButton(flat=True)
        self._remove_button.setToolTip('Remove octave')
        self._remove_button.setEnabled(False)

        self._set_base_button = QToolButton()
        self._set_base_button.setText('Set base')
        self._set_base_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)
        self.addWidget(self._set_base_button)

        self.add_to_updaters(self._add_button, self._remove_button)

    def _on_setup(self):
        self.register_action('signal_notation_editor_selection', self._update_enabled)
        self.register_action('signal_notation_editor_octaves', self._update_enabled)
        self.register_action(
                'signal_notation_editor_octave_selection', self._update_enabled)

        self._add_button.set_icon('add')
        self._remove_button.set_icon('remove')

        self._add_button.clicked.connect(self._add_octave)
        self._remove_button.clicked.connect(self._remove_octave)
        self._set_base_button.clicked.connect(self._set_base_octave)

        self._update_enabled()

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        if not notation:
            self.setEnabled(False)
            return

        self.setEnabled(True)
        has_selected_octave = notation_mgr.get_editor_selected_octave_id() != None
        self._remove_button.setEnabled(
                has_selected_octave and notation.get_octave_count() > 1)
        self._set_base_button.setEnabled(has_selected_octave)

    def _add_octave(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        notation.add_octave()
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_octaves')

    def _remove_octave(self):
        notation_mgr = self._ui_model.get_notation_manager()
        selected_octave_id = notation_mgr.get_editor_selected_octave_id()
        notation = notation_mgr.get_editor_selected_notation()
        notation.remove_octave(selected_octave_id)
        base_octave_id = notation.get_base_octave_id()
        if base_octave_id > 0 and base_octave_id >= selected_octave_id:
            notation.set_base_octave_id(base_octave_id - 1)
        notation_mgr.set_editor_selected_octave_id(max(0, selected_octave_id - 1))
        notation_mgr.set_editor_selected_key_index(None)
        self._updater.signal_update(
            'signal_notation',
            'signal_notation_editor_octaves',
            'signal_notation_editor_octave_selection')

    def _set_base_octave(self):
        notation_mgr = self._ui_model.get_notation_manager()
        selected_octave_id = notation_mgr.get_editor_selected_octave_id()
        notation = notation_mgr.get_editor_selected_notation()
        notation.set_base_octave_id(selected_octave_id)
        self._updater.signal_update('signal_notation_editor_octaves')


class OctaveListModel(QAbstractListModel, Updater):

    def __init__(self):
        super().__init__()
        self._items = []

    def _on_setup(self):
        self._make_items()

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()

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
                    notation_mgr = self._ui_model.get_notation_manager()
                    notation = notation_mgr.get_editor_selected_notation()
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
                notation_mgr = self._ui_model.get_notation_manager()
                notation = notation_mgr.get_editor_selected_notation()
                notation.set_octave_name(index.row(), new_name)
                self._updater.signal_update(
                    'signal_notation', 'signal_notation_editor_octaves')
                return True

        return False


class OctaveListView(QListView, Updater):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)
        self.setMinimumWidth(100)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.setMinimumWidth(style_mgr.get_scaled_size(16))

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item != None:
            notation_mgr = self._ui_model.get_notation_manager()
            notation_mgr.set_editor_selected_octave_id(cur_index.row())
            notation_mgr.set_editor_selected_key_index(None)
            self._updater.signal_update('signal_notation_editor_octave_selection')

    def setModel(self, model):
        super().setModel(model)

        selection_model = self.selectionModel()

        notation_mgr = self._ui_model.get_notation_manager()
        selected_octave_id = notation_mgr.get_editor_selected_octave_id()
        if selected_octave_id != None:
            selection_model.select(
                    model.get_index(selected_octave_id), QItemSelectionModel.Select)

        selection_model.currentChanged.connect(self._select_entry)


class Octaves(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._toolbar = OctaveListToolBar()

        self._list_model = None
        self._list_view = OctaveListView()

        self._header = HeaderLine('Octaves')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._toolbar, self._list_view)
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_editor_octaves', self._update_all)
        self.register_action(
                'signal_notation_editor_octave_selection', self._update_enabled)

        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_model()
        self._update_enabled()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_all(self):
        self._update_model()
        self._update_enabled()

    def _update_model(self):
        if self._list_model:
            self.remove_from_updaters(self._list_model)
        self._list_model = OctaveListModel()
        self.add_to_updaters(self._list_model)
        self._list_view.setModel(self._list_model)

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        self.setEnabled(notation_mgr.get_editor_selected_notation_id() != None)


class NoteListToolBar(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._add_button = IconButton(flat=True)
        self._add_button.setToolTip('Add note')
        self._add_button.setEnabled(True)

        self._remove_button = IconButton(flat=True)
        self._remove_button.setToolTip('Remove note')
        self._remove_button.setEnabled(False)

        self.addWidget(self._add_button)
        self.addWidget(self._remove_button)

        self.add_to_updaters(self._add_button, self._remove_button)

    def _on_setup(self):
        self.register_action('signal_notation_editor_selection', self._update_enabled)
        self.register_action('signal_notation_editor_notes', self._update_enabled)
        self.register_action(
                'signal_notation_editor_note_selection', self._update_enabled)

        self._add_button.set_icon('add')
        self._remove_button.set_icon('remove')

        self._add_button.clicked.connect(self._add_note)
        self._remove_button.clicked.connect(self._remove_note)

        self._update_enabled()

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        if not notation:
            self.setEnabled(False)
            return

        self.setEnabled(True)
        has_selected_note = notation_mgr.get_editor_selected_note_index() != None
        has_notes = bool(notation.get_notes())
        self._remove_button.setEnabled(has_selected_note and has_notes)

    def _add_note(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        notation.add_note()
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_notes')

    def _remove_note(self):
        notation_mgr = self._ui_model.get_notation_manager()
        note_index = notation_mgr.get_editor_selected_note_index()
        notation = notation_mgr.get_editor_selected_notation()
        notation.remove_note(note_index)
        notation_mgr.set_editor_selected_note_index(None)
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_notes')


class NoteListModel(QAbstractListModel, Updater):

    def __init__(self):
        super().__init__()
        self._items = []

    def _on_setup(self):
        self._make_items()

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()

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


class NoteListView(QListView, Updater):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)
        self.setMinimumWidth(100)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.setMinimumWidth(style_mgr.get_scaled_size(16))

    def _select_entry(self, cur_index, prev_index):
        item = self.model().get_item(cur_index)
        if item != None:
            notation_mgr = self._ui_model.get_notation_manager()
            notation_mgr.set_editor_selected_note_index(cur_index.row())
            self._updater.signal_update('signal_notation_editor_note_selection')

    def setModel(self, model):
        super().setModel(model)

        selection_model = self.selectionModel()

        notation_mgr = self._ui_model.get_notation_manager()
        selected_note_index = notation_mgr.get_editor_selected_note_index()
        if selected_note_index != None:
            selection_model.select(
                    model.get_index(selected_note_index), QItemSelectionModel.Select)

        selection_model.currentChanged.connect(
                self._select_entry, type=Qt.QueuedConnection)


class Notes(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._toolbar = NoteListToolBar()

        self._list_model = None
        self._list_view = NoteListView()

        self._header = HeaderLine('Notes')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._toolbar, self._list_view)
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_editor_notes', self._update_all)
        self.register_action(
                'signal_notation_editor_note_selection', self._update_enabled)

        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_model()
        self._update_enabled()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_all(self):
        self._update_model()
        self._update_enabled()

    def _update_model(self):
        if self._list_model:
            self.remove_from_updaters(self._list_model)
        self._list_model = NoteListModel()
        self.add_to_updaters(self._list_model)
        self._list_view.setModel(self._list_model)

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        self.setEnabled(notation_mgr.get_editor_selected_notation_id() != None)


class Note(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._cents = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._cents.setRange(-9999, 9999)

        self._name = QLineEdit()

        self._editor_layout = QHBoxLayout()
        self._editor_layout.setContentsMargins(0, 0, 0, 0)
        self._editor_layout.setSpacing(4)
        self._editor_layout.addWidget(QLabel('Name:'))
        self._editor_layout.addWidget(self._name)
        self._editor_layout.addWidget(QLabel('Cents:'))
        self._editor_layout.addWidget(self._cents)

        self._header = HeaderLine('Current note')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addLayout(self._editor_layout)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_editor_notes', self._update_all)
        self.register_action('signal_notation_editor_note_selection', self._update_all)

        self.register_action('signal_style_changed', self._update_style)

        self._cents.valueChanged.connect(self._change_cents)
        self._name.textChanged.connect(self._change_name)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._cents.update_style(style_mgr)
        self._header.update_style(style_mgr)
        self._editor_layout.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_all(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        note_index = notation_mgr.get_editor_selected_note_index()
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
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        notation.set_note_cents(notation_mgr.get_editor_selected_note_index(), value)
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_notes')

    def _change_name(self, name):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        notation.set_note_name(notation_mgr.get_editor_selected_note_index(), name)
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_notes')


class Keymap(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._key_count = KeyCount()
        self._key_selector = KeySelector()
        self._key_editor = KeyEditor()

        self._header = HeaderLine('Keymap')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._key_count)
        v.addWidget(self._key_selector)
        v.addWidget(self._key_editor)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._key_count, self._key_selector, self._key_editor)
        self.register_action('signal_notation_editor_selection', self._update_enabled)
        self.register_action('signal_notation_editor_octaves', self._update_enabled)
        self.register_action(
                'signal_notation_editor_octave_selection', self._update_enabled)

        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_enabled()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_enabled(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()
        if not notation or octave_id == None:
            self.setEnabled(False)
            return

        self.setEnabled(True)


_KEYS_MAX = 33


class KeyCount(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._count = QSpinBox()
        self._count.setRange(0, _KEYS_MAX)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(QLabel('Number of keys:'))
        h.addWidget(self._count)
        h.addStretch(1)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_editor_octaves', self._update_all)
        self.register_action('signal_notation_editor_octave_selection', self._update_all)

        self.register_action('signal_style_changed', self._update_style)

        self._count.valueChanged.connect(self._change_key_count)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_all(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()

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
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()

        notation.set_key_count_in_octave(octave_id, new_count)
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_key_count')


class KeyButton(QPushButton, Updater):

    def __init__(self, index):
        super().__init__()
        self._index = index

        self.setFixedSize(QSize(60, 60))
        self.setCheckable(True)

    def _on_setup(self):
        self.clicked.connect(self._select_key)

    def set_pressed(self, pressed):
        old_block = self.blockSignals(True)
        self.setChecked(pressed)
        self.blockSignals(old_block)

    def _select_key(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation_mgr.set_editor_selected_key_index(self._index)
        self._updater.signal_update('signal_notation_editor_key_selection')


class KeySelector(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._keys = [KeyButton(i) for i in range(_KEYS_MAX)]

        top_row = QHBoxLayout()
        top_row.setContentsMargins(0, 0, 0, 0)
        top_row.setSpacing(2)
        top_row.addSpacing(31)
        for key in self._keys[1::2]:
            top_row.addWidget(key)
        top_row.addStretch(1)

        bottom_row = QHBoxLayout()
        bottom_row.setContentsMargins(0, 0, 0, 0)
        bottom_row.setSpacing(2)
        for key in self._keys[0::2]:
            bottom_row.addWidget(key)
        bottom_row.addStretch(1)

        rows = QVBoxLayout()
        rows.setContentsMargins(0, 0, 0, 0)
        rows.setSpacing(2)
        rows.addLayout(top_row)
        rows.addLayout(bottom_row)
        self.setLayout(rows)

    def _on_setup(self):
        self.add_to_updaters(*self._keys)
        self.register_action('signal_notation_editor_selection', self._update_keys)
        self.register_action(
                'signal_notation_editor_octave_selection', self._update_keys)
        self.register_action('signal_notation_editor_notes', self._update_keys)
        self.register_action('signal_notation_editor_key_count', self._update_keys)
        self.register_action('signal_notation_editor_key_selection', self._update_keys)
        self.register_action('signal_notation_editor_key', self._update_keys)

        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_keys(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()

        texts = [''] * _KEYS_MAX
        if notation and (octave_id != None):
            count = notation.get_key_count_in_octave(octave_id)
            for i in range(count):
                text = '(none)'
                cents = notation.get_key_cents(octave_id, i)
                if cents != None:
                    text = notation.get_full_name(cents)
                texts[i] = text

        selected_index = notation_mgr.get_editor_selected_key_index()

        for i, text in enumerate(texts):
            key = self._keys[i]
            key.setText(text)
            key.setEnabled(bool(text))
            key.set_pressed(i == selected_index)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        rows = self.layout()
        rows.setSpacing(style_mgr.get_scaled_size_param('small_padding'))

        button_spacing = style_mgr.get_scaled_size_param('medium_padding')

        top_row = rows.itemAt(0).layout()
        top_row.setSpacing(button_spacing)
        padding = top_row.itemAt(0).spacerItem()
        padding.changeSize(style_mgr.get_scaled_size_param('typewriter_padding'), 2)

        bottom_row = rows.itemAt(1).layout()
        bottom_row.setSpacing(button_spacing)

        button_size = style_mgr.get_scaled_size_param('typewriter_button_size')
        for button in self._keys:
            button.setFixedSize(QSize(button_size, button_size))


class KeyEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._enabled = QCheckBox()
        self._enabled.setText('Enabled')

        self._cents_label = QLabel('Cents:')

        self._cents = VarPrecSpinBox(step_decimals=0, max_decimals=2)
        self._cents.setRange(-9999, 9999)

        self._note_selector_label = QLabel('Select note:')

        self._note_selector = KeyNoteSelector()

        self._editor_layout = QHBoxLayout()
        self._editor_layout.setContentsMargins(0, 0, 0, 0)
        self._editor_layout.setSpacing(4)
        self._editor_layout.addWidget(self._enabled)
        self._editor_layout.addWidget(self._cents_label)
        self._editor_layout.addWidget(self._cents)
        self._editor_layout.addWidget(self._note_selector_label)
        self._editor_layout.addWidget(self._note_selector)
        self._editor_layout.addStretch(1)

        self._header = HeaderLine('Key')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addLayout(self._editor_layout)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._note_selector)
        self.register_action('signal_notation_editor_selection', self._update_all)
        self.register_action('signal_notation_editor_octaves', self._update_all)
        self.register_action('signal_notation_editor_octave_selection', self._update_all)
        self.register_action('signal_notation_editor_key_selection', self._update_all)
        self.register_action('signal_notation_editor_key', self._update_all)

        self.register_action('signal_style_changed', self._update_style)

        self._enabled.stateChanged.connect(self._set_enabled)
        self._cents.valueChanged.connect(self._set_cents)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._cents.update_style(style_mgr)
        self._header.update_style(style_mgr)
        self._editor_layout.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_all(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()
        key_index = notation_mgr.get_editor_selected_key_index()

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

        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()
        key_index = notation_mgr.get_editor_selected_key_index()

        if enabled:
            new_cents = self._cents.value() or 0
            notation.set_key_cents(octave_id, key_index, new_cents)
        else:
            notation.set_key_cents(octave_id, key_index, None)

        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_key')

    def _set_cents(self, cents):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()
        key_index = notation_mgr.get_editor_selected_key_index()

        notation.set_key_cents(octave_id, key_index, cents)
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_key')


class KeyNoteSelector(KqtComboBox, Updater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_notation_editor_selection', self._update_notes)
        self.register_action('signal_notation_editor_notes', self._update_notes)
        self.register_action('signal_notation_editor_octaves', self._update_selection)
        self.register_action(
                'signal_notation_editor_octave_selection', self._update_selection)
        self.register_action(
                'signal_notation_editor_key_selection', self._update_selection)
        self.register_action('signal_notation_editor_key', self._update_selection)

        self.activated.connect(self._select_note)

        self._update_notes()

    def _update_notes(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()

        old_block = self.blockSignals(True)
        if notation:
            self.set_items((name, cents) for (cents, name) in notation.get_notes())
            self._update_selection()
        else:
            self.set_items([])
        self.blockSignals(old_block)

    def _update_selection(self):
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()
        key_index = notation_mgr.get_editor_selected_key_index()

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

        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_editor_selected_notation()
        octave_id = notation_mgr.get_editor_selected_octave_id()
        key_index = notation_mgr.get_editor_selected_key_index()

        cents = self.itemData(index)
        notation.set_key_cents(octave_id, key_index, cents)
        self._updater.signal_update(
            'signal_notation', 'signal_notation_editor_key')


