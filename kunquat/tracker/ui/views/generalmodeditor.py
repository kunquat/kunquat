# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .headerline import HeaderLine
from .numberslider import NumberSlider
from .updater import Updater
from .utils import get_default_font_info


class GeneralModEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._title = Title()
        self._authors = Authors()
        self._message = Message()

        self._mixing_volume = MixingVolume()
        self._force_shift = ForceShift()
        self._dc_blocker = DCBlocker()
        self._random_seed = RandomSeed()

        self.add_to_updaters(
                self._title,
                self._authors,
                self._message,
                self._mixing_volume,
                self._force_shift,
                self._dc_blocker,
                self._random_seed)

        self._meta_layout = QGridLayout()
        self._meta_layout.setContentsMargins(0, 0, 0, 0)
        self._meta_layout.setHorizontalSpacing(4)
        self._meta_layout.setVerticalSpacing(2)
        self._meta_layout.addWidget(QLabel('Title:'), 0, 0)
        self._meta_layout.addWidget(self._title, 0, 1)
        self._meta_layout.addWidget(QLabel('Authors:'), 1, 0)
        self._meta_layout.addWidget(self._authors, 1, 1)

        separator = QFrame()
        separator.setObjectName('HackSeparator')
        separator.setFrameShape(QFrame.HLine)
        separator.setFrameShadow(QFrame.Sunken)
        separator.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)
        separator.setMinimumHeight(2)

        self._global_settings_layout = QGridLayout()
        self._global_settings_layout.setContentsMargins(0, 0, 0, 0)
        self._global_settings_layout.setHorizontalSpacing(4)
        self._global_settings_layout.setVerticalSpacing(2)
        self._global_settings_layout.addWidget(QLabel('Mixing volume:'), 0, 0)
        self._global_settings_layout.addWidget(self._mixing_volume, 0, 1)
        self._global_settings_layout.addWidget(QLabel('Note force shift:'), 1, 0)
        self._global_settings_layout.addWidget(self._force_shift, 1, 1)
        self._global_settings_layout.addWidget(QLabel('Block dc:'), 2, 0)
        self._global_settings_layout.addWidget(self._dc_blocker, 2, 1)
        self._global_settings_layout.addWidget(QLabel('Initial random seed:'), 3, 0)
        self._global_settings_layout.addWidget(self._random_seed, 3, 1)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(4)
        v.addLayout(self._meta_layout)
        v.addWidget(HeaderLine('Message'))
        v.addWidget(self._message)
        v.addWidget(separator)
        v.addLayout(self._global_settings_layout)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        spacing_x = style_mgr.get_scaled_size_param('medium_padding')
        spacing_y = style_mgr.get_scaled_size_param('small_padding')

        self._meta_layout.setHorizontalSpacing(spacing_x)
        self._meta_layout.setVerticalSpacing(spacing_y)
        self._global_settings_layout.setHorizontalSpacing(spacing_x)
        self._global_settings_layout.setVerticalSpacing(spacing_y)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


class Title(QLineEdit, Updater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_title', self._update_title)

        self.textEdited.connect(self._change_title)

        self._update_title()

    def _update_title(self):
        module = self._ui_model.get_module()
        title = module.get_title() or ''

        if self.text() != title:
            old_block = self.blockSignals(True)
            self.setText(title)
            self.blockSignals(old_block)

    def _change_title(self, title):
        module = self._ui_model.get_module()
        module.set_title(title)
        self._updater.signal_update('signal_title')


class AuthorTableModel(QAbstractTableModel, Updater):

    def __init__(self):
        super().__init__()
        self._items = ['']

    def _on_setup(self):
        self._make_items()

    def _make_items(self):
        module = self._ui_model.get_module()
        count = module.get_author_count()

        self._items = []
        for i in range(count):
            self._items.append(module.get_author(i))
        self._items.append('')

    def get_index(self, author_index):
        return self.createIndex(0, author_index, self._items[author_index])

    # Qt interface

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._items)

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return 1

    def data(self, index, role):
        if role in (Qt.DisplayRole, Qt.EditRole):
            row = index.row()
            column = index.column()
            if row == 0:
                if 0 <= column < len(self._items):
                    return self._items[column]

        return None

    def headerData(self, section, orientation, role):
        return None

    def flags(self, index):
        default_flags = super().flags(index)
        if not index.isValid():
            return default_flags
        if index.row() != 0 or not 0 <= index.column() < len(self._items):
            return default_flags

        return default_flags | Qt.ItemIsEditable

    def setData(self, index, value, role):
        if role == Qt.EditRole:
            row = index.row()
            column = index.column()
            if row == 0 and 0 <= column < len(self._items):
                new_name = value
                module = self._ui_model.get_module()
                module.set_author(column, new_name)
                self._updater.signal_update('signal_authors')
                return True

        return False


class Authors(QTableView, Updater):

    def __init__(self):
        super().__init__()
        self._model = None

        self.horizontalHeader().hide()
        self.verticalHeader().hide()
        self.verticalHeader().setDefaultSectionSize(10)

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

        self.setEditTriggers(
                QAbstractItemView.SelectedClicked |
                QAbstractItemView.EditKeyPressed |
                QAbstractItemView.AnyKeyPressed)

    def _on_setup(self):
        self.register_action('signal_authors', self._update_model)
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()
        self._update_model()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.verticalHeader().setDefaultSectionSize(style_mgr.get_scaled_size(2))

    def _update_model(self):
        selected = None
        if self._model:
            selection_model = self.selectionModel()
            current_index = selection_model.currentIndex()
            if current_index.isValid():
                selected = current_index.column()

        if self._model:
            self.remove_from_updaters(self._model)
        self._model = AuthorTableModel()
        self.add_to_updaters(self._model)
        self.setModel(self._model)

        if selected != None:
            selection_model = self.selectionModel()
            selection_model.setCurrentIndex(
                    self._model.get_index(selected), QItemSelectionModel.SelectCurrent)

        # Remove excess height
        self.resizeColumnsToContents()

        row_height = self.rowHeight(0)
        opt = QStyleOption()
        opt.initFrom(self)
        size = self.style().sizeFromContents(
                QStyle.CT_LineEdit, opt, QSize(1, row_height), self).expandedTo(
                        QApplication.globalStrut())
        self.setFixedHeight(size.height())

    def keyPressEvent(self, event):
        if self.state() != QAbstractItemView.EditingState:
            index = self.currentIndex()
            if event.key() == Qt.Key_Return:
                if index.isValid() and self.edit(index, self.EditKeyPressed, event):
                    event.accept()
                    return
        return super().keyPressEvent(event)


class Message(QTextEdit, Updater):

    def __init__(self):
        super().__init__()
        self.setAcceptRichText(False)

    def _on_setup(self):
        self.register_action('signal_module_message', self._update_message)
        self.register_action('signal_style_changed', self._update_style)
        self.textChanged.connect(self._change_message)
        self._update_style()
        self._update_message()

    def _update_style(self):
        self.setStyleSheet('QTextEdit { font-family: monospace; }')

    def _update_message(self):
        module = self._ui_model.get_module()
        msg = module.get_message()

        if msg != self.toPlainText():
            old_block = self.blockSignals(True)
            self.setPlainText(msg)
            self.blockSignals(old_block)

    def _change_message(self):
        text = self.toPlainText()

        module = self._ui_model.get_module()
        module.set_message(text)

        self._updater.signal_update('signal_module_message')


class MixingVolume(NumberSlider, Updater):

    def __init__(self):
        super().__init__(1, -60, 18, width_txt='-60.0')

    def _on_setup(self):
        self.register_action('signal_mixing_volume', self._update_mixing_volume)
        self.register_action('signal_style_changed', self._update_style)

        self.numberChanged.connect(self._change_mixing_volume)

        self._update_style()
        self._update_mixing_volume()

    def _update_style(self):
        self.update_style(self._ui_model.get_style_manager())

    def _update_mixing_volume(self):
        module = self._ui_model.get_module()
        mix_vol = module.get_mixing_volume()
        self.set_number(mix_vol)

    def _change_mixing_volume(self, value):
        module = self._ui_model.get_module()
        module.set_mixing_volume(value)
        self._updater.signal_update('signal_mixing_volume')


class ForceShift(NumberSlider, Updater):

    def __init__(self):
        super().__init__(0, -60, 0, width_txt='-60.0')

    def _on_setup(self):
        self.register_action('signal_force_shift', self._update_shift)
        self.register_action('signal_style_change', self._update_style)

        self.numberChanged.connect(self._change_shift)

        self._update_style()
        self._update_shift()

    def _update_style(self):
        self.update_style(self._ui_model.get_style_manager())

    def _update_shift(self):
        module = self._ui_model.get_module()
        self.set_number(module.get_force_shift())

    def _change_shift(self, value):
        module = self._ui_model.get_module()
        module.set_force_shift(value)
        self._updater.signal_update('signal_force_shift')


class DCBlocker(QCheckBox, Updater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_dc_blocker', self._update_enabled)

        self.stateChanged.connect(self._change_enabled)

        self._update_enabled()

    def _update_enabled(self):
        module = self._ui_model.get_module()
        enabled = module.get_dc_blocker_enabled()

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _change_enabled(self, state):
        enabled = (state == Qt.Checked)

        module = self._ui_model.get_module()
        module.set_dc_blocker_enabled(enabled)
        self._updater.signal_update('signal_dc_blocker')


class RandomSeed(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._seed = UInt63SpinBox()
        self._auto_update = QCheckBox('Update automatically')

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._seed)
        h.addWidget(self._auto_update)
        h.addStretch()
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_random_seed', self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._seed.valueChanged.connect(self._change_random_seed)
        self._auto_update.stateChanged.connect(self._change_auto_update)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_all(self):
        module = self._ui_model.get_module()
        seed = module.get_random_seed()
        auto_update_enabled = module.get_random_seed_auto_update()

        self._seed.set_value(seed)
        self._seed.setEnabled(not auto_update_enabled)

        old_block = self._auto_update.blockSignals(True)
        self._auto_update.setChecked(auto_update_enabled)
        self._auto_update.blockSignals(old_block)

    def _change_random_seed(self):
        module = self._ui_model.get_module()
        module.set_random_seed(self._seed.get_value())
        self._updater.signal_update('signal_random_seed')

    def _change_auto_update(self, state):
        enabled = (state == Qt.Checked)
        module = self._ui_model.get_module()
        module.set_random_seed_auto_update(enabled)
        self._updater.signal_update('signal_random_seed')


class UInt63SpinBox(QAbstractSpinBox):

    _LIMIT = 2**63

    valueChanged = Signal(name='valueChanged')

    def __init__(self):
        super().__init__()

        self._value = 0
        self._minimum_size = None

        line_edit = self.lineEdit()
        line_edit.setText(str(self._value))
        line_edit.textChanged.connect(self._change_value)

    def set_value(self, value):
        old_block = self.blockSignals(True)
        self._value = value
        self.lineEdit().setText(str(self._value))
        self.blockSignals(old_block)
        self.update()

    def get_value(self):
        return self._value

    def _change_value(self, value_str):
        try:
            value = int(value_str)
        except ValueError:
            return

        self.set_value(value)
        self.valueChanged.emit()

    def stepEnabled(self):
        if self.wrapping():
            return QAbstractSpinBox.StepUpEnabled | QAbstractSpinBox.StepDownEnabled

        flags = QAbstractSpinBox.StepNone
        if self._value > 0:
            flags |= QAbstractSpinBox.StepDownEnabled
        if self._value < self._LIMIT:
            flags |= QAbstractSpinBox.StepUpEnabled

        return flags

    def stepBy(self, steps):
        self._value += steps

        if self.wrapping():
            while self._value < 0:
                self._value += self._LIMIT
            while self._value >= self._LIMIT:
                self._value -= self._LIMIT
        else:
            self._value = min(max(0, self._value), self._LIMIT - 1)

        line_edit = self.lineEdit()
        old_block = line_edit.blockSignals(True)
        line_edit.setText(str(self._value))
        line_edit.blockSignals(old_block)

        self.valueChanged.emit()
        self.update()

    def text(self):
        return str(self._value)

    def fixup(self, in_str):
        if not in_str:
            in_str = '0'
        return super().fixup(in_str)

    def validate(self, in_str, pos):
        if not in_str:
            return (QValidator.Intermediate, in_str, pos)

        try:
            value = int(in_str)
        except ValueError:
            return (QValidator.Invalid, in_str, pos)

        if value >= self._LIMIT:
            return (QValidator.Invalid, in_str, pos)

        return (QValidator.Acceptable, in_str, pos)

    def minimumSizeHint(self):
        if not self._minimum_size:
            fm = self.fontMetrics()
            width = fm.width(str(self._LIMIT - 1)) + 2
            height = self.lineEdit().minimumSizeHint().height()

            opt = QStyleOptionSpinBox()
            self.initStyleOption(opt)
            self._minimum_size = self.style().sizeFromContents(
                    QStyle.CT_SpinBox, opt, QSize(width, height), self).expandedTo(
                            QApplication.globalStrut())

        return self._minimum_size


