# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

import kunquat.tracker.cmdline as cmdline
from .updater import Updater


DISP_CONTEXTS = {
        'mix': 'Playback',
        'fire': 'User',
        #'tfire': 'Tracker',
        }


class EventListModel(QAbstractTableModel):

    HEADERS = ["#", "Chn", "Type", "Value", "Context"]

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self._log = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def _perform_updates(self, signals):
        event_history = self._ui_model.get_event_history()
        log = event_history.get_log()

        oldlen = len(self._log)
        newlen = len(log)
        if oldlen < newlen:
            self.beginInsertRows(QModelIndex(), oldlen, newlen - 1)
            self._log = log
            self.endInsertRows()
        elif oldlen > newlen:
            self.beginRemoveRows(QModelIndex(), 0, oldlen - newlen - 1)
            self._log = log
            self.endRemoveRows()
        else:
            self._log = log

        self.dataChanged.emit(
                self.index(0, 0),
                self.index(len(self._log) - 1, len(self.HEADERS) - 1))

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    # Qt interface

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._log)

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return len(self.HEADERS)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row, column = index.row(), index.column()
            if 0 <= column < len(self.HEADERS) and 0 <= row < len(self._log):
                value = str(self._log[len(self._log) - row - 1][column])
                if self.HEADERS[column] == 'Context':
                    value = DISP_CONTEXTS[value]
                return value

        return None

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole:
            if orientation == Qt.Horizontal:
                if 0 <= section < len(self.HEADERS):
                    return self.HEADERS[section]

        return None


class EventTable(QTableView):

    def __init__(self):
        super().__init__()
        self._focusbottom = True

        vscrollbar = self.verticalScrollBar()
        vscrollbar.rangeChanged.connect(self._on_rangeChanged)
        vscrollbar.valueChanged.connect(self._on_valueChanged)

        self.horizontalHeader().setStretchLastSection(True)

        self.verticalHeader().setSectionResizeMode(QHeaderView.Fixed)

    def update_style(self, style_mgr):
        hh = self.horizontalHeader()
        hh.resizeSection(0, style_mgr.get_scaled_size(6))
        hh.resizeSection(1, style_mgr.get_scaled_size(4))
        hh.resizeSection(2, style_mgr.get_scaled_size(8))
        hh.resizeSection(3, style_mgr.get_scaled_size(20))

        vh = self.verticalHeader()
        vh.setDefaultSectionSize(style_mgr.get_scaled_size(2.5))

    def setModel(self, model):
        super().setModel(model)

        hh = self.horizontalHeader()
        hh.resizeSection(0, 60)
        hh.resizeSection(1, 40)
        hh.resizeSection(2, 80)
        hh.resizeSection(3, 200)

    def _on_rangeChanged(self, rmin, rmax):
        if self._focusbottom:
            vscrollbar = self.verticalScrollBar()
            vscrollbar.setValue(vscrollbar.maximum())

    def _on_valueChanged(self, value):
        vscrollbar = self.verticalScrollBar()
        self._focusbottom = (vscrollbar.value() == vscrollbar.maximum())


class EventFilterButton(QCheckBox):

    def __init__(self, context):
        super().__init__(DISP_CONTEXTS[context])
        self._ui_model = None
        self._updater = None
        self._context = context

        self.clicked.connect(self._on_clicked)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signal):
        self._update_all()

    def _update_all(self):
        event_history = self._ui_model.get_event_history()
        self.blockSignals(True)
        event_count = event_history.get_event_count_with_filter(self._context)
        self.setText('{} ({})'.format(DISP_CONTEXTS[self._context], event_count))
        is_allowed = event_history.is_context_allowed(self._context)
        if is_allowed != self.isChecked():
            self.setChecked(is_allowed)
        self.blockSignals(False)

    def _on_clicked(self, is_checked):
        event_history = self._ui_model.get_event_history()
        event_history.allow_context(self._context, is_checked)


class EventFilterView(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._mix_toggle = EventFilterButton('mix')
        self._fire_toggle = EventFilterButton('fire')

        self.add_to_updaters(self._mix_toggle, self._fire_toggle)

        h = QHBoxLayout()
        h.setContentsMargins(6, 6, 6, 6)
        h.setSpacing(8)
        h.addWidget(self._mix_toggle)
        h.addWidget(self._fire_toggle)
        h.addStretch()
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        spacing = style_mgr.get_scaled_size_param('large_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(spacing)


class EventList(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._logmodel = EventListModel()
        self._filters = EventFilterView()

        self.add_to_updaters(self._logmodel, self._filters)

        self._tableview = EventTable()
        self._tableview.setModel(self._logmodel)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._tableview)
        v.addWidget(self._filters)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._tableview.update_style(style_mgr)

        margin = style_mgr.get_scaled_size_param('medium_padding')
        spacing = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(spacing)


