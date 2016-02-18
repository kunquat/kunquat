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

from kunquat.kunquat.limits import *


class Expressions(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._expr_list = ExpressionList()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(4)
        v.addWidget(self._expr_list)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._expr_list.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._expr_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._expr_list.unregister_updaters()


class ExpressionListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._new_button = QToolButton()
        self._new_button.setText('New expression')
        self._new_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove expression')
        self._remove_button.setEnabled(False)

        self.addWidget(self._new_button)
        self.addWidget(self._remove_button)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._new_button, SIGNAL('clicked()'), self._add_expression)
        QObject.connect(
                self._remove_button, SIGNAL('clicked()'), self._remove_expression)

        self._update_buttons()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_list_update_signal_type(self):
        return 'signal_expr_list_{}'.format(self._au_id)

    def _get_selection_update_signal_type(self):
        return 'signal_expr_selection_{}'.format(self._au_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_list_update_signal_type(),
            self._get_selection_update_signal_type()])

        if not signals.isdisjoint(update_signals):
            self._update_buttons()

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
        self._updater.signal_update(set([self._get_list_update_signal_type()]))

    def _remove_expression(self):
        au = self._get_audio_unit()
        selected_expr = au.get_selected_expression()
        if not selected_expr:
            return

        au.remove_expression(selected_expr)
        au.set_selected_expression(None)
        self._updater.signal_update(set([
            self._get_list_update_signal_type(),
            self._get_selection_update_signal_type()]))


class ExpressionListModel(QAbstractListModel):

    def __init__(self):
        QAbstractListModel.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_au_id(self, au_id):
        self._au_id = au_id

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
                return QVariant(expr_name)

        return QVariant()

    def headerData(self, section, orientation, role):
        return QVariant()


class ExpressionListView(QListView):

    def __init__(self):
        QListView.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        for signal_type in ('clicked', 'activated'):
            signal = '{}(const QModelIndex&)'.format(signal_type)
            QObject.connect(
                    self, SIGNAL(signal), self._select_expression)

    def unregister_updaters(self):
        pass

    def _get_selection_update_signal_type(self):
        return 'signal_expr_selection_{}'.format(self._au_id)

    def _select_expression(self, index):
        expr_name = self.model().get_item(index)
        if expr_name:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)
            au.set_selected_expression(expr_name)
            self._updater.signal_update(set([self._get_selection_update_signal_type()]))


class ExpressionList(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._toolbar = ExpressionListToolBar()

        self._expr_list_model = None
        self._expr_list_view = ExpressionListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._expr_list_view)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._toolbar.set_au_id(au_id)
        self._expr_list_view.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._toolbar.set_ui_model(ui_model)
        self._expr_list_view.set_ui_model(ui_model)

        self._update_model()

    def unregister_updaters(self):
        self._expr_list_view.unregister_updaters()
        self._toolbar.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _get_list_update_signal_type(self):
        return 'signal_expr_list_{}'.format(self._au_id)

    def _perform_updates(self, signals):
        if self._get_list_update_signal_type() in signals:
            self._update_model()

    def _update_model(self):
        self._expr_list_model = ExpressionListModel()
        self._expr_list_model.set_au_id(self._au_id)
        self._expr_list_model.set_ui_model(self._ui_model)
        self._expr_list_view.setModel(self._expr_list_model)


class ExpressionEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def unregister_updaters(self):
        pass


