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

from itertools import chain

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.kunquat.limits import *
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from kunquat.tracker.ui.views.varnamevalidator import VarNameValidator
from .updatingauview import UpdatingAUView


class Expressions(QWidget, UpdatingAUView):

    def __init__(self):
        super().__init__()

        self._default_note_expr = DefaultNoteExpr()
        self._expr_list = ExpressionList()
        self._expr_editor = ExpressionEditor()

        self.add_updating_child(
                self._default_note_expr, self._expr_list, self._expr_editor)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._default_note_expr)
        v.addWidget(self._expr_list)
        v.addWidget(self._expr_editor)
        v.addStretch(1)
        self.setLayout(v)


class DefaultNoteExpr(QWidget, UpdatingAUView):

    def __init__(self):
        super().__init__()
        self._expr_names = KqtComboBox()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(QLabel('Default note expression:'))
        h.addWidget(self._expr_names, 1)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action(self._get_list_update_signal_type(), self._update_contents)
        self.register_action(
                self._get_default_update_signal_type(), self._update_contents)

        QObject.connect(
                self._expr_names,
                SIGNAL('currentIndexChanged(int)'),
                self._change_expression)

        self._update_contents()

    def _get_list_update_signal_type(self):
        return 'signal_expr_list_{}'.format(self._au_id)

    def _get_default_update_signal_type(self):
        return 'signal_expr_default_{}'.format(self._au_id)

    def _get_audio_unit(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au

    def _update_contents(self):
        au = self._get_audio_unit()
        names = sorted(au.get_expression_names())
        selection = au.get_default_note_expression()

        old_block = self._expr_names.blockSignals(True)
        self._expr_names.setEnabled(len(names) > 0)
        self._expr_names.set_items(chain(['(none)'], (name for name in names)))
        try:
            self._expr_names.setCurrentIndex(names.index(selection) + 1)
        except ValueError:
            self._expr_names.setCurrentIndex(0)
        self._expr_names.blockSignals(old_block)

    def _change_expression(self, item_index):
        au = self._get_audio_unit()
        if item_index == 0:
            au.set_default_note_expression('')
        else:
            expr_name = self._expr_names.itemText(item_index)
            au.set_default_note_expression(expr_name)


class ExpressionListToolBar(QToolBar, UpdatingAUView):

    def __init__(self):
        super().__init__()

        self._new_button = QToolButton()
        self._new_button.setText('New expression')
        self._new_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove expression')
        self._remove_button.setEnabled(False)

        self.addWidget(self._new_button)
        self.addWidget(self._remove_button)

    def _on_setup(self):
        self.register_action(self._get_list_update_signal_type(), self._update_buttons)
        self.register_action(
                self._get_selection_update_signal_type(), self._update_buttons)

        QObject.connect(self._new_button, SIGNAL('clicked()'), self._add_expression)
        QObject.connect(
                self._remove_button, SIGNAL('clicked()'), self._remove_expression)

        self._update_buttons()

    def _get_list_update_signal_type(self):
        return 'signal_expr_list_{}'.format(self._au_id)

    def _get_selection_update_signal_type(self):
        return 'signal_expr_selection_{}'.format(self._au_id)

    def _get_audio_unit(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au

    def _update_buttons(self):
        au = self._get_audio_unit()
        selected_expr = au.get_selected_expression()
        self._remove_button.setEnabled(selected_expr != None)

    def _add_expression(self):
        au = self._get_audio_unit()
        au.add_expression()
        self._updater.signal_update(self._get_list_update_signal_type())

    def _remove_expression(self):
        au = self._get_audio_unit()
        selected_expr = au.get_selected_expression()
        if not selected_expr:
            return

        au.remove_expression(selected_expr)
        au.set_selected_expression(None)
        self._updater.signal_update(
            self._get_list_update_signal_type(),
            'signal_au_conns_expr_{}'.format(self._au_id),
            self._get_selection_update_signal_type())


class ExpressionListModel(QAbstractListModel, UpdatingAUView):

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
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        expr_names = au.get_expression_names()

        for expr_name in sorted(expr_names):
            self._items.append(expr_name)

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                expr_name = self._items[row]
                return expr_name

        return None

    def headerData(self, section, orientation, role):
        return None


class ExpressionListView(QListView, UpdatingAUView):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def _on_setup(self):
        for signal_type in ('clicked', 'activated'):
            signal = '{}(const QModelIndex&)'.format(signal_type)
            QObject.connect(
                    self, SIGNAL(signal), self._select_expression)

    def _get_selection_update_signal_type(self):
        return 'signal_expr_selection_{}'.format(self._au_id)

    def _select_expression(self, index):
        expr_name = self.model().get_item(index)
        if expr_name:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)
            au.set_selected_expression(expr_name)
            self._updater.signal_update(self._get_selection_update_signal_type())


class ExpressionList(QWidget, UpdatingAUView):

    def __init__(self):
        super().__init__()
        self._toolbar = ExpressionListToolBar()

        self._expr_list_model = None
        self._expr_list_view = ExpressionListView()

        self.add_updating_child(self._toolbar, self._expr_list_view)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._expr_list_view)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action(self._get_list_update_signal_type(), self._update_model)
        self._update_model()

    def _get_list_update_signal_type(self):
        return 'signal_expr_list_{}'.format(self._au_id)

    def _update_model(self):
        if self._expr_list_model:
            self.remove_updating_child(self._expr_list_model)
        self._expr_list_model = ExpressionListModel()
        self.add_updating_child(self._expr_list_model)
        self._expr_list_view.setModel(self._expr_list_model)


class ExpressionName(QWidget, UpdatingAUView):

    def __init__(self):
        super().__init__()
        self._name_editor = QLineEdit()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(QLabel('Name:'))
        h.addWidget(self._name_editor)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action(
                self._get_list_update_signal_type(), self._update_used_names)
        self.register_action(self._get_selection_update_signal_type(), self._update_name)

        QObject.connect(
                self._name_editor, SIGNAL('editingFinished()'), self._change_name)

        self._update_name()

    def _get_list_update_signal_type(self):
        return 'signal_expr_list_{}'.format(self._au_id)

    def _get_selection_update_signal_type(self):
        return 'signal_expr_selection_{}'.format(self._au_id)

    def _get_audio_unit(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au

    def _update_used_names(self):
        used_names = self._get_audio_unit().get_expression_names()
        self._name_editor.setValidator(VarNameValidator(set(used_names)))

    def _update_name(self):
        au = self._get_audio_unit()
        self._name = au.get_selected_expression() or ''

        if self._name_editor.text() != self._name:
            old_block = self._name_editor.blockSignals(True)
            self._name_editor.setText(self._name)
            self._name_editor.blockSignals(old_block)

    def _change_name(self):
        au = self._get_audio_unit()
        old_name = au.get_selected_expression()
        if not old_name:
            return

        new_name = str(self._name_editor.text())
        au.change_expression_name(old_name, new_name)
        au.set_selected_expression(new_name)
        if au.get_connections_expr_name() == old_name:
            au.set_connections_expr_name(new_name)
        self._updater.signal_update(
                self._get_list_update_signal_type(),
                self._get_selection_update_signal_type())


class ExpressionEditor(QWidget, UpdatingAUView):

    def __init__(self):
        super().__init__()

        self._name = ExpressionName()

        self.add_updating_child(self._name)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._name)
        self.setLayout(v)


