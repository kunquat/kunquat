# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import re

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.kunquat.limits import *
from .kqtcombobox import KqtComboBox
from .notationeditor import RatioValidator
from .updater import Updater


class TuningTableEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._table_id = None

        self._name = QLineEdit()

        self._ref_pitch = QDoubleSpinBox()
        self._ref_pitch.setDecimals(2)
        self._ref_pitch.setRange(-9999, 9999)

        self._pitch_offset = QDoubleSpinBox()
        self._pitch_offset.setDecimals(2)
        self._pitch_offset.setRange(-9999, 9999)

        self._octave_width = QLineEdit()
        self._octave_width.setValidator(RatioValidator())

        self._centre_octave = QSpinBox()
        self._centre_octave.setRange(0, TUNING_TABLE_OCTAVES - 1)

        self._tuning_centre = KqtComboBox()

        self._notes = Notes()

        gl = QGridLayout()
        gl.setContentsMargins(0, 0, 0, 0)
        gl.setHorizontalSpacing(4)
        gl.setVerticalSpacing(2)
        gl.addWidget(QLabel('Name:'), 0, 0)
        gl.addWidget(self._name, 0, 1)
        gl.addWidget(QLabel('Reference pitch:'), 1, 0)
        gl.addWidget(self._ref_pitch, 1, 1)
        gl.addWidget(QLabel('Global pitch offset:'), 2, 0)
        gl.addWidget(self._pitch_offset, 2, 1)
        gl.addWidget(QLabel('Octave width:'), 3, 0)
        gl.addWidget(self._octave_width, 3, 1)
        gl.addWidget(QLabel('Centre octave:'), 4, 0)
        gl.addWidget(self._centre_octave, 4, 1)
        gl.addWidget(QLabel('Tuning centre:'), 5, 0)
        gl.addWidget(self._tuning_centre, 5, 1)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addLayout(gl)
        v.addWidget(self._notes)
        self.setLayout(v)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id
        self._notes.set_tuning_table_id(table_id)

    def _on_setup(self):
        assert self._table_id != None
        self.add_updating_child(self._notes)
        self.register_action('signal_tuning_tables', self._update_name)
        self.register_action(self._get_update_signal_type(), self._update_params)

        QObject.connect(
                self._name, SIGNAL('textChanged(const QString&)'), self._change_name)
        QObject.connect(
                self._ref_pitch, SIGNAL('valueChanged(double)'), self._change_ref_pitch)
        QObject.connect(
                self._pitch_offset,
                SIGNAL('valueChanged(double)'),
                self._change_pitch_offset)
        QObject.connect(
                self._octave_width,
                SIGNAL('editingFinished()'),
                self._change_octave_width)
        QObject.connect(
                self._centre_octave,
                SIGNAL('valueChanged(int)'),
                self._change_centre_octave)
        QObject.connect(
                self._tuning_centre,
                SIGNAL('currentIndexChanged(int)'),
                self._change_tuning_centre)

        self._update_name()
        self._update_params()

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _get_tuning_table(self):
        module = self._ui_model.get_module()
        return module.get_tuning_table(self._table_id)

    def _update_name(self):
        table = self._get_tuning_table()
        name = table.get_name() or ''
        old_block = self._name.blockSignals(True)
        if self._name.text() != name:
            self._name.setText(name)
        self._name.blockSignals(old_block)

    def _update_params(self):
        table = self._get_tuning_table()

        # Reference pitch
        ref_pitch = table.get_ref_pitch()

        old_block = self._ref_pitch.blockSignals(True)
        if self._ref_pitch.value() != ref_pitch:
            self._ref_pitch.setValue(ref_pitch)
        self._ref_pitch.blockSignals(old_block)

        # Global pitch offset
        pitch_offset = table.get_pitch_offset()

        old_block = self._pitch_offset.blockSignals(True)
        if self._pitch_offset.value() != pitch_offset:
            self._pitch_offset.setValue(pitch_offset)
        self._pitch_offset.blockSignals(old_block)

        # Octave width
        octave_width = table.get_octave_width()
        if isinstance(octave_width, list):
            octave_width_text = '{}/{}'.format(*octave_width)
        else:
            octave_width_text = '{}'.format(octave_width)

        old_block = self._octave_width.blockSignals(True)
        if self._octave_width.text() != octave_width_text:
            self._octave_width.setText(octave_width_text)
        self._octave_width.blockSignals(old_block)

        # Centre octave
        centre_octave = table.get_centre_octave()

        old_block = self._centre_octave.blockSignals(True)
        if self._centre_octave.value() != centre_octave:
            self._centre_octave.setValue(centre_octave)
        self._centre_octave.blockSignals(old_block)

        # Tuning centre
        ref_note_index = table.get_ref_note_index()

        old_block = self._tuning_centre.blockSignals(True)
        self._tuning_centre.set_items(
                table.get_note_name(i) for i in range(table.get_note_count()))
        self._tuning_centre.setCurrentIndex(ref_note_index)
        self._tuning_centre.blockSignals(old_block)

    def _change_name(self, name):
        table = self._get_tuning_table()
        table.set_name(name)
        self._updater.signal_update('signal_tuning_tables')

    def _change_ref_pitch(self, pitch):
        table = self._get_tuning_table()
        table.set_ref_pitch(pitch)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_pitch_offset(self, offset):
        table = self._get_tuning_table()
        table.set_pitch_offset(offset)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_octave_width(self):
        text = str(self._octave_width.text())
        if '/' in text:
            parts = text.split('/')
            octave_width = [int(part) for part in parts]
        else:
            octave_width = float(text)

        table = self._get_tuning_table()
        table.set_octave_width(octave_width)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_centre_octave(self, index):
        table = self._get_tuning_table()
        table.set_centre_octave(index)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_tuning_centre(self, centre):
        table = self._get_tuning_table()
        table.set_ref_note_index(centre)
        self._updater.signal_update(self._get_update_signal_type())


class NotesToolBar(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._table_id = None

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

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_enabled)
        self.register_action(self._get_selection_signal_type(), self._update_enabled)

        icon_bank = self._ui_model.get_icon_bank()
        self._add_button.setIcon(QIcon(icon_bank.get_icon_path('add')))
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('remove')))

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._add_note)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_note)

        self._update_enabled()

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _get_selection_signal_type(self):
        return 'signal_tuning_table_note_selection_{}'.format(self._table_id)

    def _get_tuning_table(self):
        module = self._ui_model.get_module()
        return module.get_tuning_table(self._table_id)

    def _update_enabled(self):
        table = self._get_tuning_table()
        note_count = table.get_note_count()
        self._add_button.setEnabled(note_count < TUNING_TABLE_NOTES_MAX)
        self._remove_button.setEnabled(
                note_count > 1 and table.get_selected_note() != None)

    def _add_note(self):
        table = self._get_tuning_table()
        table.add_note()
        self._updater.signal_update(self._get_update_signal_type())

    def _remove_note(self):
        table = self._get_tuning_table()
        coords = table.get_selected_note()
        selected_index, _ = coords
        if selected_index != None:
            table.remove_note(selected_index)
            table.set_selected_note((max(0, selected_index - 1), coords[1]))
            self._updater.signal_update(
                self._get_update_signal_type(), self._get_selection_signal_type())


class NoteTableModel(QAbstractTableModel, Updater):

    def __init__(self):
        super().__init__()
        self._table_id = None
        self._items = []

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id

    def _on_setup(self):
        self._make_items()

    def get_index(self, row, column):
        return self.createIndex(row, column, None)

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _get_tuning_table(self):
        module = self._ui_model.get_module()
        return module.get_tuning_table(self._table_id)

    def _make_items(self):
        table = self._get_tuning_table()

        self._items = []
        for i in range(table.get_note_count()):
            name = table.get_note_name(i)
            pitch = table.get_note_pitch(i)
            self._items.append((name, pitch))

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
                    _, pitch = self._items[row]
                    if isinstance(pitch, list):
                        return '{}/{}'.format(*pitch)
                    else:
                        return str(pitch)

        return None

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            if section == 0:
                return 'Name'
            elif section == 1:
                return 'Pitch'
        return None

    def flags(self, index):
        default_flags = super().flags(index)
        if not index.isValid():
            return default_flags
        if not 0 <= index.row() < len(self._items):
            return default_flags

        return default_flags | Qt.ItemIsEditable

    def _get_validated_pitch(self, text):
        if '/' in text:
            if re.match('[0-9]+/[0-9]+$', text):
                parts = str(text).split('/')
                nums = [int(part) for part in parts]
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
        return None

    def setData(self, index, value, role):
        if role == Qt.EditRole:
            row = index.row()
            column = index.column()
            if 0 <= row < len(self._items):
                if column == 0:
                    new_name = value
                    table = self._get_tuning_table()
                    table.set_note_name(row, new_name)
                    self._updater.signal_update(self._get_update_signal_type())
                    return True
                elif column == 1:
                    new_pitch = self._get_validated_pitch(value)
                    if new_pitch == None:
                        return False
                    table = self._get_tuning_table()
                    table.set_note_pitch(row, new_pitch)
                    self._updater.signal_update(self._get_update_signal_type())
                    return True

        return False


class NoteTableView(QTableView, Updater):

    def __init__(self):
        super().__init__()
        self._table_id = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

        header = self.horizontalHeader()
        header.setStretchLastSection(True)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _get_selection_signal_type(self):
        return 'signal_tuning_table_note_selection_{}'.format(self._table_id)

    def _get_tuning_table(self):
        module = self._ui_model.get_module()
        return module.get_tuning_table(self._table_id)

    def _select_entry(self, cur_index, prev_index):
        if not cur_index.isValid():
            return
        row, column = cur_index.row(), cur_index.column()

        table = self._get_tuning_table()
        table.set_selected_note((row, column))
        self._updater.signal_update(self._get_selection_signal_type())

    def setModel(self, model):
        super().setModel(model)

        selection_model = self.selectionModel()

        table = self._get_tuning_table()
        coords = table.get_selected_note()
        if coords:
            row, column = coords
            selection_model.select(
                    model.get_index(row, column), QItemSelectionModel.Select)

        QObject.connect(
                selection_model,
                SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
                self._select_entry)


class Notes(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._table_id = None

        self._toolbar = NotesToolBar()

        self._table_model = None
        self._table_view = NoteTableView()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._toolbar)
        v.addWidget(self._table_view)
        self.setLayout(v)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id
        self._toolbar.set_tuning_table_id(table_id)
        self._table_view.set_tuning_table_id(table_id)

    def _on_setup(self):
        self.add_updating_child(self._toolbar, self._table_view)
        self.register_action(self._get_update_signal_type(), self._update_model)

        self._update_model()

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _update_model(self):
        if self._table_model:
            self.remove_updating_child(self._table_model)
        self._table_model = NoteTableModel()
        self._table_model.set_tuning_table_id(self._table_id)
        self.add_updating_child(self._table_model)
        self._table_view.setModel(self._table_model)


