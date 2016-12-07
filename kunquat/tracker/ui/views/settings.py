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

import os

from PySide.QtCore import *
from PySide.QtGui import *

import kunquat.tracker.config as config
from .headerline import HeaderLine


class Settings(QWidget):

    def __init__(self):
        super().__init__()

        self._modules = Modules()

        dl = QGridLayout()
        dl.setContentsMargins(0, 0, 0, 0)
        dl.setHorizontalSpacing(4)
        dl.setVerticalSpacing(2)
        dl.addWidget(QLabel('Modules:'), 0, 0)
        dl.addWidget(self._modules, 0, 1)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(4)
        v.addWidget(HeaderLine('Default directories'))
        v.addLayout(dl)
        v.addStretch(1)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._modules.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._modules.unregister_updaters()


class Directory(QWidget):

    def __init__(self, conf_key):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self._conf_key = conf_key

        self._text = QLineEdit()
        self._browse = QPushButton('Browse...')

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._text)
        h.addWidget(self._browse)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._text, SIGNAL('textEdited(const QString&)'), self._change_dir_text)
        QObject.connect(self._browse, SIGNAL('clicked()'), self._change_dir_browse)

        self._update_dir()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_settings_dir' in signals:
            self._update_dir()

    def _update_dir(self):
        cfg = config.get_config()
        directory = cfg.get_value(self._conf_key) or ''

        old_block = self._text.blockSignals(True)
        if directory != self._text.text():
            self._text.setText(directory)
        self._text.blockSignals(old_block)

    def _change_dir_text(self, text):
        cfg = config.get_config()
        cfg.set_value(self._conf_key, text)
        self._updater.signal_update(set(['signal_settings_dir']))

    def _change_dir_browse(self):
        cfg = config.get_config()
        cur_dir = cfg.get_value(self._conf_key) or os.getcwd()
        new_dir = QFileDialog.getExistingDirectory(dir=cur_dir)
        if new_dir:
            self._change_dir_text(new_dir)


class Modules(Directory):

    def __init__(self):
        super().__init__('dir_modules')


