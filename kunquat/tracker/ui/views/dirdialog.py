# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import deque
from itertools import takewhile
import os
import os.path
import sys
import time

from kunquat.tracker.ui.qt import *

from .utils import get_abs_window_size


class DirDialog(QDialog):

    def __init__(self, ui_model, title, start_dir):
        super().__init__()

        if not start_dir:
            start_dir = os.getcwd()

        start_dir = os.path.normpath(start_dir)

        self._ui_model = ui_model
        self._current_dir = None
        self._selected_dir = None

        self.setWindowTitle(title)

        style_mgr = self._ui_model.get_style_manager()

        self._dir_branch = DirBranch(self._ui_model)
        self._dir_tree_view = DirTreeView(self._ui_model)
        self._cancel_button = QPushButton('Cancel')
        self._select_button = QPushButton('Choose')
        self._select_button.setEnabled(False)

        bl = QHBoxLayout()
        bl.setContentsMargins(0, 0, 0, 0)
        bl.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))
        bl.addStretch(1)
        bl.addWidget(self._cancel_button)
        bl.addWidget(self._select_button)

        v = QVBoxLayout()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        v.setContentsMargins(margin, margin, margin, margin)
        v.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))
        v.addWidget(self._dir_branch)
        v.addWidget(self._dir_tree_view)
        v.addLayout(bl)
        self.setLayout(v)

        self._cancel_button.clicked.connect(self.close)
        self._select_button.clicked.connect(self._choose_current_dir)

        self._dir_tree_view.dirChanged.connect(self._on_dir_changed)

        self._set_current_dir(start_dir)

        self._update_enabled()

    def get_path(self):
        self.exec_()
        return self._selected_dir

    def _set_current_dir(self, new_dir):
        if self._current_dir == new_dir:
            return

        self._current_dir = new_dir
        self._dir_branch.set_current_dir(self._current_dir)
        self._dir_tree_view.set_current_dir(self._current_dir)

    def _on_dir_changed(self, dir_name):
        if self._current_dir == dir_name:
            return

        self._current_dir = dir_name
        self._dir_branch.set_current_dir(self._current_dir)

        self._update_enabled()

    def _choose_current_dir(self):
        self._selected_dir = self._current_dir
        self.close()

    def _update_enabled(self):
        enabled = os.path.exists(self._current_dir)
        self._select_button.setEnabled(enabled)

    def sizeHint(self):
        return get_abs_window_size(0.4, 0.5)


class DirBranch(QWidget):

    def __init__(self, ui_model):
        super().__init__()

        self._ui_model = ui_model
        self._current_dir = None

        self._path = QLabel()

        style_mgr = self._ui_model.get_style_manager()

        h = QHBoxLayout()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        h.setContentsMargins(margin, margin, margin, margin)
        h.setSpacing(style_mgr.get_scaled_size_param('large_padding'))
        h.addWidget(QLabel('Selected directory:'))
        h.addWidget(self._path, 1)
        self.setLayout(h)

    def set_current_dir(self, new_dir):
        self._current_dir = new_dir
        self._path.setText(self._current_dir)


class DirEntry():

    def __init__(self, path, parent):
        self._parent = parent
        self._path = path
        self.subdirs = []
        self.is_open = False

        head, tail = os.path.split(self._path)
        if not tail:
            head, tail = os.path.split(head)
            if not tail:
                if sys.platform.startswith(('win32', 'cygwin')):
                    self._name, _ = os.path.splitdrive(head)
                self._name = head
            else:
                self._name = tail
        else:
            self._name = tail

    def __eq__(self, other):
        return (self._path == other._path)

    def _is_dir_visible(self, name):
        return (not name.startswith('.'))

    def get_updated_subdirs(self):
        dirs = []
        try:
            with os.scandir(self._path) as dir_entries:
                for dir_entry in dir_entries:
                    if dir_entry.is_dir() and self._is_dir_visible(dir_entry.name):
                        subdir = DirEntry(dir_entry.path, self)
                        dirs.append(subdir)
        except OSError:
            return []

        dirs.sort(key=lambda e: e.get_name())

        return dirs

    def get_name(self):
        return self._name

    def get_path(self):
        return self._path

    def get_parent(self):
        return self._parent


def _get_all_path_components(path):
    if not path:
        return []

    head, tail = os.path.split(path)
    if head == path:
        return [head]

    parts = _get_all_path_components(head)
    parts.append(tail)
    return parts


def _get_root_path():
    return os.path.split(os.sep)[0]


class DirTreeModel(QAbstractItemModel):

    stepScanEntries = Signal(name='stepScanEntries')
    startDirFound = Signal(name='startDirFound')
    startDirNotFound = Signal(name='startDirNotFound')

    def __init__(self, ui_model):
        super().__init__()
        self._ui_model = ui_model
        self._start_dir_components_to_resolve = None
        self._resolved_start_entries = []

        self._roots = []

        self._entries_to_scan = deque()
        self._icons = {}

        self.stepScanEntries.connect(self._step_scan_entries, Qt.QueuedConnection)

        if sys.platform.startswith(('win32', 'cygwin')):
            raise NotImplementedError
        else:
            self._roots = [DirEntry(_get_root_path(), None)]

        self._request_scans(self._roots)

    def set_start_dir(self, new_dir):
        self._start_dir_components_to_resolve = deque(_get_all_path_components(new_dir))
        self._resolved_start_entries = []

        # Try to resolve as much of our starting directory as possible
        subdirs = self._roots
        scan_start_entry = None
        while self._start_dir_components_to_resolve:
            found_next_component = False

            for subdir in subdirs:
                if subdir.get_name() == self._start_dir_components_to_resolve[0]:
                    found_next_component = True
                    self._resolved_start_entries.append(subdir)
                    subdirs = subdir.subdirs
                    self._start_dir_components_to_resolve.popleft()
                    scan_start_entry = subdir
                    break

            if not found_next_component:
                break

        if self._start_dir_components_to_resolve:
            if scan_start_entry:
                self._request_scans([scan_start_entry])
        else:
            self.startDirFound.emit()

    def get_root_dir_index(self):
        return self._get_model_index(self._roots[0])

    def get_resolved_start_dir_indices(self):
        return (self._get_model_index(e) for e in self._resolved_start_entries)

    def update_subdirs(self, index):
        if not index.isValid():
            return

        entry = index.internalPointer()
        assert entry
        self._request_scans(entry.subdirs)

    def _request_scans(self, entries):
        self._entries_to_scan.extend(entries)

        if entries:
            self.stepScanEntries.emit()

    def _try_extend_start_dir_at(self, new_entry):
        if not self._start_dir_components_to_resolve:
            return

        parent_entry = new_entry.get_parent()

        if not self._resolved_start_entries:
            # Try to find our first entry
            if parent_entry:
                return

            if sys.platform.startswith(('win32', 'cygwin')):
                raise NotImplementedError
            else:
                if new_entry.get_path() == self._start_dir_components_to_resolve[0]:
                    self._resolved_start_entries.append(new_entry)
                    self._start_dir_components_to_resolve.popleft()

            if not self._start_dir_components_to_resolve:
                self.startDirFound.emit()
            return

        # Try to continue our existing non-empty list of entries
        if parent_entry and (parent_entry == self._resolved_start_entries[-1]):
            if new_entry.get_name() == self._start_dir_components_to_resolve[0]:
                self._resolved_start_entries.append(new_entry)
                self._start_dir_components_to_resolve.popleft()

                if not self._start_dir_components_to_resolve:
                    self.startDirFound.emit()
                else:
                    self._request_scans([new_entry])

    def _step_scan_entries(self):
        start_time = time.time()
        while self._entries_to_scan:
            entry = self._entries_to_scan.popleft()
            entry_model_index = self._get_model_index(entry)
            assert entry_model_index.isValid()

            new_subdirs = entry.get_updated_subdirs()

            row = 0
            read_index = 0
            while row < len(entry.subdirs):
                if read_index >= len(new_subdirs):
                    remove_count = len(entry.subdirs) - row
                    self.beginRemoveRows(entry_model_index, row, row + remove_count - 1)
                    entry.subdirs[row:] = []
                    self.endRemoveRows()
                    break

                old_name = entry.subdirs[row].get_name()
                new_name = new_subdirs[read_index].get_name()
                if old_name == new_name:
                    row += 1
                    read_index += 1
                elif old_name < new_name:
                    remove_count = len(takewhile(
                        lambda e: e.get_name() < new_name, entry.subdirs[row:]))
                    assert remove_count > 0
                    self.beginRemoveRows(entry_model_index, row, row + remove_count - 1)
                    entry.subdirs[row:row + remove_count] = []
                    self.endRemoveRows()
                else: # old_name > new_name
                    add_entries = list(takewhile(
                        lambda e: e.get_name() < old_name, new_subdirs[read_index:]))
                    add_count = len(add_entries)
                    assert add_count > 0
                    self.beginInsertRows(entry_model_index, row, row + add_count - 1)
                    entry.subdirs[row:row] = add_entries
                    self.endInsertRows()
                    read_index += add_count
                    for e in add_entries:
                        self._try_extend_start_dir_at(e)

            if read_index < len(new_subdirs):
                add_count = len(new_subdirs) - read_index
                self.beginInsertRows(entry_model_index, row, row + add_count - 1)
                entry.subdirs.extend(new_subdirs[read_index:])
                self.endInsertRows()
                for e in new_subdirs[read_index:]:
                    self._try_extend_start_dir_at(e)

            cur_time = time.time()
            elapsed = cur_time - start_time
            if elapsed > 0.03:
                break

        if self._entries_to_scan:
            self.stepScanEntries.emit()
        else:
            if self._start_dir_components_to_resolve:
                self.startDirNotFound.emit()
                self._start_dir_components_to_resolve = []
                self._resolved_start_entries = []

    def _get_icon(self, entry):
        if 'file_directory' not in self._icons:
            icon_bank = self._ui_model.get_icon_bank()
            style_mgr = self._ui_model.get_style_manager()
            size = style_mgr.get_scaled_size_param('file_dialog_icon_size')
            icon_size = QSize(size, size)

            image = QImage()
            image.load(icon_bank.get_icon_path('file_directory'))
            image = image.convertToFormat(QImage.Format_ARGB32_Premultiplied)
            icon = image.scaled(icon_size, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)

            self._icons['file_directory'] = icon

        return self._icons['file_directory']

    def _get_model_index(self, entry):
        parent = entry.get_parent()
        children = parent.subdirs if parent else self._roots
        for i, e in enumerate(children):
            if e == entry:
                return self.createIndex(i, 0, entry)
        return QModelIndex()

    # Qt interface

    def rowCount(self, parent):
        if not parent.isValid():
            return len(self._roots)

        entry = parent.internalPointer()
        return len(entry.subdirs)

    def columnCount(self, parent):
        return 1

    def index(self, row, column, parent):
        if column != 0:
            return QModelIndex()

        if not parent.isValid():
            if row >= len(self._roots):
                return QModelIndex()
            entry = self._roots[row]
        else:
            parent_entry = parent.internalPointer()
            subdirs = parent_entry.subdirs
            if row >= len(subdirs):
                return QModelIndex()
            entry = subdirs[row]

        return self.createIndex(row, 0, entry)

    def parent(self, index):
        if not index.isValid():
            return QModelIndex()

        entry = index.internalPointer()
        parent_entry = entry.get_parent()
        if not parent_entry:
            return QModelIndex()

        return self._get_model_index(parent_entry)

    def data(self, index, role):
        if not index.isValid():
            return None

        entry = index.internalPointer()
        if role == Qt.DisplayRole:
            return entry.get_name()
        elif role == Qt.DecorationRole:
            return self._get_icon(entry)

        return None

    def headerData(self, section, orientation, role):
        return None


class DirTreeView(QTreeView):

    dirChanged = Signal(str, name='dirChanged')

    def __init__(self, ui_model):
        super().__init__()

        self._ui_model = ui_model
        self._current_dir = None

        self.setUniformRowHeights(True)
        self.setRootIsDecorated(True)

        self.setModel(DirTreeModel(self._ui_model))

        self.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.setSelectionMode(QAbstractItemView.SingleSelection)

        self.model().startDirFound.connect(self._on_start_dir_found)
        self.model().startDirNotFound.connect(self._on_start_dir_not_found)
        self.selectionModel().currentChanged.connect(self._on_dir_changed)
        self.expanded.connect(self._on_expanded)

    def set_current_dir(self, new_dir):
        self._current_dir = new_dir
        self.model().set_start_dir(new_dir)

    def _on_start_dir_found(self):
        indices = list(self.model().get_resolved_start_dir_indices())
        assert indices
        for index in indices[:-1]:
            self.expand(index)
        self.selectionModel().setCurrentIndex(indices[-1], QItemSelectionModel.Select)

    def _on_start_dir_not_found(self):
        indices = list(self.model().get_resolved_start_dir_indices())
        if not indices:
            root_index = self.model().get_root_dir_index()
            self.expand(root_index)
            self.selectionModel().setCurrentIndex(root_index, QItemSelectionModel.Select)
            return

        for index in indices:
            self.expand(index)
        self.selectionModel().setCurrentIndex(indices[-1], QItemSelectionModel.Select)

    def _on_expanded(self, index):
        self.model().update_subdirs(index)

    def _on_dir_changed(self, cur_index, prev_index):
        entry = cur_index.internalPointer()
        if not entry:
            return

        self.dirChanged.emit(entry.get_path())


