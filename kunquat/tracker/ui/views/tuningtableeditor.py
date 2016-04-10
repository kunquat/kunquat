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

import re

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.kunquat.limits import *
from .notationeditor import RatioValidator


class TuningTableEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._table_id = None
        self._ui_model = None
        self._updater = None

        self._name = QLineEdit()

        self._ref_pitch = QDoubleSpinBox()
        self._ref_pitch.setDecimals(2)
        self._ref_pitch.setRange(-9999, 9999)

        self._pitch_offset = QDoubleSpinBox()
        self._pitch_offset.setDecimals(2)
        self._pitch_offset.setRange(-9999, 9999)

        self._octave_width = QLineEdit()
        self._octave_width.setValidator(RatioValidator())

        self._center_octave = QSpinBox()
        self._center_octave.setRange(0, TUNING_TABLE_OCTAVES - 1)

        self._tuning_center = QComboBox()

        self._notes = Notes()

        gl = QGridLayout()
        gl.setMargin(0)
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
        gl.addWidget(QLabel('Center octave:'), 4, 0)
        gl.addWidget(self._center_octave, 4, 1)
        gl.addWidget(QLabel('Tuning center:'), 5, 0)
        gl.addWidget(self._tuning_center, 5, 1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addLayout(gl)
        v.addWidget(self._notes)
        self.setLayout(v)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id
        self._notes.set_tuning_table_id(table_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._notes.set_ui_model(ui_model)

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
                self._center_octave,
                SIGNAL('valueChanged(int)'),
                self._change_center_octave)
        QObject.connect(
                self._tuning_center,
                SIGNAL('currentIndexChanged(int)'),
                self._change_tuning_center)

        self._update_name()
        self._update_params()

    def unregister_updaters(self):
        self._notes.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _perform_updates(self, signals):
        if 'signal_tuning_tables' in signals:
            self._update_name()
        if self._get_update_signal_type() in signals:
            self._update_params()

    def _get_tuning_table(self):
        module = self._ui_model.get_module()
        return module.get_tuning_table(self._table_id)

    def _update_name(self):
        table = self._get_tuning_table()
        name = table.get_name() or u''
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
            octave_width_text = u'{}/{}'.format(*octave_width)
        else:
            octave_width_text = u'{}'.format(octave_width)

        old_block = self._octave_width.blockSignals(True)
        if self._octave_width.text() != octave_width_text:
            self._octave_width.setText(octave_width_text)
        self._octave_width.blockSignals(old_block)

        # Center octave
        center_octave = table.get_center_octave()

        old_block = self._center_octave.blockSignals(True)
        if self._center_octave.value() != center_octave:
            self._center_octave.setValue(center_octave)
        self._center_octave.blockSignals(old_block)

        # Tuning center
        ref_note_index = table.get_ref_note_index()

        old_block = self._tuning_center.blockSignals(True)
        self._tuning_center.clear()
        for i in xrange(table.get_note_count()):
            note_name = table.get_note_name(i)
            self._tuning_center.addItem(note_name)
        self._tuning_center.setCurrentIndex(ref_note_index)
        self._tuning_center.blockSignals(old_block)

    def _change_name(self, name_qstring):
        name = unicode(name_qstring)
        table = self._get_tuning_table()
        table.set_name(name)
        self._updater.signal_update(set(['signal_tuning_tables']))

    def _change_ref_pitch(self, pitch):
        table = self._get_tuning_table()
        table.set_ref_pitch(pitch)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _change_pitch_offset(self, offset):
        table = self._get_tuning_table()
        table.set_pitch_offset(offset)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _change_octave_width(self):
        text = unicode(self._octave_width.text())
        if '/' in text:
            parts = text.split('/')
            octave_width = [int(part) for part in parts]
        else:
            octave_width = float(text)

        table = self._get_tuning_table()
        table.set_octave_width(octave_width)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _change_center_octave(self, index):
        table = self._get_tuning_table()
        table.set_center_octave(index)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _change_tuning_center(self, center):
        table = self._get_tuning_table()
        table.set_ref_note_index(center)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class NotesToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._table_id = None
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

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id

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

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _get_selection_signal_type(self):
        return 'signal_tuning_table_note_selection_{}'.format(self._table_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_update_signal_type(), self._get_selection_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

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
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _remove_note(self):
        table = self._get_tuning_table()
        coords = table.get_selected_note()
        selected_index, _ = coords
        if selected_index != None:
            table.remove_note(selected_index)
            table.set_selected_note((max(0, selected_index - 1), coords[1]))
            self._updater.signal_update(set([
                self._get_update_signal_type(), self._get_selection_signal_type()]))


class NoteTableModel(QAbstractTableModel):

    def __init__(self):
        QAbstractTableModel.__init__(self)
        self._table_id = None
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._make_items()

    def unregister_updaters(self):
        pass

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
        for i in xrange(table.get_note_count()):
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
                    return QVariant(name)
                elif column == 1:
                    _, pitch = self._items[row]
                    if isinstance(pitch, list):
                        return QVariant('{}/{}'.format(*pitch))
                    else:
                        return QVariant(str(pitch))

        return QVariant()

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            if section == 0:
                return QVariant('Name')
            elif section == 1:
                return QVariant('Pitch')
        return QVariant()

    def flags(self, index):
        default_flags = QAbstractTableModel.flags(self, index)
        if not index.isValid():
            return default_flags
        if not 0 <= index.row() < len(self._items):
            return default_flags

        return default_flags | Qt.ItemIsEditable

    def _get_validated_pitch(self, text):
        if '/' in text:
            if re.match('[0-9]+/[0-9]+$', text):
                parts = unicode(text).split('/')
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
                    new_name = unicode(value.toString())
                    table = self._get_tuning_table()
                    table.set_note_name(row, new_name)
                    self._updater.signal_update(set([self._get_update_signal_type()]))
                    return True
                elif column == 1:
                    new_pitch = self._get_validated_pitch(unicode(value.toString()))
                    if new_pitch == None:
                        return False
                    table = self._get_tuning_table()
                    table.set_note_pitch(row, new_pitch)
                    self._updater.signal_update(set([self._get_update_signal_type()]))
                    return True

        return False


class NoteTableView(QTableView):

    def __init__(self):
        QTableView.__init__(self)
        self._table_id = None
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

        header = self.horizontalHeader()
        header.setStretchLastSection(True)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def unregister_updaters(self):
        pass

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
        self._updater.signal_update(set([self._get_selection_signal_type()]))

    def setModel(self, model):
        QTableView.setModel(self, model)

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


class Notes(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._table_id = None
        self._ui_model = None
        self._updater = None

        self._toolbar = NotesToolBar()

        self._table_model = None
        self._table_view = NoteTableView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(self._toolbar)
        v.addWidget(self._table_view)
        self.setLayout(v)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id
        self._toolbar.set_tuning_table_id(table_id)
        self._table_view.set_tuning_table_id(table_id)

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

    def _get_update_signal_type(self):
        return 'signal_tuning_table_{}'.format(self._table_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_model()

    def _update_model(self):
        self._table_model = NoteTableModel()
        self._table_model.set_tuning_table_id(self._table_id)
        self._table_model.set_ui_model(self._ui_model)
        self._table_view.setModel(self._table_model)


