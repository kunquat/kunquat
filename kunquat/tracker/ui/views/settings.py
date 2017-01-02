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

import os
import string

from PySide.QtCore import *
from PySide.QtGui import *

import kunquat.tracker.config as config
from .headerline import HeaderLine
from . import utils


class Settings(QWidget):

    def __init__(self):
        super().__init__()

        self._modules = Modules()
        self._instruments = Instruments()
        self._samples = Samples()
        self._effects = Effects()

        self._style_toggle = StyleToggle()
        self._colours = Colours()

        dgl = QGridLayout()
        dgl.setContentsMargins(0, 0, 0, 0)
        dgl.setHorizontalSpacing(4)
        dgl.setVerticalSpacing(2)
        dgl.addWidget(QLabel('Modules:'), 0, 0)
        dgl.addWidget(self._modules, 0, 1)
        dgl.addWidget(QLabel('Instruments:'), 1, 0)
        dgl.addWidget(self._instruments, 1, 1)
        dgl.addWidget(QLabel('Samples:'), 2, 0)
        dgl.addWidget(self._samples, 2, 1)
        dgl.addWidget(QLabel('Effects:'), 3, 0)
        dgl.addWidget(self._effects, 3, 1)

        dl = QVBoxLayout()
        dl.setContentsMargins(0, 0, 0, 0)
        dl.setSpacing(4)
        dl.addWidget(HeaderLine('Default directories'))
        dl.addLayout(dgl)
        dl.addStretch(1)

        uil = QVBoxLayout()
        uil.setContentsMargins(0, 0, 0, 0)
        uil.setSpacing(4)
        uil.addWidget(HeaderLine('User interface'))
        uil.addWidget(self._style_toggle)
        uil.addWidget(self._colours)

        h = QHBoxLayout()
        h.setContentsMargins(2, 2, 2, 2)
        h.setSpacing(8)
        h.addLayout(dl)
        h.addLayout(uil)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._modules.set_ui_model(ui_model)
        self._instruments.set_ui_model(ui_model)
        self._samples.set_ui_model(ui_model)
        self._effects.set_ui_model(ui_model)
        self._style_toggle.set_ui_model(ui_model)
        self._colours.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._colours.unregister_updaters()
        self._style_toggle.unregister_updaters()
        self._effects.unregister_updaters()
        self._samples.unregister_updaters()
        self._instruments.unregister_updaters()
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


class Instruments(Directory):

    def __init__(self):
        super().__init__('dir_instruments')


class Samples(Directory):

    def __init__(self):
        super().__init__('dir_samples')


class Effects(Directory):

    def __init__(self):
        super().__init__('dir_effects')


class StyleToggle(QCheckBox):

    def __init__(self):
        super().__init__('Enable custom style')
        self._ui_model = None
        self._updater = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('stateChanged(int)'), self._change_enabled)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_style_changed' in signals:
            self._update_enabled()

    def _update_enabled(self):
        style_manager = self._ui_model.get_style_manager()
        enabled = style_manager.is_custom_style_enabled()

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _change_enabled(self, state):
        enabled = (state == Qt.Checked)

        style_manager = self._ui_model.get_style_manager()
        style_manager.set_custom_style_enabled(enabled)

        self._updater.signal_update(set(['signal_style_changed']))


class ColourModel():

    def __init__(self, key, colour):
        self._key = key
        self._colour = colour

    def get_key(self):
        return self._key

    def get_colour(self):
        return self._colour


_COLOUR_DESCS = [
    ('bg_colour'         , 'Default background'),
    ('fg_colour'         , 'Default text'),
    ('disabled_fg_colour', 'Disabled text'),
    ('button_bg_colour'  , 'Button background'),
    ('button_fg_colour'  , 'Button foreground'),
]

_COLOUR_DESCS_DICT = dict(_COLOUR_DESCS)


class ColoursModel(QAbstractItemModel):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self._colours = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        self._make_colour_list()

    def _make_colour_list(self):
        style_manager = self._ui_model.get_style_manager()

        colours = []

        for k, _ in _COLOUR_DESCS:
            colour = style_manager.get_style_param(k)
            colours.append(ColourModel(k, colour))

        self._colours = colours

    def get_colour_indices(self):
        for row, node in enumerate(self._colours):
            yield self.createIndex(row, 1, node)

    # Qt interface

    def columnCount(self, _):
        return 2

    def rowCount(self, parent):
        if not parent.isValid():
            return len(self._colours)
        return 0

    def index(self, row, col, parent):
        if not parent.isValid():
            node = self._colours[row]
        else:
            return QModelIndex()
        return self.createIndex(row, col, node)

    def parent(self, index):
        return QModelIndex()

    def data(self, index, role):
        if role == Qt.DisplayRole:
            assert index.isValid()
            node = index.internalPointer()
            column = index.column()
            if column == 0:
                key = node.get_key()
                desc = _COLOUR_DESCS_DICT[key]
                return desc
            elif column == 1:
                return node.get_colour()

    def flags(self, index):
        default_flags = super().flags(index)
        if not index.isValid():
            return default_flags
        return Qt.ItemIsEnabled


class ColourButton(QPushButton):

    colourSelected = Signal(str, name='colourSelected')

    def __init__(self, key):
        super().__init__()
        self._key = key

        self.setFixedSize(QSize(48, 16))
        self.setAutoFillBackground(True)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def get_key(self):
        return self._key

    def set_colour(self, colour):
        style = 'QPushButton {{ background-color: {}; }}'.format(colour)
        self.setStyleSheet(style)

    def _clicked(self):
        QObject.emit(self, SIGNAL('colourSelected(QString)'), self._key)


class ColourCodeValidator(QValidator):

    def __init__(self):
        super().__init__()

    def validate(self, contents, pos):
        if (not contents) or (contents[0] != '#') or (len(contents) > 7):
            return (QValidator.Invalid, contents, pos)
        if not all(c in string.hexdigits for c in contents[1:]):
            return (QValidator.Invalid, contents, pos)
        if len(contents) < 7:
            return (QValidator.Intermediate, contents, pos)
        return (QValidator.Acceptable, contents, pos)


class ColourEditor(QWidget):

    colourModified = Signal(str, str, 'colourModified')

    def __init__(self):
        super().__init__()
        self._key = None
        self._orig_colour = None
        self._new_colour = None

        self._code_editor = QLineEdit()
        self._code_editor.setValidator(ColourCodeValidator())

        self._revert_button = QPushButton('Revert')
        self._accept_button = QPushButton('Done')

        cl = QHBoxLayout()
        cl.setContentsMargins(0, 0, 0, 0)
        cl.setSpacing(2)
        cl.addWidget(QLabel('Code:'))
        cl.addWidget(self._code_editor, 1)

        bl = QHBoxLayout()
        bl.setContentsMargins(0, 0, 0, 0)
        bl.setSpacing(2)
        bl.addWidget(self._revert_button)
        bl.addWidget(self._accept_button)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(2)
        v.addLayout(cl)
        v.addLayout(bl)
        self.setLayout(v)

        QObject.connect(
                self._code_editor,
                SIGNAL('textEdited(const QString&)'),
                self._change_code)

        QObject.connect(self._revert_button, SIGNAL('clicked()'), self._revert)

        QObject.connect(self._accept_button, SIGNAL('clicked()'), self.hide)

    def set_colour(self, key, colour):
        self._key = key
        self._orig_colour = colour
        self._new_colour = colour

        desc = _COLOUR_DESCS_DICT[key]
        noncap_desc = desc[0].lower() + desc[1:]
        self.setWindowTitle('Choose colour for ' + noncap_desc)

        code = utils.get_str_from_colour(self._orig_colour)
        old_block = self._code_editor.blockSignals(True)
        self._code_editor.setText(code)
        self._code_editor.blockSignals(old_block)

        self._revert_button.setEnabled(False)

    def _update_colour(self, new_colour):
        self._new_colour = new_colour
        self._revert_button.setEnabled(self._new_colour != self._orig_colour)

    def _change_code(self, text):
        if self._code_editor.hasAcceptableInput():
            self._update_colour(utils.get_colour_from_str(text))
            QObject.emit(
                    self, SIGNAL('colourModified(QString, QString)'), self._key, text)

    def _revert(self):
        orig_code = utils.get_str_from_colour(self._orig_colour)
        QObject.emit(
                self, SIGNAL('colourModified(QString, QString)'), self._key, orig_code)
        self.hide()


class Colours(QTreeView):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self._model = None

        self._colour_editor = ColourEditor()

        header = self.header()
        header.setStretchLastSection(False)
        header.setResizeMode(0, QHeaderView.Stretch)
        self.setHeaderHidden(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._model = ColoursModel()
        self._model.set_ui_model(ui_model)
        self.setModel(self._model)

        for index in self._model.get_colour_indices():
            node = index.internalPointer()
            key = node.get_key()
            button = ColourButton(key)
            QObject.connect(
                    button, SIGNAL('colourSelected(QString)'), self._open_colour_editor)
            self.setIndexWidget(index, button)

        self.resizeColumnToContents(0)
        self.setColumnWidth(1, 48)

        QObject.connect(
                self._colour_editor,
                SIGNAL('colourModified(QString, QString)'),
                self._update_colour)

        self._update_enabled()
        self._update_button_colours()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_style_changed' in signals:
            self._update_enabled()
            self._update_button_colours()

    def _update_enabled(self):
        style_manager = self._ui_model.get_style_manager()
        self.setEnabled(style_manager.is_custom_style_enabled())

    def _update_button_colours(self):
        style_manager = self._ui_model.get_style_manager()

        for index in self._model.get_colour_indices():
            button = self.indexWidget(index)
            key = button.get_key()
            colour = style_manager.get_style_param(key)
            button.set_colour(colour)

    def _open_colour_editor(self, key):
        style_manager = self._ui_model.get_style_manager()
        cur_colour_str = style_manager.get_style_param(key)
        cur_colour = utils.get_colour_from_str(cur_colour_str)
        self._colour_editor.set_colour(key, cur_colour)
        self._colour_editor.show()

    def _update_colour(self, key, colour_code):
        style_manager = self._ui_model.get_style_manager()
        style_manager.set_style_param(key, colour_code)
        self._updater.signal_update(set(['signal_style_changed']))

    def hideEvent(self, event):
        self._colour_editor.hide()


