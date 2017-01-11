# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

import kunquat.tracker.cmdline as cmdline


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

        QObject.emit(
                self,
                SIGNAL('dataChanged(QModelIndex, QModelIndex)'),
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
        QObject.connect(
                vscrollbar,
                SIGNAL('rangeChanged(int, int)'),
                self._on_rangeChanged)
        QObject.connect(
                vscrollbar,
                SIGNAL('valueChanged(int)'),
                self._on_valueChanged)

    def setModel(self, model):
        super().setModel(model)

        hh = self.horizontalHeader()
        hh.resizeSection(0, 60)
        hh.resizeSection(1, 40)
        hh.resizeSection(2, 80)
        hh.resizeSection(3, 200)
        hh.resizeSection(4, 100)

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

        QObject.connect(self, SIGNAL('clicked(bool)'), self._on_clicked)

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


class EventFilterView(QWidget):

    def __init__(self):
        super().__init__()

        h = QHBoxLayout()
        h.setContentsMargins(6, 6, 6, 6)
        h.setSpacing(8)
        self._mix_toggle = EventFilterButton('mix')
        self._fire_toggle = EventFilterButton('fire')
        #self._tfire_toggle = EventFilterButton('tfire')
        h.addWidget(self._mix_toggle)
        h.addWidget(self._fire_toggle)
        #h.addWidget(self._tfire_toggle)
        h.addStretch()
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._mix_toggle.set_ui_model(ui_model)
        self._fire_toggle.set_ui_model(ui_model)
        #self._tfire_toggle.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._mix_toggle.unregister_updaters()
        self._fire_toggle.unregister_updaters()
        #self._tfire_toggle.unregister_updaters()


class EventList(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self.setWindowTitle('Event Log')

        self._logmodel = EventListModel()

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)

        self._tableview = EventTable()
        self._tableview.setModel(self._logmodel)
        v.addWidget(self._tableview)
        self._filters = EventFilterView()
        v.addWidget(self._filters)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._logmodel.set_ui_model(ui_model)
        self._filters.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._logmodel.unregister_updaters()
        self._filters.unregister_updaters()

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_event_log()

    def sizeHint(self):
        return QSize(600, 768)


