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
        # TODO

    def _change_name(self, name_qstring):
        name = unicode(name_qstring)
        table = self._get_tuning_table()
        table.set_name(name)
        self._updater.signal_update(set(['signal_tuning_tables']))


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

    def _perform_updates(self, signals):
        pass # TODO

    def _update_enabled(self):
        pass # TODO

    def _add_note(self):
        pass # TODO

    def _remove_note(self):
        pass # TODO


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

    def _make_items(self):
        pass # TODO

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
                    pass # TODO
                elif column == 1:
                    pass # TODO

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

    def setData(self, index, value, role):
        if role == Qt.EditRole:
            row = index.row()
            column = index.column()
            if 0 <= row < len(self._items):
                if column == 0:
                    pass # TODO
                elif column == 1:
                    pass # TODO

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

    def _select_entry(self, cur_index, prev_index):
        if not cur_index.isValid():
            return
        row, column = cur_index.row(), cur_index.column()

        # TODO

    def setModel(self, model):
        QTableView.setModel(self, model)

        selection_model = self.selectionModel()

        # TODO: Refresh selection

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

    def _perform_updates(self, signals):
        if 'signal_tuning_table' in signals:
            self._update_model()

    def _update_model(self):
        self._table_model = NoteTableModel()
        self._table_model.set_tuning_table_id(self._table_id)
        self._table_model.set_ui_model(self._ui_model)
        self._table_view.setModel(self._table_model)


