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

import datetime
import errno
import json
import os
import os.path
import time
import zipfile

from kunquat.extras.sndfile import SndFileR, SndFileError
from kunquat.tracker.ui.qt import *

from .confirmdialog import ConfirmDialog
from .utils import get_abs_window_size, get_default_font_info


class FileDialog(QDialog):

    TYPE_KQT            = 0x1
    TYPE_KQTI           = 0x2
    TYPE_KQTE           = 0x4
    TYPE_ALL_KQT        = TYPE_KQT | TYPE_KQTI | TYPE_KQTE
    TYPE_WAV            = 0x100
    TYPE_AIFF           = 0x200
    TYPE_AU             = 0x400
    TYPE_WAVPACK        = 0x800
    TYPE_FLAC           = 0x1000
    TYPE_ALL_PCM        = TYPE_WAV | TYPE_AIFF | TYPE_AU | TYPE_WAVPACK | TYPE_FLAC
    TYPE_ANY            = 0x100000

    _TYPE_DIR           = -1
    _TYPE_FILE          = -2
    _TYPE_INACCESSIBLE  = -3

    MODE_OPEN = 'open'
    MODE_OPEN_MULT = 'open_mult'
    MODE_SAVE = 'save'
    MODE_CHOOSE_DIR = 'choose_dir'

    def __init__(self, ui_model, mode, title, start_path, filters=0):
        super().__init__()

        self._ui_model = ui_model
        self._mode = mode
        self._current_dir = None
        self._final_infos = None

        if self._mode == FileDialog.MODE_SAVE:
            if os.path.isdir(start_path):
                start_dir = start_path
                start_file = None
            else:
                start_dir, start_file = os.path.split(start_path)
                if not os.path.exists(start_dir):
                    start_dir = start_path
                    start_file = None
        else:
            start_dir = start_path
            start_file = None

        if self._mode == FileDialog.MODE_SAVE:
            filters |= FileDialog.TYPE_ANY

        self._dir_history = [os.path.abspath(os.path.expanduser('~'))]

        self.setWindowTitle(title)

        style_mgr = self._ui_model.get_style_manager()

        select_texts = {
            FileDialog.MODE_OPEN:       'Open',
            FileDialog.MODE_OPEN_MULT:  'Open',
            FileDialog.MODE_SAVE:       'Save',
            FileDialog.MODE_CHOOSE_DIR: 'Choose',
        }
        select_text = select_texts[self._mode]

        allow_multiple = (self._mode == FileDialog.MODE_OPEN_MULT)

        self._dir_branch = DirectoryBranch(self._ui_model)
        self._dir_view = DirectoryView(self._ui_model, filters, allow_multiple)
        self._file_name = FileName()
        self._cancel_button = QPushButton('Cancel')
        self._select_button = QPushButton(select_text)
        self._select_button.setEnabled(False)

        fl = QGridLayout()
        fl.setContentsMargins(0, 0, 0, 0)
        fl.setHorizontalSpacing(style_mgr.get_scaled_size_param('medium_padding'))
        fl.setVerticalSpacing(style_mgr.get_scaled_size_param('small_padding'))
        fl.addWidget(QLabel('File name:'), 0, 0)
        fl.addWidget(self._file_name, 0, 1)

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
        v.addWidget(self._dir_view)
        if self._mode == FileDialog.MODE_SAVE:
            v.addLayout(fl)
        v.addLayout(bl)
        self.setLayout(v)

        self._dir_view.selectedEntriesChanged.connect(self._update_entries)
        self._dir_view.commitEntries.connect(self._commit_entries)

        if self._mode == FileDialog.MODE_SAVE:
            self._file_name.fileNameChanged.connect(self._on_file_name_changed)
            self._file_name.selectFileName.connect(self._on_select)

        self._cancel_button.clicked.connect(self.close)
        self._select_button.clicked.connect(self._on_select)

        self._set_current_dir(start_dir)
        if start_file:
            self._file_name.set_init_file_name(start_file)

    def get_paths_with_type_info(self):
        self.exec_()
        return self._final_infos

    def get_path_with_type_info(self):
        assert self._mode != FileDialog.MODE_OPEN_MULT
        infos = self.get_paths_with_type_info()
        if infos:
            assert len(infos) == 1
            return infos[0]
        return None, None

    def get_paths(self):
        infos = self.get_paths_with_type_info()
        if not infos:
            return None
        return [info[0] for info in infos]

    def get_path(self):
        assert self._mode != FileDialog.MODE_OPEN_MULT
        paths = self.get_paths()
        if paths:
            assert len(paths) == 1
            return paths[0]
        return None

    def _update_entries(self):
        if self._mode == FileDialog.MODE_SAVE:
            entries = self._dir_view.get_entries()
            if entries:
                assert len(entries) == 1
                entry = entries[0]
                if entry.is_dir():
                    if entry.name != self._file_name.get_file_name():
                        self._file_name.set_file_name('')
                else:
                    self._file_name.set_file_name(entry.name)

        self._update_select_enabled()

    def _update_select_enabled(self):
        enabled = False

        entries = self._dir_view.get_entries()
        if (self._mode != FileDialog.MODE_SAVE) and (not entries):
            self._select_button.setEnabled(False)
            return

        if self._mode in (FileDialog.MODE_OPEN, FileDialog.MODE_OPEN_MULT):
            paths = [os.path.join(self._current_dir, e.name) for e in entries]
            if paths:
                enabled = all(not e.is_dir() for e in entries)
        elif self._mode == FileDialog.MODE_SAVE:
            file_name = self._file_name.get_file_name()
            path = os.path.join(self._current_dir, file_name)
            if os.path.exists(path) and os.path.isdir(path):
                enabled = False
            else:
                enabled = bool(file_name)
        elif self._mode == FileDialog.MODE_CHOOSE_DIR:
            assert len(entries) == 1
            entry = entries[0]
            enabled = entry.is_dir() and (entry.name != '..')
        else:
            assert False

        self._select_button.setEnabled(enabled)

    def _commit_entries(self):
        entries = self._dir_view.get_entries()

        if entries and entries[0].is_dir():
            assert len(entries) == 1
            entry = entries[0]
            self._update_current_dir(entry.name)
            return

        if self._select_button.isEnabled():
            self._on_select()

    def _on_select(self):
        if self._mode in (FileDialog.MODE_OPEN, FileDialog.MODE_OPEN_MULT):
            entries = self._dir_view.get_entries()
            assert not any(e.is_dir() for e in entries)
            infos = [(os.path.join(self._current_dir, e.name), e.type) for e in entries]
            self._return_infos(infos)

        elif self._mode == FileDialog.MODE_SAVE:
            file_name = self._file_name.get_file_name()
            path = os.path.join(self._current_dir, file_name)

            infos = []
            if os.path.exists(path):
                confirm = lambda: infos.append((path, None))
                dialog = OverwriteConfirmDialog(self._ui_model, path, confirm)
                dialog.exec_()
            else:
                infos = [(path, None)]

            if not infos:
                return
            self._return_infos(infos)

        elif self._mode == FileDialog.MODE_CHOOSE_DIR:
            entries = self._dir_view.get_entries()
            assert len(entries) == 1
            entry = entries[0]
            infos = [os.path.join(self._current_dir, entry.name), None]
            self._return_infos(infos)

    def _return_infos(self, infos):
        if self._mode != FileDialog.MODE_OPEN_MULT:
            assert len(infos) == 1

        self._final_infos = infos
        self.close()

    def _on_file_name_changed(self):
        assert self._mode == FileDialog.MODE_SAVE
        file_name = self._file_name.get_file_name()
        self._dir_view.select_file_name(file_name)

        self._update_select_enabled()

    def _update_current_dir(self, name):
        if name == '..':
            new_dir, _ = os.path.split(self._current_dir)
        else:
            new_dir = os.path.join(self._current_dir, name)

        self._set_current_dir(new_dir)

    def _set_current_dir(self, new_dir):
        if self._current_dir == new_dir:
            return

        self._current_dir = new_dir
        self._dir_branch.set_current_dir(self._current_dir)
        self._dir_view.set_current_dir(self._current_dir)
        self._file_name.set_current_dir(self._current_dir)

        self._dir_history.append(new_dir)

    def sizeHint(self):
        return get_abs_window_size(0.5, 0.5)


_DESCS = {
    FileDialog._TYPE_DIR            : 'Directory',
    FileDialog._TYPE_FILE           : 'File',
    FileDialog._TYPE_INACCESSIBLE   : 'Inaccessible',

    FileDialog.TYPE_KQT     : 'Kunquat module',
    FileDialog.TYPE_KQTI    : 'Kunquat instrument',
    FileDialog.TYPE_KQTE    : 'Kunquat effect',

    FileDialog.TYPE_WAV     : 'Waveform audio file',
    FileDialog.TYPE_AIFF    : 'AIFF audio file',
    FileDialog.TYPE_AU      : 'Sun Au file',
    FileDialog.TYPE_WAVPACK : 'WavPack audio file',
    FileDialog.TYPE_FLAC    : 'FLAC audio file',
}


class OverwriteConfirmDialog(ConfirmDialog):

    def __init__(self, ui_model, path, action_confirm):
        super().__init__(ui_model)

        self._action_confirm = action_confirm

        self.setWindowTitle('Confirm overwrite')

        msg = 'Path {} already exists.'.format(path)
        self._set_message(msg)

        self._cancel_button = QPushButton('Cancel')
        self._overwrite_button = QPushButton('Overwrite')

        b = self._get_button_layout()
        b.addWidget(self._cancel_button)
        b.addWidget(self._overwrite_button)

        self._cancel_button.setFocus(Qt.PopupFocusReason)

        self._overwrite_button.clicked.connect(self._confirm_overwrite)
        self._cancel_button.clicked.connect(self.close)

    def _confirm_overwrite(self):
        self._action_confirm()
        self.close()


class DirectoryBranch(QWidget):

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
        h.addWidget(QLabel('Look in:'))
        h.addWidget(self._path, 1)
        self.setLayout(h)

    def set_current_dir(self, new_dir):
        self._current_dir = new_dir
        self._path.setText(self._current_dir)


class DirEntry():

    _HEADERS = ['Name', 'Size', 'Type', 'Modified']

    @staticmethod
    def get_header_by_index(index):
        return DirEntry._HEADERS[index]

    @staticmethod
    def get_header_count():
        return len(DirEntry._HEADERS)

    def __init__(self, dir_entry_or_custom):
        if isinstance(dir_entry_or_custom, os.DirEntry):
            dir_entry = dir_entry_or_custom
            self.name = dir_entry.name
            self.type = (FileDialog._TYPE_DIR if dir_entry.is_dir()
                    else FileDialog._TYPE_FILE)
            sr = dir_entry.stat()
            self.size = sr.st_size
            self.modified = sr.st_mtime
        else:
            fname, ftype, sr = dir_entry_or_custom
            self.name = fname
            self.type = ftype
            if sr:
                self.size = sr.st_size
                self.modified = sr.st_mtime
            else:
                self.size = 0
                self.modified = None

        self.checked = False
        self.is_visible = self.is_dir()

    def is_dir(self):
        return (self.type == FileDialog._TYPE_DIR)

    def get_size_desc(self):
        if self.is_dir():
            return ''

        if self.size == 1:
            return '1 byte'
        elif self.size < 1024:
            return '{} bytes'.format(self.size)

        suffixes = ['KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB']
        min_sizes = (2**(10 * (x + 1)) for x in range(len(suffixes)))
        suffix = ''
        min_size = 0
        for next_suffix, next_min_size in zip(suffixes, min_sizes):
            if self.size < next_min_size:
                break
            suffix = next_suffix
            min_size = next_min_size

        return '{:.1f} {}'.format(self.size / min_size, suffix)

    def get_type_desc(self):
        return _DESCS[self.type]

    def get_modified_date(self):
        if self.modified == None:
            return ''
        try:
            dt = datetime.datetime.fromtimestamp(self.modified)
        except (OverflowError, OSError):
            return ''
        return dt.isoformat(sep=' ', timespec='seconds')

    def get_sort_key(self):
        return (0 if self.is_dir() else 1,
                self.name.lower() if self.name != '..' else '')

    def get_field_by_index(self, index):
        if index == 0:
            return self.name
        elif index == 1:
            return self.get_size_desc()
        elif index == 2:
            return self.get_type_desc()
        elif index == 3:
            return self.get_modified_date()

        assert False


class DirectoryModel(QAbstractTableModel):

    scanDirFinished = Signal(name='scanDirFinished')

    _HEADERS = [DirEntry.get_header_by_index(i) for i in range(4)]

    _ICON_NAMES = {
        FileDialog._TYPE_DIR            : 'file_directory',
        FileDialog._TYPE_FILE           : 'file_generic',
        FileDialog._TYPE_INACCESSIBLE   : 'file_error',

        FileDialog.TYPE_KQT     : 'file_kqt',
        FileDialog.TYPE_KQTI    : 'file_kqti',
        FileDialog.TYPE_KQTE    : 'file_kqte',

        FileDialog.TYPE_WAV     : 'file_sample',
        FileDialog.TYPE_AIFF    : 'file_sample',
        FileDialog.TYPE_AU      : 'file_sample',
        FileDialog.TYPE_WAVPACK : 'file_sample',
        FileDialog.TYPE_FLAC    : 'file_sample',
    }

    def __init__(self, ui_model, filter_mask):
        super().__init__()
        self._ui_model = ui_model
        self._current_dir = None
        self._all_entries = []
        self._entries = []
        self._show_all_files = False
        self._entry_filters = EntryFilters(filter_mask)
        self._entry_checker = None
        self._scan_error = None
        self._fs_watcher = QFileSystemWatcher()
        self._icons = {}

        self._fs_watcher.directoryChanged.connect(self._on_dir_modified)
        self._fs_watcher.fileChanged.connect(self._on_file_modified)

    def _try_add_parent_dir(self, entries):
        parent_dir, _ = os.path.split(self._current_dir)
        if parent_dir == self._current_dir:
            return entries
        try:
            pstat = os.stat(parent_dir)
        except OSError:
            pstat = None
        parent_entry = DirEntry(('..', FileDialog._TYPE_DIR, pstat))
        return [parent_entry] + entries

    def _try_scan_directory(self, directory):
        entries = []
        scan_error = None

        try:
            with os.scandir(directory) as es:
                for e in es:
                    entry = DirEntry(e)
                    entries.append(entry)
        except OSError as e:
            entries = []
            _, last = os.path.split(directory)
            if isinstance(e, FileNotFoundError):
                desc = 'Directory not found'
            else:
                desc = e.strerror
            scan_error = 'Could not access directory: {}'.format(desc)

        return entries, scan_error

    def set_directory(self, new_dir):
        self._entry_checker = None

        if self._current_dir:
            self._fs_watcher.removePath(self._current_dir)
            watched_files = self._fs_watcher.files()
            if watched_files:
                self._fs_watcher.removePaths(watched_files)

        self._current_dir = new_dir

        self.beginResetModel()
        self._all_entries = []
        self._entries = []

        new_all_entries, self._scan_error = self._try_scan_directory(self._current_dir)

        self._all_entries = self._try_add_parent_dir(new_all_entries)
        self._all_entries.sort(key=lambda e: e.get_sort_key())

        self._fill_known_entries()

        self.dataChanged.emit(
                self.index(0, 0),
                self.index(len(self._entries) - 1, DirEntry.get_header_count() - 1))

        self._entry_checker = self._check_entries()

        self.endResetModel()

        if not self._scan_error:
            self._fs_watcher.addPath(self._current_dir)

        self.scanDirFinished.emit()

    def get_scan_error(self):
        return self._scan_error

    def _on_dir_modified(self, dir_path):
        if dir_path != self._current_dir:
            return

        self._entry_checker = None

        new_all_entries = []
        self._scan_error = None

        new_all_entries, self._scan_error = self._try_scan_directory(self._current_dir)

        new_all_entries = self._try_add_parent_dir(new_all_entries)
        new_all_entries.sort(key=lambda e: e.get_sort_key())

        final_all_entries = []
        get_old = (e for e in self._all_entries)
        get_new = (e for e in new_all_entries)
        cur_old_entry = next(get_old, None)
        cur_new_entry = next(get_new, None)
        while cur_old_entry and cur_new_entry:
            if cur_old_entry.name == cur_new_entry.name:
                if (cur_old_entry.checked and
                        (cur_old_entry.modified == cur_new_entry.modified)):
                    final_all_entries.append(cur_old_entry)
                else:
                    final_all_entries.append(cur_new_entry)
                cur_old_entry = next(get_old, None)
                cur_new_entry = next(get_new, None)
            elif cur_old_entry.get_sort_key() < cur_new_entry.get_sort_key():
                # Old entry removed
                cur_old_entry = next(get_old, None)
            elif cur_old_entry.get_sort_key() > cur_new_entry.get_sort_key():
                # New entry added
                final_all_entries.append(cur_new_entry)
                new_path = os.path.join(self._current_dir, cur_new_entry.name)
                self._fs_watcher.addPath(new_path)
                cur_new_entry = next(get_new, None)
            else:
                assert False

        while cur_new_entry:
            final_all_entries.append(cur_new_entry)
            new_path = os.path.join(self._current_dir, cur_new_entry.name)
            self._fs_watcher.addPath(new_path)
            cur_new_entry = next(get_new, None)

        self.beginResetModel()
        self._all_entries = final_all_entries
        self._fill_known_entries()
        self._entry_checker = self._check_entries()
        self.endResetModel()

        self.scanDirFinished.emit()

    def _on_file_modified(self, file_path):
        dir_path, file_name = os.path.split(file_path)
        if (not os.path.exists(file_path)) or (dir_path != self._current_dir):
            self._fs_watcher.removePath(file_path)
            return

        for entry in self._all_entries:
            if entry.name == file_name:
                entry.checked = False
                break
        self._on_dir_modified(dir_path)

    def _is_entry_file_visible(self, entry):
        return (self._show_all_files or
                (not entry.name.startswith('.')) or
                (entry.name == '..'))

    def _fill_known_entries(self):
        self._entries = []
        for entry in self._all_entries:
            if entry.is_dir() or (entry.checked and entry.is_visible):
                if self._is_entry_file_visible(entry):
                    self._entries.append(entry)

    def has_unchecked_entries(self):
        return (self._entry_checker != None)

    def step_check_entries(self):
        if not self._entry_checker:
            return []

        try:
            added_entries = next(self._entry_checker)
            return added_entries
        except StopIteration:
            self._entry_checker = None

        return []

    def get_entries(self):
        return self._entries

    def _check_entries(self):
        current_dir = self._current_dir

        run_start_time = time.time()

        added_entries = []

        row = 0
        for entry in self._all_entries:
            assert row <= len(self._entries)
            shown_entry = None
            if row < len(self._entries):
                shown_entry = self._entries[row]
                assert entry.get_sort_key() <= shown_entry.get_sort_key()

            if entry.is_dir() or entry.checked:
                if shown_entry and (entry.name == shown_entry.name):
                    row += 1
                continue

            # Apply filtering
            path = os.path.join(current_dir, entry.name)
            self._entry_filters.filter_entry(path, entry)
            if entry.is_visible:
                if self._is_entry_file_visible(entry):
                    if shown_entry and (entry.name == shown_entry.name):
                        self._entries[row] = entry
                        self.dataChanged.emit(
                                self.index(row, 0),
                                self.index(row, DirEntry.get_header_count() - 1))
                    else:
                        self.beginInsertRows(QModelIndex(), row, row)
                        self._entries.insert(row, entry)
                        added_entries.append(entry)
                        self.endInsertRows()
                        row += 1

            entry.checked = True

            # Yield if needed to keep the dialog responsive
            cur_time = time.time()
            if (cur_time - run_start_time) > 0.03:
                yield added_entries
                added_entries = []
                run_start_time = time.time()

        yield added_entries

    def _get_icon(self, entry):
        icon_name = self._ICON_NAMES.get(entry.type, '')
        if icon_name not in self._icons:
            icon_bank = self._ui_model.get_icon_bank()
            style_mgr = self._ui_model.get_style_manager()
            size = style_mgr.get_scaled_size_param('file_dialog_icon_size')
            icon_size = QSize(size, size)

            if icon_name:
                image = QImage()
                image.load(icon_bank.get_icon_path(icon_name))
                image = image.convertToFormat(QImage.Format_ARGB32_Premultiplied)
                icon = image.scaled(
                        icon_size, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            else:
                icon = QImage(icon_size, QImage.Format_ARGB32_Premultiplied)
                icon.fill(0)

            self._icons[icon_name] = icon

        return self._icons[icon_name]

    # Qt interface

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._entries)

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return DirEntry.get_header_count()

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row, column = index.row(), index.column()
            if (0 <= column < DirEntry.get_header_count() and
                    0 <= row < len(self._entries)):
                return self._entries[row].get_field_by_index(column)
        elif role == Qt.DecorationRole:
            row, column = index.row(), index.column()
            if (column == 0) and (0 <= row < len(self._entries)):
                entry = self._entries[row]
                return self._get_icon(entry)

        return None

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole:
            if orientation == Qt.Horizontal:
                if 0 <= section < DirEntry.get_header_count():
                    return DirEntry.get_header_by_index(section)

        return None


class DirectoryView(QTreeView):

    selectedEntriesChanged = Signal(name='selectedEntriesChanged')
    commitEntries = Signal(name='commitEntries')
    stepCheckEntries = Signal(name='stepCheckEntries')

    def __init__(self, ui_model, filter_mask, allow_multiple):
        super().__init__()

        self._ui_model = ui_model
        self._current_dir = None
        self._allow_multiple = allow_multiple
        self._model = DirectoryModel(self._ui_model, filter_mask)

        self.setSelectionBehavior(QAbstractItemView.SelectRows)
        if allow_multiple:
            self.setSelectionMode(QAbstractItemView.ExtendedSelection)
        else:
            self.setSelectionMode(QAbstractItemView.SingleSelection)

        self._entries = []
        self._prev_entries = []

        self._suggested_file_name = None

        self._error_label = QLabel(self)
        self._error_label.hide()

        style_mgr = self._ui_model.get_style_manager()

        self.setModel(self._model)

        self.setUniformRowHeights(True)
        self.setRootIsDecorated(False)

        header = self.header()
        header.setStretchLastSection(True)
        header.resizeSection(0, style_mgr.get_scaled_size(40))
        header.resizeSection(1, style_mgr.get_scaled_size(9))
        header.resizeSection(2, style_mgr.get_scaled_size(20))
        header.resizeSection(3, style_mgr.get_scaled_size(15))

        self._model.modelReset.connect(self._step_check_entries)
        self._model.scanDirFinished.connect(self._on_dir_updated)

        self.selectionModel().selectionChanged.connect(self._on_selection_changed)
        self.doubleClicked.connect(self._on_double_clicked)

        self.stepCheckEntries.connect(self._step_check_entries, Qt.QueuedConnection)

    def set_current_dir(self, new_dir):
        if self._current_dir == new_dir:
            return

        self._current_dir = new_dir
        self._model.set_directory(self._current_dir)

        sm = self.selectionModel()
        new_entries = self._model.get_entries()
        if new_entries:
            self._select_rows([0])
        else:
            self._clear_selection()

        if self._model.has_unchecked_entries():
            self.stepCheckEntries.emit()

    def _on_dir_updated(self):
        error = self._model.get_scan_error()
        if error:
            self._error_label.setText(error)

            # Estimate actual text size as the label doesn't know it yet
            style_mgr = self._ui_model.get_style_manager()
            font_family, font_size = get_default_font_info(style_mgr)
            font = QFont(font_family, font_size)
            fm = QFontMetrics(font, self)
            rect = fm.boundingRect(self._error_label.text())

            x = (self.width() - rect.width()) // 2
            y = (self.height() - rect.height()) // 2
            self._error_label.move(x, y)
            self._error_label.show()
        else:
            self._error_label.hide()

    def get_entries(self):
        assert self._is_selection_valid(self._entries)
        return self._entries

    def _select_rows(self, rows):
        sm = self.selectionModel()

        selection = None
        for row in rows:
            left_index = self._model.createIndex(row, 0)
            right_index = self._model.createIndex(row, DirEntry.get_header_count() - 1)
            new_selection = QItemSelection(left_index, right_index)
            if selection:
                selection.merge(new_selection, QItemSelectionModel.Select)
            else:
                selection = new_selection

        sm.select(selection, QItemSelectionModel.ClearAndSelect)
        old_block = sm.blockSignals(True)
        sm.setCurrentIndex(left_index, QItemSelectionModel.Select)
        sm.blockSignals(old_block)

    def _clear_selection(self):
        sm = self.selectionModel()
        sm.select(QItemSelection(), QItemSelectionModel.ClearAndSelect)
        old_block = sm.blockSignals(True)
        sm.setCurrentIndex(QModelIndex(), QItemSelectionModel.Select)
        sm.blockSignals(old_block)

    def _try_select_suggested_file_name(self):
        if not self._suggested_file_name:
            return

        entries = self._model.get_entries()
        for row, entry in enumerate(entries):
            if entry.name == self._suggested_file_name:
                self._select_rows([row])
                self._suggested_file_name = None
                break
        else:
            self._clear_selection()

    def select_file_name(self, file_name):
        self._suggested_file_name = file_name
        self._try_select_suggested_file_name()

    def _step_check_entries(self):
        added_entries = self._model.step_check_entries()
        if (self._suggested_file_name and
                any(e.name == self._suggested_file_name for e in added_entries)):
            self._try_select_suggested_file_name()

        if self._model.has_unchecked_entries():
            self.stepCheckEntries.emit()
        else:
            # An ugly hack, but can't think of anything better now :-P
            self.header().setStretchLastSection(False)
            self.header().setStretchLastSection(True)

    def _get_selected_entries(self, selection):
        all_entries = self._model.get_entries()
        entries = []
        indices = selection
        if isinstance(selection, QItemSelection):
            indices = selection.indexes()
        for index in indices:
            if index.column() == 0:
                entries.append(all_entries[index.row()])
        return entries

    def _is_selection_valid(self, entries):
        return (all(not e.is_dir() for e in entries) or (len(entries) == 1))

    def _get_row(self, entry):
        for i, e in enumerate(self._model.get_entries()):
            if e == entry:
                return i
        return -1

    def _on_selection_changed(self, selected, deselected):
        sm = self.selectionModel()
        new_entries = self._get_selected_entries(sm.selectedIndexes())
        if self._is_selection_valid(new_entries):
            self._prev_entries = self._entries
            self._entries = new_entries
            self.selectedEntriesChanged.emit()
        else:
            used_entries = None
            if self._is_selection_valid(self._entries):
                used_entries = self._entries
            elif self._is_selection_valid(self._prev_entries):
                used_entries = self._prev_entries

            rows = []
            if used_entries:
                for entry in used_entries:
                    row = self._get_row(entry)
                    if row >= 0:
                        rows.append(row)
            self._select_rows(rows if rows else [0])

    def _on_double_clicked(self, index):
        self.commitEntries.emit()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Return:
            self.commitEntries.emit()
            event.accept()
            return

        event.ignore()
        super().keyPressEvent(event)

    def resizeEvent(self, event):
        x = (self.width() - self._error_label.width()) // 2
        y = (self.height() - self._error_label.height()) // 2
        self._error_label.move(x, y)


class FileNameValidator(QValidator):

    def __init__(self, current_dir):
        super().__init__()
        self._current_dir = current_dir

    def _get_file_name_validity(self, file_name):
        if file_name in ('', '.', '..'):
            return QValidator.Intermediate
        if os.sep in file_name:
            return QValidator.Invalid

        path = os.path.join(self._current_dir, file_name)

        # See if the full path is valid
        try:
            os.stat(path)
        except FileNotFoundError:
            pass
        except OSError as e:
            if hasattr(e, 'winerror'):
                invalid_name = 123
                if e.winerror == invalid_name:
                    return QValidator.Intermediate
            elif e.errno in (errno.ENAMETOOLONG, errno.ERANGE):
                return QValidator.Invalid
        except (TypeError, ValueError):
            return QValidator.Invalid

        return QValidator.Acceptable

    def is_file_name_valid(self, file_name):
        return (self._get_file_name_validity(file_name) == QValidator.Acceptable)

    def validate(self, in_text, pos):
        if not in_text:
            return (QValidator.Intermediate, in_text, pos)

        return (self._get_file_name_validity(in_text), in_text, pos)


class FileName(QLineEdit):

    fileNameChanged = Signal(name='fileNameChanged')
    selectFileName = Signal(name='selectFileName')

    def __init__(self):
        super().__init__()
        self._current_dir = None
        self.textChanged.connect(self._on_name_changed)

    def set_current_dir(self, new_dir):
        if self._current_dir == new_dir:
            return

        self._current_dir = new_dir
        self.setValidator(FileNameValidator(self._current_dir))

        old_block = self.blockSignals(True)
        self.setText('')
        self.blockSignals(old_block)

    def set_init_file_name(self, init_name):
        self.setText(init_name)

    def set_file_name(self, new_name):
        old_block = self.blockSignals(True)
        self.setText(new_name)
        self.blockSignals(old_block)

    def get_file_name(self):
        text = self.text()
        if (not self.validator()) or (not self.validator().is_file_name_valid(text)):
            return ''
        return text

    def _on_name_changed(self, new_text):
        self.fileNameChanged.emit()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Return:
            if self.get_file_name():
                self.selectFileName.emit()
            return

        event.ignore()
        super().keyPressEvent(event)


def is_valid_kqt_key(key, magic_id):
    key_components = key.split('/')
    if key_components[0] != magic_id:
        return False
    if not key_components[1:]:
        return False
    if not key_components[-1]:
        return True # ignore directories
    return ('.' in key_components[-1])


def get_au_type(manifest_data):
    try:
        decoded = json.loads(str(manifest_data, encoding='utf-8'))
        version, manifest_dict = decoded
        if version != 0:
            return False
        au_type = manifest_dict['type']
    except (KeyError, ValueError):
        return None
    return au_type


def filter_any_kqt_entry(path, dir_entry, allowed_formats):
    magic_id = None
    kqt_type = None
    is_au = False

    try:
        with zipfile.ZipFile(path, mode='r') as zfile:
            for entry in zfile.infolist():
                if magic_id == None:
                    key_components = entry.filename.split('/')
                    if key_components[0] not in ('kqtc00', 'kqti00'):
                        return False
                    magic_id = key_components[0]
                    if magic_id == 'kqtc00':
                        kqt_type = FileDialog.TYPE_KQT
                    elif magic_id == 'kqti00':
                        is_au = True
                    else:
                        return False

                if not is_valid_kqt_key(entry.filename, magic_id):
                    return False

                if (not kqt_type) and is_au:
                    key_components = entry.filename.split('/')
                    if (len(key_components) == 2 and
                            key_components[-1] == 'p_manifest.json'):
                        data = zfile.read(entry)
                        if get_au_type(data) == 'instrument':
                            kqt_type = FileDialog.TYPE_KQTI
                        elif get_au_type(data) == 'effect':
                            kqt_type = FileDialog.TYPE_KQTE
                        else:
                            return False

    except zipfile.BadZipFile:
        return False

    if not kqt_type:
        return False

    dir_entry.type = kqt_type
    return True


def get_sndfile_format(path):
    try:
        sf = SndFileR(path)
        return sf.get_major_format()
    except SndFileError:
        pass
    return None


sndfile_format_descs = {
    'wav'   : FileDialog.TYPE_WAV,
    'aiff'  : FileDialog.TYPE_AIFF,
    'au'    : FileDialog.TYPE_AU,
    'flac'  : FileDialog.TYPE_FLAC,
}


def filter_sndfile_entry(path, dir_entry, allowed_formats):
    path_format = get_sndfile_format(path)
    if path_format in allowed_formats:
        dir_entry.type = sndfile_format_descs[path_format]
        return True
    return False


def filter_wavpack_entry(path, dir_entry):
    # Creating a WavPack context for format detection is too slow,
    # so let's just check a couple of fields manually
    with open(path, 'rb') as f:
        maybe_ckid = f.read(4)
        if maybe_ckid != b'wvpk':
            return False
        cksize = f.read(4)
        version_bytes = f.read(2)
        version = version_bytes[0] + version_bytes[1] * 0x100
        if not (0x402 <= version <= 0x410):
            return False

    dir_entry.type = FileDialog.TYPE_WAVPACK
    return True


def filter_any_entry(path, dir_entry):
    return True


class EntryFilters():

    _FUNCS = {
        FileDialog.TYPE_WAVPACK   : filter_wavpack_entry,
        FileDialog.TYPE_ANY       : filter_any_entry,
    }

    def __init__(self, filter_mask):
        self._used_filters = []

        # Combine Kunquat format checks to reduce the number of tests
        allowed_kqt_formats = (filter_mask & FileDialog.TYPE_ALL_KQT)
        if allowed_kqt_formats != 0:
            kqt_filter = lambda p, d: filter_any_kqt_entry(p, d, allowed_kqt_formats)

        # Combine sndfile-based checks to reduce the number of tests
        sndfile_mask = (FileDialog.TYPE_WAV |
                FileDialog.TYPE_AIFF |
                FileDialog.TYPE_AU |
                FileDialog.TYPE_FLAC)
        if (filter_mask & sndfile_mask) != 0:
            allowed_formats = set()
            format_names = {
                FileDialog.TYPE_WAV:  'wav',
                FileDialog.TYPE_AIFF: 'aiff',
                FileDialog.TYPE_AU:   'au',
                FileDialog.TYPE_FLAC: 'flac',
            }
            for bit, name in format_names.items():
                if (bit & filter_mask) != 0:
                    allowed_formats.add(name)

            sndfile_filter = lambda p, d: filter_sndfile_entry(p, d, allowed_formats)

        mask_left = filter_mask
        filter_bit = 1
        while mask_left:
            if (mask_left & 1) != 0:
                if (filter_bit & allowed_kqt_formats) != 0:
                    if kqt_filter:
                        self._used_filters.append(kqt_filter)
                        kqt_filter = None
                elif (filter_bit & sndfile_mask) != 0:
                    if sndfile_filter:
                        self._used_filters.append(sndfile_filter)
                        sndfile_filter = None
                else:
                    self._used_filters.append(EntryFilters._FUNCS[filter_bit])
            filter_bit <<= 1
            mask_left >>= 1

    def filter_entry(self, path, entry):
        entry.is_visible = False
        for f in self._used_filters:
            try:
                if f(path, entry):
                    entry.is_visible = True
                    return True
            except OSError:
                entry.type = FileDialog._TYPE_INACCESSIBLE
                entry.is_visible = (filter_any_entry in self._used_filters)
                break
        return False


