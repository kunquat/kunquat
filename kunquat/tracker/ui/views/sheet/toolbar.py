# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
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

from kunquat.kunquat.limits import *
import kunquat.tracker.cmdline as cmdline
import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from .editbutton import EditButton
from .replacebutton import ReplaceButton
from .restbutton import RestButton
from .delselectionbutton import DelSelectionButton
from .zoombutton import ZoomButton
from .lengtheditor import LengthEditor
from . import utils


class Toolbar(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self._follow_playback_button = FollowPlaybackButton()
        self._edit_button = EditButton()
        self._replace_button = ReplaceButton()
        self._rest_button = RestButton()
        self._del_selection_button = DelSelectionButton()
        self._undo_button = UndoButton()
        self._redo_button = RedoButton()
        self._cut_button = CutButton()
        self._copy_button = CopyButton()
        self._paste_button = PasteButton()
        self._convert_tr_button = ConvertTriggerButton()
        self._zoom_buttons = [
                ZoomButton('out'),
                ZoomButton('original'),
                ZoomButton('in'),
                ZoomButton('shrink_w'),
                ZoomButton('original_w'),
                ZoomButton('expand_w'),
            ]
        self._grid_toggle = GridToggle()
        self._grid_editor_button = GridEditorButton()
        self._grid_selector = GridSelector()
        self._length_editor = LengthEditor()

        h = QHBoxLayout()
        h.setContentsMargins(4, 0, 4, 4)
        h.setSpacing(2)

        h.addWidget(self._follow_playback_button)
        h.addWidget(self._edit_button)
        h.addWidget(self._replace_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._rest_button)
        if cmdline.get_experimental():
            h.addWidget(self._del_selection_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._undo_button)
        h.addWidget(self._redo_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._cut_button)
        h.addWidget(self._copy_button)
        h.addWidget(self._paste_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._convert_tr_button)
        h.addWidget(HackSeparator())
        for button in self._zoom_buttons:
            h.addWidget(button)
        h.addWidget(HackSeparator())
        h.addWidget(self._grid_toggle)
        h.addWidget(self._grid_editor_button)

        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        h.addWidget(spacer)

        h.addWidget(self._grid_selector)
        h.addWidget(self._length_editor)

        for i in range(h.count()):
            widget = h.itemAt(i).widget()
            if isinstance(widget, (QPushButton, QCheckBox)):
                widget.setFocusPolicy(Qt.NoFocus)

        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._follow_playback_button.set_ui_model(ui_model)
        self._edit_button.set_ui_model(ui_model)
        self._replace_button.set_ui_model(ui_model)
        self._rest_button.set_ui_model(ui_model)
        self._del_selection_button.set_ui_model(ui_model)
        self._undo_button.set_ui_model(ui_model)
        self._redo_button.set_ui_model(ui_model)
        self._cut_button.set_ui_model(ui_model)
        self._copy_button.set_ui_model(ui_model)
        self._paste_button.set_ui_model(ui_model)
        self._convert_tr_button.set_ui_model(ui_model)
        for button in self._zoom_buttons:
            button.set_ui_model(ui_model)
        self._grid_toggle.set_ui_model(ui_model)
        self._grid_editor_button.set_ui_model(ui_model)
        self._grid_selector.set_ui_model(ui_model)
        self._length_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._follow_playback_button.unregister_updaters()
        self._edit_button.unregister_updaters()
        self._replace_button.unregister_updaters()
        self._rest_button.unregister_updaters()
        self._del_selection_button.unregister_updaters()
        self._undo_button.unregister_updaters()
        self._redo_button.unregister_updaters()
        self._cut_button.unregister_updaters()
        self._copy_button.unregister_updaters()
        self._paste_button.unregister_updaters()
        self._convert_tr_button.unregister_updaters()
        for button in self._zoom_buttons:
            button.unregister_updaters()
        self._grid_toggle.unregister_updaters()
        self._grid_editor_button.unregister_updaters()
        self._grid_selector.unregister_updaters()
        self._length_editor.unregister_updaters()


class FollowPlaybackButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setCheckable(True)
        self.setFlat(True)
        self.setToolTip('Follow playback')
        self.setText('Follow playback')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._toggle_playback_following)

        self._update_playback_following()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_follow_playback' in signals:
            self._update_playback_following()

    def _update_playback_following(self):
        playback_manager = self._ui_model.get_playback_manager()
        old_block = self.blockSignals(True)
        self.setChecked(playback_manager.get_playback_cursor_following())
        self.blockSignals(old_block)

    def _toggle_playback_following(self):
        is_enabled = self.isChecked()
        playback_manager = self._ui_model.get_playback_manager()
        playback_manager.set_playback_cursor_following(is_enabled)
        self._updater.signal_update(set(['signal_follow_playback']))


class UndoButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setFlat(True)
        #self.setText('Undo')
        self.setToolTip('Undo (Ctrl + Z)')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._sheet_history = ui_model.get_sheet_history()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('undo')
        self.setIcon(QIcon(icon_path))

        QObject.connect(self, SIGNAL('clicked()'), self._undo)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_undo',
            'signal_redo',
            'signal_selection',
            'signal_pattern_length',
            'signal_grid'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        self.setEnabled(self._sheet_history.has_past_changes())

    def _undo(self):
        self._sheet_history.undo()
        self._ui_model.get_sheet_manager().flush_latest_column()
        self._updater.signal_update(set(['signal_undo']))


class RedoButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setFlat(True)
        #self.setText('Redo')
        self.setToolTip('Redo (Ctrl + Shift + Z)')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._sheet_history = ui_model.get_sheet_history()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('redo')
        self.setIcon(QIcon(icon_path))

        QObject.connect(self, SIGNAL('clicked()'), self._redo)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_undo',
            'signal_redo',
            'signal_selection',
            'signal_pattern_length',
            'signal_grid'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        self.setEnabled(self._sheet_history.has_future_changes())

    def _redo(self):
        self._sheet_history.redo()
        self._ui_model.get_sheet_manager().flush_latest_column()
        self._updater.signal_update(set(['signal_redo']))


class CutOrCopyButton(QPushButton):

    def __init__(self, button_type):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._sheet_manager = None

        if button_type == 'cut':
            text = 'Cut'
            shortcut = 'Ctrl + X'
        elif button_type == 'copy':
            text = 'Copy'
            shortcut = 'Ctrl + C'
        else:
            assert False

        self._button_type = button_type

        self.setFlat(True)
        #self.setText(text)
        self.setToolTip('{} ({})'.format(text, shortcut))

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._sheet_manager = ui_model.get_sheet_manager()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path(self._button_type)
        self.setIcon(QIcon(icon_path))

        QObject.connect(self, SIGNAL('clicked()'), self._cut_or_copy)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_selection', 'signal_edit_mode'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        selection = self._ui_model.get_selection()
        enabled = selection.has_area()
        if self._button_type == 'cut':
            enabled = enabled and self._sheet_manager.is_editing_enabled()
        self.setEnabled(enabled)

    def _cut_or_copy(self):
        selection = self._ui_model.get_selection()
        if (selection.has_area() and
                (self._button_type == 'copy' or
                    self._sheet_manager.is_editing_enabled())):
            utils.copy_selected_area(self._sheet_manager)
            if self._button_type == 'cut':
                self._sheet_manager.try_remove_area()
            selection.clear_area()
            self._updater.signal_update(set(['signal_selection']))


class CutButton(CutOrCopyButton):

    def __init__(self):
        super().__init__('cut')


class CopyButton(CutOrCopyButton):

    def __init__(self):
        super().__init__('copy')


class PasteButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._sheet_manager = None

        self.setFlat(True)
        #self.setText('Paste')
        self.setToolTip('Paste (Ctrl + V)')

        self._has_valid_data = False

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._sheet_manager = ui_model.get_sheet_manager()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('paste')
        self.setIcon(QIcon(icon_path))

        clipboard = QApplication.clipboard()
        QObject.connect(clipboard, SIGNAL('dataChanged()'), self._update_enabled_full)

        QObject.connect(self, SIGNAL('clicked()'), self._paste)

        self._update_enabled_full()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_selection', 'signal_edit_mode'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled_full(self):
        self._has_valid_data = utils.is_clipboard_area_valid(self._sheet_manager)
        self._update_enabled()

    def _update_enabled(self):
        selection = self._ui_model.get_selection()
        enabled = (self._sheet_manager.is_editing_enabled() and
                bool(selection.get_location()) and
                self._has_valid_data)
        self.setEnabled(enabled)

    def _paste(self):
        if self._sheet_manager.is_editing_enabled():
            selection = self._ui_model.get_selection()
            utils.try_paste_area(self._sheet_manager)
            selection.clear_area()
            self._updater.signal_update(set(['signal_selection']))


class ConvertTriggerButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setFlat(True)
        #self.setText('Convert')
        self.setToolTip('Convert between set and slide trigger (/)')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('convert_trigger')
        self.setIcon(QIcon(icon_path))

        QObject.connect(self, SIGNAL('clicked()'), self._convert_trigger)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_selection', 'signal_edit_mode', 'signal_play', 'signal_silence'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        self.setEnabled(
                sheet_manager.is_editing_enabled() and
                sheet_manager.allow_editing() and
                sheet_manager.is_at_convertible_set_or_slide_trigger())

    def _convert_trigger(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        sheet_manager.convert_set_or_slide_trigger()


class GridToggle(QCheckBox):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setText('Grid')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._set_grid_enabled)

        self._update_state()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_grid' in signals:
            self._update_state()

    def _update_state(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        is_grid_enabled = sheet_manager.is_grid_enabled()

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if is_grid_enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _set_grid_enabled(self):
        enabled = (self.checkState() == Qt.Checked)

        sheet_manager = self._ui_model.get_sheet_manager()
        sheet_manager.set_grid_enabled(enabled)
        self._updater.signal_update(set(['signal_grid']))


class GridEditorButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setText('Edit grids')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._open_grid_editor)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_grid' in signals:
            self._update_enabled()

    def _update_enabled(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        is_grid_enabled = sheet_manager.is_grid_enabled()
        self.setEnabled(is_grid_enabled)

    def _open_grid_editor(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_grid_editor()


class GridSelector(KqtComboBox):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setSizeAdjustPolicy(QComboBox.AdjustToContents)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self, SIGNAL('activated(int)'), self._change_grid_pattern)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _update_grid_pattern_names(self):
        grid_manager = self._ui_model.get_grid_manager()
        gp_ids = grid_manager.get_all_grid_pattern_ids()

        gp_items = [(gp_id, grid_manager.get_grid_pattern(gp_id).get_name())
            for gp_id in gp_ids]
        gp_items = sorted(gp_items, key=lambda x: x[1])

        old_block = self.blockSignals(True)
        self.set_items((name, gp_id) for (gp_id, name) in gp_items)
        self.blockSignals(old_block)

    def _get_pattern_instance(self):
        module = self._ui_model.get_module()
        album = module.get_album()
        if not album.get_existence():
            return None

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        song = album.get_song_by_track(location.get_track())
        pinst = song.get_pattern_instance(location.get_system())
        return pinst

    def _set_default_grid_pattern(self):
        grid_manager = self._ui_model.get_grid_manager()

        gp_id = None

        # Select grid at the current location if possible
        pinst = self._get_pattern_instance()
        if pinst:
            selection = self._ui_model.get_selection()
            location = selection.get_location()
            column = pinst.get_column(location.get_col_num())
            gp_id, _ = column.get_overlay_grid_info_at(location.get_row_ts())

            if gp_id == None:
                pattern = pinst.get_pattern()
                gp_id = pattern.get_base_grid_pattern_id()

        # Fall back to whatever we can find from the grid pattern collection
        if gp_id == None:
            all_gp_ids = grid_manager.get_all_grid_pattern_ids()
            if all_gp_ids:
                gp_id = all_gp_ids[0]

        if gp_id != None:
            grid_manager.set_default_grid_pattern_id(gp_id)

    def _update_grid_pattern_selection(self):
        grid_manager = self._ui_model.get_grid_manager()
        default_id = grid_manager.get_default_grid_pattern_id()

        old_block = self.blockSignals(True)
        index = self.findData(default_id)
        if index < 0:
            index = self.findData(0)
        assert index >= 0
        self.setCurrentIndex(index)
        self.blockSignals(old_block)

    def _perform_updates(self, signals):
        list_signals = set(['signal_module', 'signal_grid_pattern_list'])
        if not signals.isdisjoint(list_signals):
            self._update_grid_pattern_names()
            self._set_default_grid_pattern()
            self._update_grid_pattern_selection()

        update_signals = set(['signal_module', 'signal_selection', 'signal_grid'])
        if not signals.isdisjoint(update_signals):
            self._set_default_grid_pattern()
            self._update_grid_pattern_selection()

    def _change_grid_pattern(self, index):
        gp_id = self.itemData(index)
        if not gp_id:
            return

        pinst = self._get_pattern_instance()
        if not pinst:
            return

        offset = tstamp.Tstamp(0) # TODO

        pattern = pinst.get_pattern()

        sheet_manager = self._ui_model.get_sheet_manager()

        selection = self._ui_model.get_selection()
        if selection.has_rect_area():
            top_left = selection.get_area_top_left()
            bottom_right = selection.get_area_bottom_right()
            start_col = top_left.get_col_num()
            stop_col = bottom_right.get_col_num() + 1
            start_ts = top_left.get_row_ts()
            stop_ts = bottom_right.get_row_ts()

            pat_length = pattern.get_length()
            full_columns_selected = ((start_ts == tstamp.Tstamp(0)) and
                    (stop_ts > pat_length))
            all_selected = (full_columns_selected and
                    ((start_col, stop_col) == (0, COLUMNS_MAX)))

            if all_selected:
                sheet_manager.set_pattern_base_grid_pattern_id(
                        pattern, gp_id, is_final=False)

            if all_selected or (gp_id == pattern.get_base_grid_pattern_id()):
                gp_id = None

            if (full_columns_selected and
                    (pattern.get_base_grid_pattern_id() == gp_id) and
                    (pattern.get_base_grid_pattern_offset() == offset)):
                sheet_manager.clear_overlay_grids(pinst, start_col, stop_col)
            else:
                sheet_manager.set_overlay_grid(
                        pinst, start_col, stop_col, start_ts, stop_ts, gp_id, offset)

        else:
            location = selection.get_location()
            col_num = location.get_col_num()
            column = pinst.get_column(location.get_col_num())
            start_ts, stop_ts = column.get_overlay_grid_range_at(location.get_row_ts())
            col_gp_id, offset = column.get_overlay_grid_info_at(location.get_row_ts())

            if col_gp_id != None:
                pat_length = pattern.get_length()
                full_column_selected = ((start_ts == tstamp.Tstamp(0)) and
                        (stop_ts > pat_length))
                if (full_column_selected and
                        (pattern.get_base_grid_pattern_id() == gp_id) and
                        (pattern.get_base_grid_pattern_offset() == offset)):
                    sheet_manager.clear_overlay_grids(pinst, col_num, col_num + 1)
                else:
                    if gp_id == pattern.get_base_grid_pattern_id():
                        gp_id = None
                    sheet_manager.set_overlay_grid(
                        pinst, col_num, col_num + 1, start_ts, stop_ts, gp_id, offset)

            else:
                sheet_manager.set_pattern_base_grid_pattern_id(
                        pattern, gp_id, is_final=True)

        self._updater.signal_update(set(['signal_grid']))


class HackSeparator(QFrame):

    def __init__(self):
        super().__init__()
        self.setFrameShape(QFrame.VLine)
        self.setFrameShadow(QFrame.Sunken)


