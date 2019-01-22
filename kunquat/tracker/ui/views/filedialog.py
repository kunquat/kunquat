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
import os
import os.path

from kunquat.tracker.ui.qt import *

from .utils import get_abs_window_size


class FileDialog(QDialog):

    MODE_OPEN = 'open'
    MODE_OPEN_MULT = 'open_mult'
    MODE_SAVE = 'save'
    MODE_CHOOSE_DIR = 'choose_dir'

    def __init__(self, ui_model, mode, title, start_dir):
        super().__init__()

        self._ui_model = ui_model
        self._mode = mode
        self._current_dir = None
        self._final_paths = None

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

        self._dir_branch = DirectoryBranch(self._ui_model)
        self._dir_view = DirectoryView(self._ui_model)
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

        self._cancel_button.clicked.connect(self.close)
        self._select_button.clicked.connect(self._on_select)

        self._set_current_dir(start_dir)

    def get_paths(self):
        self.exec_()
        return self._final_paths

    def _update_entries(self):
        self._update_select_enabled()

    def _update_select_enabled(self):
        enabled = False

        entries = self._dir_view.get_entries()
        if not entries:
            self._select_button.setEnabled(False)
            return

        if self._mode in (FileDialog.MODE_OPEN, FileDialog.MODE_OPEN_MULT):
            paths = [os.path.join(self._current_dir, e.name) for e in entries]
            if paths:
                enabled = all(not e.is_dir() for e in entries)
        elif self._mode == FileDialog.MODE_SAVE:
            assert len(entries) == 1
            entry = entries[0]
            raise NotImplementedError
        elif self._mode == FileDialog.MODE_CHOOSE_DIR:
            assert len(entries) == 1
            entry = entries[0]
            raise NotImplementedError
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

        if self._mode in (FileDialog.MODE_OPEN, FileDialog.MODE_OPEN_MULT):
            print([e.name for e in entries])
        elif self._mode == FileDialog.MODE_SAVE:
            raise NotImplementedError
        elif self._mode == FileDialog.MODE_CHOOSE_DIR:
            raise NotImplementedError
        else:
            assert False

    def _on_select(self):
        current_file = self._file_name.get_file_name()

        assert self._current_dir
        assert current_file

        # TODO: Confirm overwrite in save mode

        self._selected_path = os.path.join(self._current_dir, current_file)

        self.close()

    def _update_current_dir(self, name):
        if name == '..':
            new_dir, _ = os.path.split(self._current_dir)
        else:
            new_dir = os.path.join(self._current_dir, name)

        if os.path.exists(new_dir):
            self._set_current_dir(new_dir)

    def _set_current_dir(self, new_dir):
        if self._current_dir == new_dir:
            return

        self._current_dir = new_dir
        try:
            self._dir_branch.set_current_dir(self._current_dir)
            self._dir_view.set_current_dir(self._current_dir)
            self._file_name.set_current_dir(self._current_dir)
        except OSError as e:
            # Display access error dialog
            base, dir_name = os.path.split(new_dir)
            if not dir_name:
                _, dir_name = os.path.split(base)
            message = 'Could not access directory "{}":\n{}'.format(
                    dir_name, e.strerror)
            err_dialog = ErrorDialog(
                    self._ui_model,
                    'Could not access directory',
                    message)
            err_dialog.exec_()

            # Return to previous directory
            if self._dir_history:
                prev_dir = self._dir_history.pop()
            else:
                prev_dir = os.path.abspath(os.sep)
            self._set_current_dir(prev_dir)
            return

        self._dir_history.append(new_dir)

    def sizeHint(self):
        return get_abs_window_size(0.5, 0.5)


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
            self.type = 'Directory' if dir_entry.is_dir() else 'File'
            sr = dir_entry.stat()
            self.size = sr.st_size
            self.modified = sr.st_mtime
        else:
            fname, ftype, sr = dir_entry_or_custom
            self.name = fname
            self.type = ftype
            self.size = sr.st_size
            self.modified = sr.st_mtime

    def is_dir(self):
        return self.type == 'Directory'

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

    def get_modified_date(self):
        try:
            dt = datetime.datetime.fromtimestamp(self.modified)
        except (OverflowError, OSError):
            return ''
        return dt.isoformat(sep=' ', timespec='seconds')

    def get_sort_key(self):
        return (0 if self.is_dir() else 1, self.name)

    def get_field_by_index(self, index):
        if index == 0:
            return self.name
        elif index == 1:
            return self.get_size_desc()
        elif index == 2:
            return self.type
        elif index == 3:
            return self.get_modified_date()

        assert False


class DirectoryModel(QAbstractTableModel):

    _HEADERS = [DirEntry.get_header_by_index(i) for i in range(4)]

    def __init__(self):
        super().__init__()
        self._current_dir = None
        self._entries = []

    def set_directory(self, new_dir):
        self._current_dir = new_dir

        self._entries = []

        with os.scandir(self._current_dir) as es:
            for e in es:
                self._entries.append(DirEntry(e))

        self._entries.sort(key=lambda e: e.get_sort_key())

        parent_dir, _ = os.path.split(self._current_dir)
        if parent_dir != self._current_dir:
            pstat = os.stat(parent_dir)
            parent_entry = DirEntry(('..', 'Directory', pstat))
            self._entries = [parent_entry] + self._entries

        self._update_data()

    def get_entries(self):
        return self._entries

    def _update_data(self):
        self.dataChanged.emit(
                self.index(0, 0),
                self.index(len(self._entries) - 1, DirEntry.get_header_count() - 1))

    def _to_size_desc(self, byte_count):
        if byte_count == 1:
            return '1 byte'
        elif byte_count < 1024:
            return '{} bytes'.format(byte_count)

        suffixes = ['KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB']
        mult = 1024
        for s in suffixes[:-1]:
            next_mult = mult * 1024
            if mult <= byte_count < next_mult:
                return '{:.1f} {}'.format(byte_count / mult, s)
            mult = next_mult

        return '{:.1f} {}'.format(byte_count / mult, suffixes[-1])

    def _to_date(self, timestamp):
        try:
            dt = datetime.datetime.fromtimestamp(timestamp)
        except (OverflowError, OSError):
            return ''
        return dt.isoformat(sep=' ', timespec='seconds')

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

    def __init__(self, ui_model):
        super().__init__()

        self._ui_model = ui_model
        self._current_dir = None
        self._model = DirectoryModel()

        self._entries = []
        self._prev_selection = QItemSelection()

        style_mgr = self._ui_model.get_style_manager()

        self.setModel(self._model)

        header = self.header()
        header.setStretchLastSection(True)
        header.resizeSection(0, style_mgr.get_scaled_size(40))
        header.resizeSection(1, style_mgr.get_scaled_size(9))
        header.resizeSection(2, style_mgr.get_scaled_size(20))
        header.resizeSection(3, style_mgr.get_scaled_size(15))

        self.selectionModel().selectionChanged.connect(self._on_selection_changed)
        self.doubleClicked.connect(self._on_double_clicked)

    def set_current_dir(self, new_dir):
        self._current_dir = new_dir
        self._model.set_directory(self._current_dir)

    def get_entries(self):
        assert self._is_selection_valid(self._entries)
        return self._entries

    def _get_selected_entries(self, selection):
        all_entries = self._model.get_entries()
        entries = []
        for index in selection.indexes():
            if index.column() == 0:
                entries.append(all_entries[index.row()])
        return entries

    def _is_selection_valid(self, entries):
        return (all(not e.is_dir() for e in entries) or (len(entries) == 1))

    def _on_selection_changed(self, selected, deselected):
        new_entries = self._get_selected_entries(selected)
        if self._is_selection_valid(new_entries):
            self._entries = new_entries
            self._prev_selection = selected
            self.selectedEntriesChanged.emit()
        else:
            prev_selection = self._prev_selection
            self._prev_selection = QItemSelection()
            prev_entries = self._get_selected_entries(prev_selection)
            if self._is_selection_valid(prev_entries):
                self.selectionModel().select(prev_selection)
            else:
                self.selectionModel().select(QItemSelection())

    def _on_double_clicked(self, index):
        self.commitEntries.emit()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Return:
            self.commitEntries.emit()
            event.accept()
            return

        event.ignore()
        super().keyPressEvent(event)


class FileNameValidator(QValidator):

    def __init__(self, current_dir):
        super().__init__()
        self._current_dir = current_dir

    def _is_file_name_valid(self, file_name):
        if file_name in ('', '.', '..'):
            return False

        path = os.path.join(self._current_dir, file_name)

        # See if the full path is valid
        try:
            os.stat(path)
        except OSError as e:
            if hasattr(e, 'winerror'):
                invalid_name = 123
                if e.winerror == invalid_name:
                    return False
            elif e.errno in (errno.ENAMETOOLONG, errno.ERANGE):
                return False
        except ValueError:
            return False

        containing_dir, _ = os.path.split(path)
        if not os.path.samefile(containing_dir, path):
            # File name contains a directory component
            return False

        return True

    def validate(self, in_text, pos):
        if not in_text:
            return (QValidator.Intermediate, in_text, pos)
        if not self._is_file_name_valid(in_text):
            return (QValidator.Invalid, in_text, pos)

        return (QValidator.Acceptable, in_text, pos)


class FileName(QLineEdit):

    fileNameChanged = Signal(name='fileNameChanged')

    def __init__(self):
        super().__init__()
        self._current_dir = None
        self.textEdited.connect(self._on_name_changed)

    def set_current_dir(self, new_dir):
        if self._current_dir == new_dir:
            return

        self._current_dir = new_dir
        self.setValidator(FileNameValidator(self._current_dir))

        old_block = self.blockSignals(True)
        self.setText('')
        self.blockSignals(old_block)

    def get_file_name(self):
        return self.text()

    def _on_name_changed(self, new_text):
        self.fileNameChanged.emit()


class ErrorDialog(QDialog):

    def __init__(self, ui_model, title, message):
        super().__init__()
        style_mgr = ui_model.get_style_manager()
        icon_bank = ui_model.get_icon_bank()

        self.setWindowTitle(title)

        error_img_orig = QPixmap(icon_bank.get_icon_path('error'))
        error_img = error_img_orig.scaledToWidth(
                style_mgr.get_scaled_size_param('dialog_icon_size'),
                Qt.SmoothTransformation)
        error_label = QLabel()
        error_label.setPixmap(error_img)

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        margin = style_mgr.get_scaled_size_param('large_padding')
        h.setContentsMargins(margin, margin, margin, margin)
        h.setSpacing(margin * 2)
        h.addWidget(error_label)
        h.addWidget(self._message)

        self._button_layout = QHBoxLayout()

        v = QVBoxLayout()
        v.addLayout(h)
        v.addLayout(self._button_layout)

        self.setLayout(v)

        # Dialog contents, TODO: make a common ErrorDialog class
        msg_lines = message.split('\n')
        format_msg = '<p>{}</p>'.format('<br>'.join(msg_lines))
        self._message.setText(format_msg)

        ok_button = QPushButton('OK')
        self._button_layout.addStretch(1)
        self._button_layout.addWidget(ok_button)
        self._button_layout.addStretch(1)

        ok_button.clicked.connect(self.close)


