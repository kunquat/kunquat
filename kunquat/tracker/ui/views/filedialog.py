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

import os.path

from kunquat.tracker.ui.qt import *

from .utils import get_abs_window_size


class FileDialog(QDialog):

    MODE_OPEN = 'open'
    MODE_SAVE = 'save'
    MODE_CHOOSE_DIR = 'choose_dir'

    def __init__(self, ui_model, mode, title, start_dir):
        super().__init__()

        self._ui_model = ui_model
        self._selected_path = None

        self.setWindowTitle(title)

        style_mgr = self._ui_model.get_style_manager()

        if mode == FileDialog.MODE_OPEN:
            select_text = 'Open'
        elif mode == FileDialog.MODE_SAVE:
            select_text = 'Save'
        elif mode == FileDialog.MODE_CHOOSE_DIR:
            select_text = 'Choose'

        self._dir_branch = DirectoryBranch(self._ui_model, start_dir)
        self._dir_view = DirectoryView(self._ui_model, start_dir)
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
        v.addLayout(fl)
        v.addLayout(bl)
        self.setLayout(v)

        self._cancel_button.clicked.connect(self.close)
        self._select_button.clicked.connect(self._on_select)

    def get_path(self):
        self.exec_()
        return self._selected_path

    def _on_select(self):
        current_dir = self._dir_view.get_current_dir()
        current_file = self._file_name.get_file_name()

        assert current_dir
        assert current_file

        # TODO: Confirm overwrite in save mode

        self._selected_path = os.path.join(current_dir, current_file)

        self.close()

    def sizeHint(self):
        return get_abs_window_size(0.5, 0.5)


class DirectoryBranch(QWidget):

    def __init__(self, ui_model, start_dir):
        super().__init__()

        self._ui_model = ui_model
        self._current_dir = start_dir

        self._path = QLabel(start_dir)

        style_mgr = self._ui_model.get_style_manager()

        h = QHBoxLayout()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        h.setContentsMargins(margin, margin, margin, margin)
        h.setSpacing(style_mgr.get_scaled_size_param('large_padding'))
        h.addWidget(QLabel('Look in:'))
        h.addWidget(self._path, 1)
        self.setLayout(h)


class DirectoryModel(QAbstractTableModel):

    _HEADERS = ['Name', 'Size', 'Type', 'Modified']

    def __init__(self):
        super().__init__()

    # Qt interface

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return 0 # TODO: entry count

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._HEADERS)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            return None # TODO

        return None

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole:
            if orientation == Qt.Horizontal:
                if 0 <= section < len(self._HEADERS):
                    return self._HEADERS[section]

        return None


class DirectoryView(QTableView):

    def __init__(self, ui_model, start_dir):
        super().__init__()

        self._ui_model = ui_model
        self._current_dir = start_dir
        self._model = DirectoryModel()

        style_mgr = self._ui_model.get_style_manager()

        self.setModel(self._model)

        hh = self.horizontalHeader()
        hh.setStretchLastSection(True)
        hh.resizeSection(0, style_mgr.get_scaled_size(40))
        hh.resizeSection(1, style_mgr.get_scaled_size(8))
        hh.resizeSection(2, style_mgr.get_scaled_size(20))
        hh.resizeSection(3, style_mgr.get_scaled_size(15))

    def get_current_dir(self):
        return self._current_dir


class FileName(QLineEdit):

    def __init__(self):
        super().__init__()

    def get_file_name(self):
        return self.text()


